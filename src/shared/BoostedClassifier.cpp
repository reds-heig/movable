/*******************************************************************************
 ** MOVABLE project - REDS Institute, HEIG-VD, Yverdon-les-Bains (CH) - 2016  **
 **									      **
 ** This file is part of MOVABLE.					      **
 **									      **
 **  MOVABLE is free software: you can redistribute it and/or modify	      **
 **  it under the terms of the GNU General Public License as published by     **
 **  the Free Software Foundation, either version 3 of the License, or	      **
 **  (at your option) any later version.				      **
 **									      **
 **  MOVABLE is distributed in the hope that it will be useful,		      **
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of	      **
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	      **
 **  GNU General Public License for more details.			      **
 **									      **
 **  You should have received a copy of the GNU General Public License	      **
 **  along with MOVABLE.  If not, see <http://www.gnu.org/licenses/>.	      **
 ******************************************************************************/

#include <iostream>
#include <vector>
#include <cassert>

#include "BoostedClassifier.hpp"
#include "utils.hpp"

#ifdef MOVABLE_TRAIN
#include <chrono>
#include <ctime>

BoostedClassifier::BoostedClassifier(const Parameters &params,
				     const SmoothingMatrices &SM,
				     const Dataset &dataset,
				     const unsigned int gtPair)
	: gtPair(gtPair)
{
	std::chrono::time_point< std::chrono::system_clock > start;
	std::chrono::time_point< std::chrono::system_clock > end;
	start = std::chrono::system_clock::now();

	log_info("Learning Boosted classifier on gt pair (%d-%d)...",
		 dataset.getGtNegativePairValue(gtPair),
		 dataset.getGtPositivePairValue(gtPair));
	weakLearners.resize(params.wlNo);

	/* Collect the samples for the considered gt pair by first
	   getting the positive ones, and then pasting after the
	   negative ones */
	sampleSet samplePositions;
	dataset.getSamplePositions(POS_GT_CLASS, gtPair,
				   params.posSamplesNo,
				   samplePositions);
	sampleSet negSamples;
	dataset.getSamplePositions(NEG_GT_CLASS, gtPair,
				   params.negSamplesNo,
				   negSamples);

	/* If dataset balancing is selected and the dataset is unbalanced,
	   downsize the samples set that is too big */
	if (params.datasetBalance) {
		log_info("\tUsing dataset balancing");
		log_info("\t\tBEFORE: negative = %d, positive = %d",
			 (int)negSamples.size(), (int)samplePositions.size());
		if (samplePositions.size() > 1.2*negSamples.size()) {
			dataset.shrinkSamplePositions(samplePositions,
						      1.2*negSamples.size());
		}
		if (negSamples.size() > 1.2*samplePositions.size()) {
			dataset.shrinkSamplePositions(negSamples,
						      1.2*samplePositions.size());
		}
		log_info("\t\tAFTER : negative = %d, positive = %d",
			 (int)negSamples.size(), (int)samplePositions.size());
	} else {
		log_info("\tNo dataset balancing");
		log_info("\t\tnegative = %d, positive = %d",
			 (int)negSamples.size(), (int)samplePositions.size());
	}

	samplePositions.insert(samplePositions.end(),
			       negSamples.begin(),
			       negSamples.end());

	/* Classifier's cumulated response */
	EVec currentResponse(samplePositions.size());
	/* Initial response is zero */
	currentResponse.setZero();
	/* Labels corresponding to the sampled set */
	EVec Y(samplePositions.size());
	for (unsigned int i = 0; i < samplePositions.size(); ++i) {
		Y(i) = samplePositions[i].label;
	}
	/* At the beginning, all the samples are equal... */
	EVec W(samplePositions.size());
	W.setOnes();

	/* ... but some samples are more equal than the others: those in images
	   returned by a technician will have their weight increased */
	for (unsigned int i = 0; i < samplePositions.size(); ++i) {
		if (dataset.isFeedbackImage(samplePositions[i].imageNo)) {
			W(i) = FEEDBACK_SAMPLE_WEIGHT;
		}
	}

#ifndef TESTS
	/* Files used to store statistics about the learning process */
	std::string MR_fname = params.intermedResDir[gtPair]
		+ std::string("/MR.txt");
	FILE *fp_MR = fopen(MR_fname.c_str(), "wt");
	if (fp_MR == NULL) {
		log_err("Cannot open MR statistics file %s", MR_fname.c_str());
		throw std::runtime_error("MRstatisticsFile");
	}

	/* Open the destination file for the classifier to avoid wasting energy
	   learning only to get something we cannot store */
	std::string dstFname = params.intermedResDir[gtPair]+
		"/bc_classifier.json";
	std::ofstream file(dstFname);
	if (!file.is_open()) {
		log_err("Unable to open destination file %s", dstFname.c_str());
		throw std::runtime_error("dstClassifier");
	}
#endif /* TESTS */

	/* Create the weak learners, altering each time the weights and
	   the current response as the learning progresses */

	/* We stop learning if either:
	   + the MR is below 0.5% (we want to avoid overfitting)
	   + the MR hasn't been going down in the past 20 iterations
	   + the loss is too low (below 1%)
	   + the loss hasn't been going down in the past 20 iterations
	*/
	bool stop_learning = false;
	for (unsigned int wl = 0; wl < params.wlNo && !stop_learning; ++wl) {
		log_info("\tLearning weak learner %d/%d (gt pair %d-%d)...",
			 wl+1, params.wlNo,
			 dataset.getGtNegativePairValue(gtPair),
			 dataset.getGtPositivePairValue(gtPair));
		weakLearners[wl] = new WeakLearner(params,
						   SM,
						   dataset,
						   samplePositions,
						   W,
						   Y,
						   currentResponse);
		stop_learning = (weakLearners[wl]->getMR() < 0.005) ||
			((wl >= 20) &&
			 (fabs(weakLearners[wl]->getMR()-weakLearners[wl-20]->getMR()) < 1e-3)) ||
			(weakLearners[wl]->getLoss() < 0.01) ||
			((wl >= 20) &&
			 (fabs(weakLearners[wl]->getLoss()-weakLearners[wl-20]->getLoss()) < 1e-3));

		if (stop_learning) {
			log_info("\tCriteria to stop learning met, no further "
				 "WL for this classifier will be learnt");
			weakLearners.resize(wl);
		}
#ifndef TESTS
		fprintf(fp_MR, "%.4f\n", weakLearners[wl]->getMR());
#endif /* TESTS */
	}

#ifndef TESTS
	fclose(fp_MR);

	end = std::chrono::system_clock::now();
	std::chrono::duration< double > elapsed_s = end-start;
	log_info("Learning Boosted classifier on gt pair (%d-%d) COMPLETED! "
		 "(took %.3fs)\n",
		 dataset.getGtNegativePairValue(gtPair),
		 dataset.getGtPositivePairValue(gtPair),
		 elapsed_s.count());
#endif /* TESTS */

	/* Store the individual classifier in a file */
	std::string BC_json;
	JSONSerializer::Serialize(this, BC_json);

#ifndef TESTS
	file << BC_json;
	file.close();
#endif /* TESTS */
}

