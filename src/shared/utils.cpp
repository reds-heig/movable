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
#include <random>
#include <iomanip>
#include <vector>
#include <fstream>
#include <numeric>

#include <cassert>
#include <ctime>

#include "DataTypes.hpp"
#include "Dataset.hpp"
#include "logging.hpp"
#include "utils.hpp"

#ifdef MOVABLE_TRAIN

float
computeF1Score(const cv::Mat &scoreImage,
	       const cv::Mat &gt,
	       float threshold)
{
	cv::Mat img;
	cv::threshold(scoreImage, img, threshold,
		      POS_GT_CLASS, cv::THRESH_BINARY);

	/* Perform image opening on the thresholded image */
	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
						    cv::Size(3, 3),
						    cv::Point(1, 1));
	cv::morphologyEx(img, img, cv::MORPH_CLOSE, element);

	float tp = 0;
	float fp = 0;
	float fn = 0;

	for (unsigned int r = 0; r < (unsigned int)img.rows; ++r) {
		for (unsigned int c = 0; c < (unsigned int)img.cols; ++c) {
			if (img.at<float>(r, c) == gt.at<float>(r, c)) {
				++tp;
			}
			if (img.at<float>(r, c) == POS_GT_CLASS &&
			    gt.at<float>(r, c) == NEG_GT_CLASS) {
				++fp;
			}
			if (img.at<float>(r, c) == NEG_GT_CLASS &&
			    gt.at<float>(r, c) == POS_GT_CLASS) {
				++fn;
			}
		}
	}

	/* +1 added to avoid numerical errors (divide-by-zero)! */
	float precision = tp / (tp+fp+1);
	float recall = tp / (tp+fn+1);

	/* This is not the true F1 score -- it is just proportional to it! */
	return precision*recall / (precision+recall+1e-5);
}

float
computeMR(const EMat &img, const EMat &gt, const unsigned int borderSize)
{
	assert(img.rows() == gt.rows());
	assert(img.cols() == gt.cols());

	float err_count = 0;
	for (unsigned int r = borderSize; r < img.rows()-borderSize; ++r) {
		for (unsigned int c = borderSize; c < img.cols()-borderSize; ++c) {
			/* Put a threshold at 0 -- we can do much better! */
			if (img(r, c)*gt(r, c) < 0) {
				err_count += 1;
			}
		}
	}
	return err_count / ((img.rows()-2*borderSize)*(img.cols()-2*borderSize));
}

std::vector< unsigned int >
randomSamplingWithoutReplacement(const unsigned int M,
				 const unsigned int N)
{
	assert (M < N);
	assert (M > 0);

	/* Uniformly-distributed RNG */
	std::random_device rd;
	std::mt19937 randomGenerator(rd());

	/* Index pool */
	std::vector< unsigned int > idxs(N);
	std::iota(idxs.begin(), idxs.end(), 0);

	/* Returned indexes */
	std::vector< unsigned int > vResult(M);

	for (unsigned int i = 0, max = N - 1; i < M; ++i, --max) {
		std::uniform_int_distribution<> uniformDistribution(0, (int)max);
		int index = uniformDistribution(randomGenerator);
		std::swap(idxs[(unsigned int)index], idxs[max]);
		vResult[i] = idxs[max];
	}

	assert (vResult.size() == M);

	return vResult;
}

