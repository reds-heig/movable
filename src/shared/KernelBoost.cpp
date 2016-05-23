/*******************************************************************************
** MOVABLE project - REDS Institute, HEIG-VD, Yverdon-les-Bains (CH) - 2016   **
**                                                                            **
** This file is part of MOVABLE.                                              **
**                                                                            **
**  MOVABLE is free software: you can redistribute it and/or modify           **
**  it under the terms of the GNU General Public License as published by      **
**  the Free Software Foundation, either version 3 of the License, or         **
**  (at your option) any later version.                                       **
**                                                                            **
**  MOVABLE is distributed in the hope that it will be useful,                **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of            **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             **
**  GNU General Public License for more details.                              **
**                                                                            **
**  You should have received a copy of the GNU General Public License         **
**  along with MOVABLE.  If not, see <http://www.gnu.org/licenses/>.          **
*******************************************************************************/

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
#ifndef TESTS
	Dataset dataset_final(params, dataset, boostedClassifiers);
#else
	Dataset dataset_final(dataset, boostedClassifiers);
#endif /* TESTS */
	finalClassifier = new BoostedClassifier(params, SM, dataset_final, 0);

	std::vector< cv::Mat > scoreImages(dataset_final.getImagesNo());
	/* Perform the final classification on training images */
#pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < dataset_final.getImagesNo(); ++i) {
		std::chrono::time_point< std::chrono::system_clock > start;
		std::chrono::time_point< std::chrono::system_clock > end;
		start = std::chrono::system_clock::now();

		/* Prepare a vector containing the set of OpenCV matrices
		   corresponding to the available channels */
		std::vector< cv::Mat > chs;
		dataset_final.getChsForImage(i, chs);

		EMat result;
		finalClassifier->classifyImage(chs,
					       dataset_final.getBorderSize(),
					       result);
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

#if 0
	/* Now computing a threshold, for the binary thresholding of the
	   classified images, that maximizes the F1-score */
	std::vector< float > thresholds(dataset_final.getImagesNo());
	log_info("Computing the best thresholding value...");
	/* ONLY images with some positive values in it are considered (otherwise
	   the business gets shady...) */
	// #pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < dataset_final.getImagesNo(); ++i) {
		EMat egt = dataset_final.getGt(0, i);
		if (egt.maxCoeff() > NEG_GT_CLASS) {
			cv::Mat gt(egt.rows(), egt.cols(), CV_32FC1);
			cv::eigen2cv(egt, gt);
			float F1Score = 0;
			float f;
			for (float thr = -1+1.0/200; thr < 1; thr += 1.0/200) {
				f = computeF1Score(scoreImages[i], gt, thr);
				if (f > F1Score) {
					F1Score = f;
					thresholds[i] = thr;
				}
			}
			log_info("\t\tImage %d/%d, threshold %.3f gives "
				 "F1-score %.3f",
				 i+1, dataset_final.getImagesNo(),
				 thresholds[i], F1Score);
		} else {
			/* Set the threshold to -10 to rule this image out */
			thresholds[i] = -10;
		}
	}

	/* Get the final threshold as an average of the thresholds of the
	   considered images */
	binaryThreshold = 0;
	int cnt = 0;
	for (unsigned int i = 0; i < dataset_final.getImagesNo(); ++i) {
		if (thresholds[i] > -10) {
			binaryThreshold += thresholds[i];
			++cnt;
		}
	}
	if (cnt == 0) {
		log_info("WEIRD, no images actually contributed to threshold "
			 "determination... :(");
	} else {
		binaryThreshold /= cnt;
	}
#endif /* 0 */

	/* For the moment, use a fixed threshold */
	binaryThreshold = 0.6;

#ifndef TESTS
	/* Dumping the thresholded images for reference */
	for (unsigned int i = 0; i < dataset_final.getImagesNo(); ++i) {
		saveThresholdedImage(scoreImages[i],
				     dataset_final.getMask(i),
				     binaryThreshold,
				     params.finalResDir,
				     dataset_final.getImageName(i),
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
#endif /* MOVABLE_TRAIN */

#ifdef MOVABLE_TRAIN
KernelBoost::KernelBoost(std::string &descr_json)
{
	Json::Value root;
	Json::Reader reader;

	if (!reader.parse(descr_json, root)) {
		throw std::runtime_error("invalidJSONDescription");
	}

	Deserialize(root);
}
#else
KernelBoost::KernelBoost(std::string &descr_json,
			 const Parameters &params, const Dataset &dataset)
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

		/* Prepare a vector containing the set of OpenCV matrices
		   corresponding to the available channels */
		std::vector< cv::Mat > chs;
		dataset_final.getChsForImage(i, chs);

		EMat result;
		finalClassifier->classifyImage(chs,
					       dataset_final.getBorderSize(),
					       result);
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
		// 	       normImage);
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
				     dataset_final.getBorderSize());

		end = std::chrono::system_clock::now();
		std::chrono::duration< double > elapsed_s = end-start;
		log_info("\t\tImage %d/%d DONE! (took %.3fs)",
			 i+1, dataset_final.getImagesNo(), elapsed_s.count());
#endif /* TESTS */
	}
}
#endif /* MOVABLE_TRAIN */

void
KernelBoost::Serialize(Json::Value &root)
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