#endif /* MOVABLE_TRAIN */

BoostedClassifier::BoostedClassifier(std::string &descr_json)
{
	Json::Value root;
	Json::Reader reader;

	if (!reader.parse(descr_json, root)) {
		throw std::runtime_error("invalidJSONDescription");
	}

	Deserialize(root);
}

void
BoostedClassifier::Serialize(Json::Value &root)
{
	Json::Value bc_json(Json::objectValue);
	Json::Value weakLearners_json(Json::arrayValue);
	for (unsigned int i = 0; i < weakLearners.size(); ++i) {
		Json::Value wl_json(Json::objectValue);

		weakLearners[i]->Serialize(wl_json);
		weakLearners_json.append(wl_json);
	}
	bc_json["WeakLearners"] = weakLearners_json;

	Json::Value params_json(Json::objectValue);
	params_json["gtPair"] = gtPair;
	bc_json["params"] = params_json;

	root["BoostedClassifier"] = bc_json;
}

void
BoostedClassifier::Deserialize(Json::Value &root)
{
	for (Json::Value::iterator it =
		     root["BoostedClassifier"]["WeakLearners"].begin();
	     it != root["BoostedClassifier"]["WeakLearners"].end(); ++it) {
		WeakLearner *wl = new WeakLearner(*it);
		weakLearners.push_back(wl);
	}
	gtPair = root["BoostedClassifier"]["params"]["gtPair"].asInt();
}
