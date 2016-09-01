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

#include <ctime>
#include <chrono>

#include "KernelBoost.hpp"

#ifdef MOVABLE_TRAIN
KernelBoost::KernelBoost(Parameters &params,
			 const SmoothingMatrices &SM,
			 const Dataset &dataset)
{
	boostedClassifiers.resize(dataset.getGtPairsNo());

	/* Create the boosted classifiers for each GT pair */
	for (unsigned int i = 0; i < dataset.getGtPairsNo(); ++i) {
		boostedClassifiers[i] = new BoostedClassifier(params, SM,
							      dataset, i);
	}

	/* Classify the images in the dataset, convert the overall
	   classification problem in a binary one, and finally solve it using
	   the performed classifications as additional channels */
	params.posSamplesNo = params.finalSamplesNo;
	params.negSamplesNo = params.finalSamplesNo;
	params.treeDepth = params.finalTreeDepth;

	Dataset dataset_final(params, dataset, boostedClassifiers);
	finalClassifier = new BoostedClassifier(params, SM, dataset_final, 0);

	std::vector< cv::Mat > scoreImages(dataset_final.getImagesNo());
	/* Perform the final classification on training images */
#pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < dataset_final.getImagesNo(); ++i) {
		std::chrono::time_point< std::chrono::system_clock > start;
		std::chrono::time_point< std::chrono::system_clock > end;
		start = std::chrono::system_clock::now();

		EMat result;
		if (params.fastClassifier) {
			const sampleSet& ePoints = dataset_final.getEPoints(i);
			finalClassifier->classifyImage(dataset_final,
						       ePoints,
						       result);
		} else {
			/* Prepare a vector containing the set of OpenCV
			   matrices corresponding to the available channels */
			std::vector< cv::Mat > chs;
			dataset_final.getChsForImage(i, chs);
			finalClassifier->classifyFullImage(chs,
							   dataset_final.getBorderSize(),
							   result);
		}
		normalizeImage(result, dataset_final.getBorderSize(),
			       scoreImages[i]);
#ifndef TESTS
		saveClassifiedImage(result,
				    params.finalResDir,
				    dataset_final.getImageName(i),
				    dataset_final.getBorderSize());
		float MR = computeMR(result, dataset_final.getGt(0, i),
				     dataset_final.getBorderSize());
		end = std::chrono::system_clock::now();
		std::chrono::duration< double > elapsed_s = end-start;
		log_info("\t\tImage %d/%d DONE! (took %.3fs, MR=%.3f)",
			 i+1, dataset_final.getImagesNo(),
			 elapsed_s.count(), MR);
#endif /* TESTS */
	}

	/* For the moment, use a fixed threshold */
	binaryThreshold = 0;

#ifndef TESTS
	/* Dumping the thresholded images for reference */
	for (unsigned int i = 0; i < dataset_final.getImagesNo(); ++i) {
		saveThresholdedImage(scoreImages[i],
				     dataset_final.getMask(i),
				     binaryThreshold,
				     params.finalResDir,
				     dataset_final.getImageName(i),
				     dataset_final.getOriginalImgSize(i),
				     dataset_final.getBorderSize());
	}

	/* Save WL distribution statistics */
	std::string statsFName = params.baseResDir + "/statistics.txt";
	FILE *fp = fopen(statsFName.c_str(), "wt");
	if (fp != NULL) {
		for (unsigned int i = 0; i < boostedClassifiers.size(); ++i) {
			fprintf(fp, "________ BC %d (%d-%d) ________\n",
				i,
				dataset.getGtNegativePairValue(i),
				dataset.getGtPositivePairValue(i));

			std::vector< int > count(dataset.getDataChNo(), 0);
			boostedClassifiers[i]->getChCount(count);
			float sum = 0;
			for (unsigned int j = 0; j < count.size(); ++j) {
				sum += count[j];
			}
			for (unsigned int j = 0; j < count.size(); ++j) {
				fprintf(fp, "ch %d/%d (%s): \t%d (%.2f%%)\n",
					j+1, (int)count.size(),
					params.channelList[j].c_str(),
					count[j], (float)count[j]/sum*100);
			}
			fprintf(fp, "\n");
		}

		fprintf(fp, "________ FINAL CLASSIFIER ________\n");
		std::vector< int > count(dataset.getDataChNo()+
					 boostedClassifiers.size(), 0);
		finalClassifier->getChCount(count);
		float sum = 0;
		for (unsigned int j = 0; j < count.size(); ++j) {
			sum += count[j];
		}
		for (unsigned int j = 0; j < count.size(); ++j) {
			if (j < params.channelList.size()) {
				fprintf(fp, "ch %d/%d (%s): \t%d (%.2f%%)\n",
					j+1, (int)count.size(),
					params.channelList[j].c_str(),
					count[j], (float)count[j]/sum*100);
			} else {
				fprintf(fp, "ch %d/%d (class %d-%d): \t%d (%.2f%%)\n",
					j+1, (int)count.size(),
					dataset.getGtNegativePairValue(j-params.channelList.size()),
					dataset.getGtPositivePairValue(j-params.channelList.size()),
					count[j], (float)count[j]/sum*100);
			}
		}
		fprintf(fp, "\n");

		fclose (fp);
	} else {
		log_err("Cannot open statistics file %s for writing",
			statsFName.c_str());
	}