int
splitSampleSet(const sampleSet &samples,
	       const EVec &Y,
	       const EVec &W,
	       const unsigned int subsetSamplesNo,
	       sampleSet &samples_fl,
	       sampleSet &samples_tree,
	       EVec &Y_fl,
	       EVec &Y_tree,
	       EVec &W_fl,
	       EVec &W_tree)
{
	assert(samples.size() > 2*subsetSamplesNo);

	samples_fl.clear();
	samples_fl.resize(subsetSamplesNo);
	samples_tree.clear();
	samples_tree.resize(subsetSamplesNo);
	Y_fl.resize(subsetSamplesNo);
	Y_tree.resize(subsetSamplesNo);
	W_fl.resize(subsetSamplesNo);
	W_tree.resize(subsetSamplesNo);

	/* Shuffle vector and reserve the first third of the indexes to the tree
	   learning, then sample another third from the remaining indexes
	   according to the weight */
	std::vector< unsigned int > idxs(samples.size());
	std::iota(idxs.begin(), idxs.end(), 0);
	std::random_shuffle(idxs.begin(), idxs.end());

	for (unsigned int i = 0; i < subsetSamplesNo; ++i) {
		samples_tree[i] = samples[idxs[i]];
		Y_tree(i) = Y[idxs[i]];
		W_tree(i) = W[idxs[i]];
	}
	W_tree /= W_tree.array().sum();

	std::vector< unsigned int > posIdxs;
	std::vector< unsigned int > negIdxs;
	std::vector< float > posW;
	std::vector< float > negW;
	float sumPosW = 0;
	float sumNegW = 0;
	for (unsigned int i = subsetSamplesNo; i < samples.size(); ++i) {
		if (samples[idxs[i]].label == POS_GT_CLASS) {
			posIdxs.push_back(idxs[i]);
			posW.push_back(W(idxs[i]));
			sumPosW += W(idxs[i]);
		} else if (samples[idxs[i]].label == NEG_GT_CLASS) {
			negIdxs.push_back(idxs[i]);
			negW.push_back(W(idxs[i]));
			sumNegW += W(idxs[i]);
		} else {
			log_err("Unrecognized label %d (sample %d)",
				samples[idxs[i]].label, idxs[i]);
			return -EXIT_FAILURE;
		}
	}
	/*
	  Randomly sample, in these two pools, according to the weight.
	  WARNING: the returned indexes are indexes in the posIdx/negIdx
	  vectors!
	*/
	const unsigned int posNo = subsetSamplesNo/2;
	const unsigned int negNo = subsetSamplesNo-subsetSamplesNo/2;
	std::vector< unsigned int > idxPosIdxs =
		randomWeightedSamplingWithReplacement(posNo, posW);
	std::vector< unsigned int > idxNegIdxs =
		randomWeightedSamplingWithReplacement(negNo, negW);

	float posWeightsMul = 0;
	float negWeightsMul = 0;
	for (unsigned int i = 0; i < posNo; ++i) {
		samples_fl[i] = samples[posIdxs[idxPosIdxs[i]]];
		W_fl(i) = W(posIdxs[idxPosIdxs[i]]);
		posWeightsMul += W_fl(i);
		Y_fl(i) = Y(posIdxs[idxPosIdxs[i]]);
	}
	for (unsigned int i = 0; i < negNo; ++i) {
		samples_fl[i+posNo] = samples[negIdxs[idxNegIdxs[i]]];
		W_fl(i+posNo) = W(negIdxs[idxNegIdxs[i]]);
		negWeightsMul += W_fl(i+posNo);
		Y_fl(i+posNo) = Y(negIdxs[idxNegIdxs[i]]);
	}

	posWeightsMul = sumPosW / posWeightsMul;
	negWeightsMul = sumNegW / negWeightsMul;

	W_fl.head(posNo) *= posWeightsMul;
	W_fl.tail(negNo) *= negWeightsMul;

	W_fl /= W_fl.array().sum();

	return EXIT_SUCCESS;
}

#endif /* MOVABLE_TRAIN */

bool checkChannelPresent(const std::string &sought,
			 const std::vector< std::string > &chList)
{
	return std::find(chList.begin(), chList.end(),
			 sought) != chList.end();
}

bool
cvMatEquals(const cv::Mat m1, const cv::Mat m2)
{
	cv::Mat tmp;

	if (m1.empty() && m2.empty()) {
		return true;
	}
	if (m1.rows != m2.rows || m1.cols != m2.cols || m1.dims != m2.dims) {
		return false;
	}

	cv::compare(m1, m2, tmp, cv::CMP_NE);
	return cv::countNonZero(tmp) == 0;
}

void
normalizeImage(const EMat &resultImage, const int borderSize,
	       cv::Mat &scoreImage)
{
	/* Convert matrix in OpenCV format */
	cv::Mat img(resultImage.rows(), resultImage.cols(), CV_32FC1);
	cv::eigen2cv(resultImage, img);

	/* Drop border */
	scoreImage = img(cv::Range(borderSize, img.rows-borderSize),
			 cv::Range(borderSize, img.cols-borderSize));

	/* Normalize image in [-1, 1] */
	double min, max;
	cv::minMaxLoc(img, &min, &max);
	if (max-min > 1e-4) {
		img = 2*(img-min)/(max-min)-1;
	} else {
		img.setTo(cv::Scalar(0));
	}
}