#endif /* TESTS */
}

KernelBoost::KernelBoost(std::string &descr_json)
{
	Json::Value root;
	Json::Reader reader;

	if (!reader.parse(descr_json, root)) {
		throw std::runtime_error("invalidJSONDescription");
	}

	Deserialize(root);
}

#else /* !MOVABLE_TRAIN */

KernelBoost::KernelBoost(std::string &descr_json,
			 const Parameters &params,
			 const Dataset &dataset)
{
	Json::Value root;
	Json::Reader reader;

	if (!reader.parse(descr_json, root)) {
		throw std::runtime_error("invalidJSONDescription");
	}

	Deserialize(root);

	log_info("Start by performing individual pairs classification...");
	Dataset dataset_final(dataset, boostedClassifiers);

	/* Perform the final classification on training images */
	log_info("Last round of classification on grouped GT values...");
#pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < dataset_final.getImagesNo(); ++i) {
#ifndef TESTS
		std::chrono::time_point< std::chrono::system_clock > start;
		std::chrono::time_point< std::chrono::system_clock > end;
		start = std::chrono::system_clock::now();
#endif /* TESTS */

		EMat result;
		if (params.fastClassifier) {
			const sampleSet& ePoints = dataset_final.getEPoints(i);
			finalClassifier->classifyImage(dataset_final,
						       ePoints,
						       result);
		} else {
			/* Prepare a vector containing the set of OpenCV matrices
			   corresponding to the available channels */
			std::vector< cv::Mat > chs;
			dataset_final.getChsForImage(i, chs);
			finalClassifier->classifyFullImage(chs,
							   dataset_final.getBorderSize(),
							   result);
		}
#ifndef TESTS
		saveClassifiedImage(result,
				    params.baseResDir,
				    dataset_final.getImageName(i),
				    dataset_final.getBorderSize());
		cv::Mat normImage;

		/* Here image normalization has been replaced by border cropping
		   followed by thresholding with a fixed threshold specified in
		   the configuration file */
		// normalizeImage(result, dataset_final.getBorderSize(),
		//	       normImage);
		cv::Mat beforeCrop(result.rows(), result.cols(), CV_32FC1);
		cv::eigen2cv(result, beforeCrop);

		/* Drop border */
		normImage = beforeCrop(cv::Range(dataset_final.getBorderSize(),
						 beforeCrop.rows-dataset_final.getBorderSize()),
				       cv::Range(dataset_final.getBorderSize(),
						 beforeCrop.cols-dataset_final.getBorderSize()));

		saveThresholdedImage(normImage,
				     dataset_final.getMask(i),
				     params.threshold,
				     params.baseResDir,
				     dataset_final.getImageName(i),
				     dataset_final.getOriginalImgSize(i),
				     dataset_final.getBorderSize());

		saveOverlayedImage(dataset_final.getImagePath(i),
				   params.baseResDir,
				   dataset_final.getImageName(i));

		end = std::chrono::system_clock::now();
		std::chrono::duration< double > elapsed_s = end-start;
		log_info("\t\tImage %d/%d DONE! (took %.3fs)",
			 i+1, dataset_final.getImagesNo(), elapsed_s.count());
#endif /* TESTS */
	}
}
#endif /* MOVABLE_TRAIN */


KernelBoost::KernelBoost(Json::Value &root)
{
	Deserialize(root);
}

KernelBoost::~KernelBoost()
{
	for (unsigned int i = 0; i < boostedClassifiers.size(); ++i) {
		delete boostedClassifiers[i];
	}
	delete finalClassifier;
}