void
removeSmallBlobs(cv::Mat& img, float size)
{
	/* Only accept CV_8UC1 */
	if (img.channels() != 1 || img.type() != CV_8U) {
		return;
	}

	/* Find all contours */
	std::vector< std::vector< cv::Point > > contours;
	cv::findContours(img.clone(), contours,
			 CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	for (unsigned int i = 0; i < contours.size(); i++) {
		/* Compute contour area */
		float area = cv::contourArea(contours[i]);

		/* Remove small objects by drawing the contour with black
		   color */
		if (area <= size) {
			cv::drawContours(img, contours, i, CV_RGB(0,0,0), -1);
		}
	}
}

void
saveClassifiedImage(const EMat &classResult,
		    const std::string &dirPath,
		    const std::string &imgName,
		    const unsigned int borderSize)
{
	/* Convert matrix in OpenCV format */
	cv::Mat img(classResult.rows(), classResult.cols(), CV_32FC1);
	cv::eigen2cv(classResult, img);

	/* Drop border */
	img = img(cv::Range(borderSize, img.rows-borderSize),
		  cv::Range(borderSize, img.cols-borderSize));

	/* Normalize image in [0, 255] */
	double min, max;
	cv::minMaxLoc(img, &min, &max);
	if (max-min > 1e-4) {
		img = (img-min)/(max-min)*255;
	} else {
		img.setTo(cv::Scalar(0));
	}
	/* Save resulting image */
	std::string dstPath = dirPath + "/" + imgName;
	cv::imwrite(dstPath.c_str(), img);

	/* Save the raw-format image too */
	EMat roi = classResult.block(borderSize, borderSize,
				     classResult.rows()-2*borderSize,
				     classResult.cols()-2*borderSize);
	dstPath += std::string(".raw");
	std::ofstream file(dstPath);
	if (!file.is_open()) {
		log_err("Unable to open destination file %s", dstPath.c_str());
		return;
	}
	file << roi;
	file.close();
}

void
saveThresholdedImage(const cv::Mat &classResult,
		     const EMat &mask,
		     const float threshold,
		     const std::string &dirPath,
		     const std::string &imgName,
		     const unsigned int borderSize)
{
	cv::Mat tmp;
	/* Apply threshold */
	cv::threshold(classResult, tmp, threshold,
		      255, cv::THRESH_BINARY);

	/* Apply mask */
	cv::Mat c_mask(mask.rows(), mask.cols(), CV_8UC1);
	cv::eigen2cv(mask, c_mask);
	c_mask = c_mask(cv::Range(borderSize, c_mask.rows-borderSize),
			cv::Range(borderSize, c_mask.cols-borderSize));
	cv::bitwise_and(tmp, c_mask, tmp);

	/* Convert to CV_8U and then remove small blobs */
	cv::Mat toClean;
	tmp.convertTo(toClean, CV_8U);
	removeSmallBlobs(toClean, 20);

	/* Perform morphological close */
	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
						    cv::Size(3, 3),
						    cv::Point(1, 1));
	cv::morphologyEx(toClean, toClean, cv::MORPH_CLOSE, element);

	/* Now replace each blob by its convex hull -- this will prevent weird
	   shapes */
	std::vector< std::vector< cv::Point > > contours;
	std::vector< cv::Vec4i > hierarchy;
	cv::findContours(toClean, contours, hierarchy,
			 cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE,
			 cv::Point(0, 0));
	std::vector< std::vector< cv::Point > > hull(contours.size());
	for (unsigned int i = 0; i < contours.size(); ++i) {
		cv::convexHull(cv::Mat(contours[i]), hull[i], false);
	}
	cv::Mat finalBinary = cv::Mat::zeros(toClean.size(), CV_8UC1);
	cv::drawContours(finalBinary, hull, -1, cv::Scalar(255), -1);

	/* Finally, fill all holes with floodfill (idea taken from
           http://www.learnopencv.com/filling-holes-in-an-image-using-opencv-python-c/ */
        cv::Mat im_floodfill = finalBinary.clone();
        cv::Mat im_floodfill_inv;
        cv::floodFill(im_floodfill, cv::Point(0,0), cv::Scalar(255));
        cv::bitwise_not(im_floodfill, im_floodfill_inv);
        finalBinary = (finalBinary | im_floodfill_inv);

	/* Save resulting image */
	std::string dstPath = dirPath + "/" + imgName + "_thresh.png";
	cv::imwrite(dstPath.c_str(), finalBinary);
}

#ifdef MOVABLE_TRAIN
int
createDirectories(Parameters &params, const Dataset &dataset)
#else
int
createDirectories(Parameters &params)
#endif /* MOVABLE_TRAIN */
{
	std::string base_path;
	if (params.resultsDir[0] != '/') {
		/* We have a relative path */
		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			log_err("Cannot get current path");
			return -EXIT_FAILURE;
		}
		base_path += std::string(cwd);
		base_path += "/";
	}
	base_path += std::string(params.resultsDir);

	if (access(base_path.c_str(), F_OK) != 0) {
		log_info("\tCreating base results directory %s",
			 base_path.c_str());
		if (mkdir(base_path.c_str(), 0700) < 0) {
			perror("mkdir");
			return -EXIT_FAILURE;
		}
	}

	base_path += "/";
	base_path += params.simName;

	log_info("\tCreating directory %s", base_path.c_str());
	if (mkdir(base_path.c_str(), 0700) < 0) {
		perror("mkdir");
		return -EXIT_FAILURE;
	}

#ifdef MOVABLE_TRAIN
	params.finalResDir = base_path + "/final_results";
	log_info("\tCreating directory %s", params.finalResDir.c_str());
	if (mkdir(params.finalResDir.c_str(), 0700) < 0) {
		perror("mkdir");
		return -EXIT_FAILURE;
	}

	params.intermedResDir.clear();
	for (unsigned int g = 0; g < dataset.getGtPairsNo(); ++g) {
		params.intermedResDir.push_back(base_path +
						"/" +
						std::to_string(dataset.getGtNegativePairValue(g)) +
						"_" +
						std::to_string(dataset.getGtPositivePairValue(g)));
		log_info("\tCreating directory %s",
			 params.intermedResDir.back().c_str());
		if (mkdir(params.intermedResDir.back().c_str(), 0700) < 0) {
			perror("mkdir");
			return -EXIT_FAILURE;
		}
	}
#endif /* MOVABLE_TRAIN */

	return EXIT_SUCCESS;
}