KernelBoost::KernelBoost(const KernelBoost &obj)
{
	/* Deallocate previous boosted classifiers */
	for (unsigned int i = 0; i < boostedClassifiers.size(); ++i) {
		delete boostedClassifiers[i];
	}
	delete finalClassifier;
	/* Resize the boosted classifiers vector */
	this->boostedClassifiers.resize(obj.boostedClassifiers.size());
	/* Create the new set of boosted classifiers from the elements
	   of the copied one */
	for (unsigned int i = 0; i < boostedClassifiers.size(); ++i) {
		boostedClassifiers[i] =
			new BoostedClassifier(*(obj.boostedClassifiers[i]));
	}
	finalClassifier = new BoostedClassifier(*(obj.finalClassifier));
	binaryThreshold = obj.binaryThreshold;
}

KernelBoost &
KernelBoost::operator=(const KernelBoost &rhs)
{
	if (this != &rhs) {
		/* Deallocate previous boosted classifiers */
		for (unsigned int i = 0; i < boostedClassifiers.size();
		     ++i) {
			delete boostedClassifiers[i];
		}
		delete finalClassifier;

		/* Resize the weak learners vector */
		this->boostedClassifiers.resize(rhs.boostedClassifiers.size());
		/* Create the new set of weak learners from the elements
		   of the copied one */
		for (unsigned int i = 0; i < boostedClassifiers.size(); ++i) {
			boostedClassifiers[i] =
				new BoostedClassifier(*(rhs.boostedClassifiers[i]));
		}
		finalClassifier =
			new BoostedClassifier(*(rhs.finalClassifier));
		binaryThreshold = rhs.binaryThreshold;
	}

	return *this;
}

bool
operator==(const KernelBoost &kb1, const KernelBoost &kb2)
{
	if (kb1.boostedClassifiers.size() != kb2.boostedClassifiers.size())
		return false;

	for (unsigned int i = 0; i < kb1.boostedClassifiers.size(); ++i) {
		if (*(kb1.boostedClassifiers[i]) != *(kb2.boostedClassifiers[i]) ||
		    *(kb1.finalClassifier) != *(kb2.finalClassifier) ||
		    kb1.binaryThreshold != kb2.binaryThreshold)
			return false;
	}

	return true;
}

bool
operator!=(const KernelBoost &kb1, const KernelBoost &kb2)
{
	return !(kb1 == kb2);
}

void
KernelBoost::Serialize(Json::Value &)
{
	throw std::runtime_error("deprecatedSerialization");
}

void
KernelBoost::serialize(Json::Value &root, const Parameters &params)
{
	Json::Value kb_json(Json::objectValue);
	Json::Value boostedClassifiers_json(Json::arrayValue);
	for (unsigned int i = 0; i < boostedClassifiers.size(); ++i) {
		Json::Value bc_json(Json::objectValue);

		boostedClassifiers[i]->Serialize(bc_json);
		boostedClassifiers_json.append(bc_json);
	}
	kb_json["BoostedClassifiers"] = boostedClassifiers_json;

	Json::Value final_bc_json(Json::objectValue);
	finalClassifier->Serialize(final_bc_json);
	kb_json["FinalClassifier"] = final_bc_json;
	kb_json["binaryThreshold"] = binaryThreshold;
	kb_json["fastClassifier"] = params.fastClassifier;

	kb_json["sampleSize"] = params.sampleSize;
	kb_json["imgRescaleFactor"] = params.imgRescaleFactor;

	kb_json["houghMinDist"] = params.houghMinDist;
	kb_json["houghHThresh"] = params.houghHThresh;
	kb_json["houghLThresh"] = params.houghLThresh;
	kb_json["houghMinRad"] = params.houghMinRad;
	kb_json["houghMaxRad"] = params.houghMaxRad;

	for (unsigned int i = 0; i < params.channelList.size(); ++i) {
		kb_json["Channels"].append(params.channelList[i]);
	}

	root["KernelBoost"] = kb_json;
}

void
KernelBoost::Deserialize(Json::Value &root)
{
	for (Json::Value::iterator it =
		     root["KernelBoost"]["BoostedClassifiers"].begin();
	     it != root["KernelBoost"]["BoostedClassifiers"].end(); ++it) {
		BoostedClassifier *bc = new BoostedClassifier(*it);
		boostedClassifiers.push_back(bc);
	}
	finalClassifier =
		new BoostedClassifier(root["KernelBoost"]["FinalClassifier"]);
	binaryThreshold =
		root["KernelBoost"]["binaryThreshold"].asDouble();
}
