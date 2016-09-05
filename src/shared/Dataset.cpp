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
#include <chrono>
#include <ctime>

#include <omp.h>

#include "Dataset.hpp"
#include "DataTypes.hpp"
#include "logging.hpp"
#include "macros.hpp"
#include "utils.hpp"
#include "Parameters.hpp"

#include "BoostedClassifier.hpp"

Dataset::Dataset(const Parameters &params)
	: imagesNo(0)
{
	/* Internal parameters */
	sampleSize = params.sampleSize;
	borderSize = 2*(sampleSize-1);

	if (checkChannelPresent("IMAGE_GRAY_CH", params.channelList)) {
		imageOps.push_back(&Dataset::imageGrayCh);
	}
	if (checkChannelPresent("IMAGE_GREEN_CH", params.channelList)) {
		imageOps.push_back(&Dataset::imageGreenCh);
	}
	if (checkChannelPresent("IMAGE_RED_CH", params.channelList)) {
		imageOps.push_back(&Dataset::imageRedCh);
	}
	if (checkChannelPresent("IMAGE_BLUE_CH", params.channelList)) {
		imageOps.push_back(&Dataset::imageBlueCh);
	}
	if (checkChannelPresent("IMAGE_HUE_CH", params.channelList)) {
		imageOps.push_back(&Dataset::imageHueCh);
	}
	if (checkChannelPresent("IMAGE_SATUR_CH", params.channelList)) {
		imageOps.push_back(&Dataset::imageSaturCh);
	}
	if (checkChannelPresent("IMAGE_VALUE_CH", params.channelList)) {
		imageOps.push_back(&Dataset::imageValueCh);
	}
	if (checkChannelPresent("IMAGE_L_CH", params.channelList)) {
		imageOps.push_back(&Dataset::imageLCh);
	}
	if (checkChannelPresent("IMAGE_A_CH", params.channelList)) {
		imageOps.push_back(&Dataset::imageACh);
	}
	if (checkChannelPresent("IMAGE_B_CH", params.channelList)) {
		imageOps.push_back(&Dataset::imageBCh);
	}
	if (checkChannelPresent("MEDIAN_FILTERING", params.channelList)) {
		imageOps.push_back(&Dataset::medianFiltering);
	}
	if (checkChannelPresent("LAPLACIAN_FILTERING", params.channelList)) {
		imageOps.push_back(&Dataset::laplacianFiltering);
	}
	if (checkChannelPresent("GAUSSIAN_FILTERING", params.channelList)) {
		imageOps.push_back(&Dataset::gaussianFiltering);
	}
	if (checkChannelPresent("SOBEL_DRV_X", params.channelList)) {
		imageOps.push_back(&Dataset::sobelDrvX);
	}
	if (checkChannelPresent("SOBEL_DRV_Y", params.channelList)) {
		imageOps.push_back(&Dataset::sobelDrvY);
	}
	dataChNo = imageOps.size();

	data.resize(dataChNo);

	imgRescaleFactor = params.imgRescaleFactor;
	if (imgRescaleFactor == 0) {
		throw std::runtime_error("Invalid rescale factor: " +
					 std::to_string(imgRescaleFactor));
	}

	houghMinDist = params.houghMinDist;
	houghHThresh = params.houghHThresh;
	houghLThresh = params.houghLThresh;
	houghMinRad = params.houghMinRad;
	houghMaxRad = params.houghMaxRad;
	fastClassifier = params.fastClassifier;
	RBCdetection = params.RBCdetection;

#ifdef MOVABLE_TRAIN
	gtValues = params.gtValues;
	createGtPairs(params.gtValues);
#endif /* MOVABLE_TRAIN */

	/* Load paths */
	std::vector< std::string > img_paths;
	std::vector< std::string > mask_paths;

#ifdef MOVABLE_TRAIN
	std::vector< std::string > gt_paths;

	if (loadPaths(params, img_paths, mask_paths,
		      gt_paths) != EXIT_SUCCESS) {
		throw std::runtime_error("loadPaths");
	}

	if (img_paths.size() != mask_paths.size() ||
	    img_paths.size() != gt_paths.size()) {
		log_err("Inconsistent path lists! img: %d, mask: %d, gt: %d",
			(int)img_paths.size(), (int)mask_paths.size(),
			(int)gt_paths.size());
		throw std::runtime_error("inconsistentPathList");
	}

	/* Pre-allocate structures */
	imagesNo = img_paths.size();
	originalSizes.resize(imagesNo);
	masks.resize(imagesNo);
	originalGts.resize(imagesNo);
	imagePaths.resize(imagesNo);
	ePoints.resize(imagesNo);
	for (unsigned int i = 0; i < imageOps.size(); ++i) {
		data[i].resize(imagesNo);
	}
	for (unsigned int i = 0; i < gtPairsNo; ++i) {
		gts[i].resize(imagesNo);
	}
	/* Load images */
#pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < img_paths.size(); ++i) {
		log_info("\t\tAdding image %d/%d...",
			 (int)i+1, (int)img_paths.size());
		if (addImage(i, img_paths[i], mask_paths[i],
			     gt_paths[i]) != EXIT_SUCCESS) {
			log_err("Error encountered while loading image %d/%d",
				(int)i+1, (int)img_paths.size());
			throw std::runtime_error("imageLoading");
		}
	}
#else
	if (loadPaths(params, img_paths, mask_paths) != EXIT_SUCCESS) {
		throw std::runtime_error("loadPaths");
	}

	if (img_paths.size() != mask_paths.size()) {
		log_err("Inconsistent path lists! img: %d, mask: %d",
			(int)img_paths.size(), (int)mask_paths.size());
		throw std::runtime_error("inconsistentPathList");
	}

	/* Pre-allocate structures */
	imagesNo = img_paths.size();
	originalSizes.resize(imagesNo);
	masks.resize(imagesNo);
	imagePaths.resize(imagesNo);
	ePoints.resize(imagesNo);
	for (unsigned int i = 0; i < imageOps.size(); ++i) {
		data[i].resize(imagesNo);
	}
	/* Load images */
#pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < img_paths.size(); ++i) {
		log_info("\t\tAdding image %d/%d...",
			 (int)i+1, (int)img_paths.size());
		if (addImage(i, img_paths[i], mask_paths[i]) != EXIT_SUCCESS) {
			log_err("Error encountered while loading image %d/%d",
				(int)i+1, (int)img_paths.size());
			throw std::runtime_error("imageLoading");
		}
	}
#endif /* MOVABLE_TRAIN */

}


#ifdef MOVABLE_TRAIN
Dataset::Dataset(
#ifndef TESTS
		 const Parameters &params,
#else
		 const Parameters & /* params */,
#endif /* TESTS */
		 const Dataset &srcDataset,
		 const std::vector< BoostedClassifier * > &boostedClassifiers)
{
	/* Copy values from the source dataset */
	this->data = srcDataset.data;
	this->dataChNo = srcDataset.dataChNo;
	this->imageOps = srcDataset.imageOps;
	this->imagesNo = srcDataset.imagesNo;
	this->imageNames = srcDataset.imageNames;
	this->imagePaths = srcDataset.imagePaths;
	this->sampleSize = srcDataset.sampleSize;
	this->borderSize = srcDataset.borderSize;
	this->masks = srcDataset.masks;
	this->ePoints = srcDataset.ePoints;
	this->fastClassifier = srcDataset.fastClassifier;
	this->RBCdetection = srcDataset.RBCdetection;
	this->originalSizes = srcDataset.originalSizes;
	this->houghMinDist = srcDataset.houghMinDist;
	this->houghHThresh = srcDataset.houghHThresh;
	this->houghLThresh = srcDataset.houghLThresh;
	this->houghMinRad = srcDataset.houghMinRad;
	this->houghMaxRad = srcDataset.houghMaxRad;
	this->gts = srcDataset.gts;
	this->originalGts = srcDataset.originalGts;
	this->gtValues = srcDataset.gtValues;
	this->gtPairValues = srcDataset.gtPairValues;
	this->feedbackImagesFlag = srcDataset.feedbackImagesFlag;

	/* Now iterate on the images, and for each image and each boosted
	   classifier create an additional channel with the obtained result */
	for (unsigned int i = 0; i < boostedClassifiers.size(); ++i) {
		dataVector tmpVec(imagesNo);
		data.push_back(tmpVec);
	}
	log_info("Classifying the images with the learned boosted "
		 "classifiers...");
#pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < imagesNo; ++i) {
#ifndef TESTS
		std::chrono::time_point< std::chrono::system_clock > start;
		std::chrono::time_point< std::chrono::system_clock > end;
		start = std::chrono::system_clock::now();
#endif /* TESTS */

		if (fastClassifier) {
			for (unsigned int bc = 0;
			     bc < boostedClassifiers.size(); ++bc) {
				boostedClassifiers[bc]->classifyImage(*this,
								      i,
								      ePoints[i],
								      data[dataChNo+bc][i]);
#ifndef TESTS
				saveClassifiedImage(data[dataChNo+bc][i],
						    params.intermedResDir[bc],
						    imageNames[i],
						    borderSize);
#endif /* TESTS */
			}
		} else {
			std::vector< cv::Mat > chs;
			getChsForImage(i, chs);
			for (unsigned int bc = 0; bc < boostedClassifiers.size(); ++bc) {
				boostedClassifiers[bc]->classifyFullImage(chs,
									  borderSize,
									  data[dataChNo+bc][i]);
#ifndef TESTS
				saveClassifiedImage(data[dataChNo+bc][i],
						    params.intermedResDir[bc],
						    imageNames[i],
						    borderSize);
#endif /* TESTS */

			}
		}

#ifndef TESTS
		end = std::chrono::system_clock::now();
		std::chrono::duration< double > elapsed_s = end-start;
		log_info("\t\tImage %d/%d DONE! (took %.3fs)",
			 i+1, imagesNo, elapsed_s.count());
#endif /* TESTS */
	}
	dataChNo += boostedClassifiers.size();

	/* Finally, alter the ground-truth by setting as positive class the last
	   gt value and putting the rest as negative class */
	log_info("\nAltering the ground-truth to make the final problem "
		 "binary...\n");
	unsigned int i = 0;
	while (gtPairValues[i].second != gtValues[gtValues.size()-1]) {
		i++;
	}
	gtPairs newGts;
	newGts.push_back(gts[i]);
	std::vector< std::pair< int, int > > newGtPairValues;
	newGtPairValues.push_back(std::pair<int, int> (gtValues[0],
						       gtValues[gtValues.size()-1]));
	std::vector< int > newGtValues;
	newGtValues.push_back(gtValues[0]);
	newGtValues.push_back(gtValues[gtValues.size()-1]);

	gts = newGts;
	gtPairValues = newGtPairValues;
	gtValues = newGtValues;
	gtPairsNo = 1;
#pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < imagesNo; ++i) {
		for (unsigned int r = 0; r < gts[0][i].rows(); ++r) {
			for (unsigned int c = 0; c < gts[0][i].cols(); ++c) {
				if (gts[0][i](r, c) == IGN_GT_CLASS) {
					gts[0][i](r, c) = NEG_GT_CLASS;
				}
			}
		}
	}
}
#else /* !MOVABLE_TRAIN */
Dataset::Dataset(const Dataset &srcDataset,
		 const std::vector< BoostedClassifier * > &boostedClassifiers)
{
	/* Copy values from the source dataset */
	this->data = srcDataset.data;
	this->dataChNo = srcDataset.dataChNo;
	this->imageOps = srcDataset.imageOps;
	this->imagesNo = srcDataset.imagesNo;
	this->imageNames = srcDataset.imageNames;
	this->imagePaths = srcDataset.imagePaths;
	this->sampleSize = srcDataset.sampleSize;
	this->borderSize = srcDataset.borderSize;
	this->masks = srcDataset.masks;
	this->ePoints = srcDataset.ePoints;
	this->originalSizes = srcDataset.originalSizes;
	this->houghMinDist = srcDataset.houghMinDist;
	this->houghHThresh = srcDataset.houghHThresh;
	this->houghLThresh = srcDataset.houghLThresh;
	this->houghMinRad = srcDataset.houghMinRad;
	this->houghMaxRad = srcDataset.houghMaxRad;

	/* Now iterate on the images, and for each image and each boosted
	   classifier create an additional channel with the obtained result */
	for (unsigned int i = 0; i < boostedClassifiers.size(); ++i) {
		dataVector tmpVec(imagesNo);
		data.push_back(tmpVec);
	}
	log_info("Classifying the images with the learned boosted "
		 "classifiers...");
#pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < imagesNo; ++i) {
#ifndef TESTS
		std::chrono::time_point< std::chrono::system_clock > start;
		std::chrono::time_point< std::chrono::system_clock > end;
		start = std::chrono::system_clock::now();
#endif /* TESTS */

		if (fastClassifier) {
			for (unsigned int bc = 0;
			     bc < boostedClassifiers.size(); ++bc) {
				boostedClassifiers[bc]->classifyImage(*this,
								      i,
								      ePoints[i],
								      data[dataChNo+bc][i]);
			}
		} else {
			std::vector< cv::Mat > chs;
			getChsForImage(i, chs);
			for (unsigned int bc = 0; bc < boostedClassifiers.size(); ++bc) {
				boostedClassifiers[bc]->classifyFullImage(chs,
									  borderSize,
									  data[dataChNo+bc][i]);
			}
		}

#ifndef TESTS
		end = std::chrono::system_clock::now();
		std::chrono::duration< double > elapsed_s = end-start;
		log_info("\t\tImage %d/%d DONE! (took %.3fs)",
			 i+1, imagesNo, elapsed_s.count());
#endif /* TESTS */
	}
	dataChNo += boostedClassifiers.size();
}

#endif /* MOVABLE_TRAIN */

#ifdef MOVABLE_TRAIN
const EMat&
Dataset::getGt(const int pairNo,
	       const unsigned int imageNo) const
{
	if (pairNo < 0 || pairNo >= (int)gtPairsNo ||
	    imageNo >= imagesNo) {
		log_err("The requested gt %d does not exist in "
			"pair %d (limits: image = %d, pair = %d)",
			imageNo, pairNo, imagesNo-1, gtPairsNo-1);
		/* Return an empty EMat */
		static EMat nullresult;
		return nullresult;
	}
	return gts[pairNo][imageNo];
}

int
Dataset::getGtNegativePairValue(const unsigned int gtPair) const
{
	if (gtPair >= gtPairsNo) {
		log_err("The desired gt pair (%d) does not exist",
			gtPair);
		return -1;
	}
	return gtPairValues[gtPair].first;
}

int
Dataset::getGtPositivePairValue(const unsigned int gtPair) const
{
	if (gtPair >= gtPairsNo) {
		log_err("The desired gt pair (%d) does not exist",
			gtPair);
		return -1;
	}
	return gtPairValues[gtPair].second;
}

const gtVector&
Dataset::getGtVector(const int pairNo) const
{
	if (pairNo >= (int)gtPairsNo || pairNo < 0) {
		log_err("The requested gt vector does not exist (pair "
			"= %d, max value = %d)", pairNo, gtPairsNo-1);
		/* Return an empty vector */
		static gtVector nullresult;
		return nullresult;
	}
	return gts[pairNo];
}

const EMat&
Dataset::getOriginalGt(const unsigned int imageNo) const
{
	if (imageNo >= imagesNo) {
		log_err("The requested original gt (%d) does not exist",
			imageNo);
		/* Return an empty EMat */
		static EMat nullresult;
		return nullresult;
	}
	return originalGts[imageNo];
}

bool
Dataset::isFeedbackImage(const int imageNo) const
{
	assert(imageNo >= 0);
	assert(imageNo < (int)imagesNo);

	return feedbackImagesFlag[imageNo];
}

unsigned int
Dataset::getGtPairsNo() const
{
	return gtPairsNo;
}

unsigned int
Dataset::getGtValuesNo() const
{
	return gtValues.size();
}

#endif /* MOVABLE_TRAIN */

void Dataset::getSampleMatrix(const sampleSet &samplePositions,
			      const std::vector< unsigned int > &samplesIdx,
			      const unsigned int chNo,
			      const unsigned int rowOffset,
			      const unsigned int colOffset,
			      const unsigned int size,
			      EMat &samples) const
{
	assert (chNo < dataChNo);

	const unsigned int samplesNo = samplesIdx.size();
	const unsigned int sampleArea = size*size;

	samples.resize(samplesNo, sampleArea);
	for (unsigned int iX = 0; iX < samplesNo; ++iX) {
		const samplePos &s = samplePositions[samplesIdx[iX]];
		EMat tmp = data[chNo][s.imageNo].block(s.row+rowOffset,
						       s.col+colOffset,
						       size, size);
		samples.row(iX) = Eigen::Map< EMat >(tmp.data(), 1,
						     sampleArea);
	}
}

void
Dataset::shrinkSamplePositions(sampleSet &samplePositions,
			       const unsigned int desiredSize)
{
	assert(desiredSize > 0);
	assert(desiredSize <= samplePositions.size());

	std::vector< unsigned int > newSamplesPos =
		randomSamplingWithoutReplacement(desiredSize,
						 samplePositions.size());

	sampleSet newSamples(desiredSize);

	for (unsigned int i = 0; i < desiredSize; ++i) {
		newSamples[i] = samplePositions[newSamplesPos[i]];
	}

	samplePositions = newSamples;
}

unsigned int
Dataset::getBorderSize() const
{
	return borderSize;
}

void
Dataset::getChsForImage(const unsigned int n, std::vector< cv::Mat > &chs) const
{
	chs.clear();
	for (unsigned int ch = 0; ch < dataChNo; ++ch) {
		cv::Mat img(data[ch][n].rows(),
			    data[ch][n].cols(),
			    CV_32FC1);
		cv::eigen2cv(data[ch][n], img);
		cv::copyMakeBorder(img, img,
				   borderSize, borderSize,
				   borderSize, borderSize,
				   cv::BORDER_REPLICATE);
		chs.push_back(img);
	}
}

const EMat&
Dataset::getData(const unsigned int channelNo,
		 const unsigned int imageNo) const
{
	if (channelNo >= dataChNo || imageNo >= imagesNo) {
		log_err("The requested image %d does not exist in "
			"channel %d (limits: image = %d, channel = %d)",
			imageNo, channelNo, imagesNo-1, dataChNo-1);
		/* Return an empty EMat */
		static EMat nullresult;
		return nullresult;
	}
	return data[channelNo][imageNo];
}

unsigned int
Dataset::getDataChNo() const
{
	return dataChNo;
}

const dataVector&
Dataset::getDataVector(const unsigned int channelNo) const
{
	if (channelNo >= dataChNo) {
		log_err("The requested data vector does not exist "
			"channel = %d, max value = %d)",
			channelNo, dataChNo-1);
		/* Return an empty vector if the wrong channel number is
		   requested */
		static dataVector nullresult;
		return nullresult;
	}
	return data[channelNo];
}

const sampleSet&
Dataset::getEPoints(const unsigned int imageNo) const
{
	if (imageNo >= imagesNo) {
		log_err("The requested image %d does not exist "
			"(limit: image = %d)",
			imageNo, imagesNo-1);
		/* Return an empty sampleSet */
		static sampleSet nullresult;
		return nullresult;
	}
	return ePoints[imageNo];
}

std::string
Dataset::getImageName(const unsigned int imageNo) const
{
	if (imageNo >= imagesNo) {
		log_err("The requested image %d does not exist",
			imageNo);
		return std::string();
	}
	return imageNames[imageNo];
}

std::string
Dataset::getImagePath(const unsigned int imageNo) const
{
	if (imageNo >= imagesNo) {
		log_err("The requested image %d does not exist",
			imageNo);
		return std::string();
	}
	return imagePaths[imageNo];
}

unsigned int
Dataset::getImagesNo() const
{
	return imagesNo;
}

const EMat&
Dataset::getMask(const unsigned int imageNo) const
{
	if (imageNo >= imagesNo) {
		log_err("The requested image %d does not exist "
			"(limit: image = %d)",
			imageNo, imagesNo-1);
		/* Return an empty EMat */
		static EMat nullresult;
		return nullresult;
	}
	return masks[imageNo];
}

std::pair< int, int >
Dataset::getOriginalImgSize(const unsigned int imgNo) const
{
	if (imgNo >= imagesNo) {
		log_err("The requested image %u does not exist "
			"(limit: image = %u)",
			imgNo, imagesNo-1);
		throw std::runtime_error("invalidRequest");
	}
	if (imgNo >= originalSizes.size()) {
		log_err("Not enough image sizes were pushed (%lu, "
			"request is for %u",
			originalSizes.size(), imgNo);
		throw std::runtime_error("invalidRequest");
	}
	return originalSizes[imgNo];
}

unsigned int
Dataset::getSampleSize() const
{
	return sampleSize;
}

void
Dataset::computeCandidatePointsMask(const unsigned int imageID,
				    const cv::Mat& colorImg,
				    const cv::Mat& mask)
{
	cv::Mat grayImg;
	cv::cvtColor(colorImg, grayImg, CV_BGR2GRAY);

	cv::Mat labImg;
	cv::cvtColor(colorImg, labImg, cv::COLOR_BGR2Lab);

	std::vector< cv::Mat > VLAB(3);
	cv::split(labImg, VLAB);

	int histSize = 256;
	float range[ ] = { 0, 256 } ;
	const float *histRange = { range };
	bool uniform = true;
	bool accumulate = false;

	cv::Mat l_hist, b_hist;
	cv::calcHist(&VLAB[0], 1, 0, cv::Mat(), l_hist, 1,
		     &histSize, &histRange, uniform, accumulate);
	cv::calcHist(&VLAB[2], 1, 0, cv::Mat(), b_hist, 1,
		     &histSize, &histRange, uniform, accumulate);

	double minTmp;
	double maxL;
	double maxB;
	cv::Point pminB;
	cv::Point pmaxB;
	minMaxLoc(l_hist, &minTmp, &maxL);
	minMaxLoc(b_hist, &minTmp, &maxB, &pminB, &pmaxB);

	/* Get the two threshold points on the histograms */
	int th_L=-1;
	int th_b=-1;

	for (int i = 0; i < histSize-1; ++i) {
		// fprintf(stderr, "%d: L=%f (%f), b=%f (%f)\n", i,
	    	//     l_hist.at<float>(0, i), l_hist.at<float>(0, i)/maxL,
	    	//     b_hist.at<float>(0, i), b_hist.at<float>(0, i)/maxB);
		if (th_L < 0 &&
		    l_hist.at<float>(0, i)/maxL >= L_THRESHOLD_INIT &&
		    (i < histSize-3 &&
		     l_hist.at< float >(0, i) < l_hist.at< float >(0, i+1) &&
		     l_hist.at< float >(0, i) < l_hist.at< float >(0, i+2) &&
		     l_hist.at< float >(0, i+2)/maxL > L_THRESHOLD_NEXT)) {
			th_L = i;
		}
	}

	unsigned int j = pmaxB.y;
	while (b_hist.at< float >(0, j)/maxB > B_THRESHOLD) {
		j--;
	}
	th_b = j+1;

	// fprintf(stderr, "L: %d, b: %d\n", th_L, th_b);

	if (th_L < 0)
		th_L = 255;
	if (th_b < 0)
		th_b = 255;

	cv::threshold(VLAB[0], VLAB[0], th_L, 255, cv::THRESH_BINARY_INV);
	cv::threshold(VLAB[2], VLAB[2], th_b, 255, cv::THRESH_BINARY_INV);

	cv::Mat thresholdedBinImage(colorImg.size(), CV_8UC1);
	thresholdedBinImage = cv::Scalar(0);

	for (int r = 0; r < colorImg.rows; ++r) {
		for (int c = 0; c < colorImg.cols; ++c) {
			if (VLAB[0].at< uchar >(r,c) > 0 ||
			    VLAB[2].at< uchar >(r,c) > 0) {
				thresholdedBinImage.at< uchar >(r,c) = 255;
			}
		}
	}

	cv::Mat eroded;
	// cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
	// 					    cv::Size(ERODE_SIZE,
	// 						     ERODE_SIZE));
	// cv::erode(thresholdedBinImage, eroded, element);

	eroded = thresholdedBinImage.clone();

	std::vector< std::vector< cv::Point > > contoursH;
	std::vector< cv::Vec4i > hierarchyH;
	cv::findContours(eroded, contoursH, hierarchyH,
			 cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE,
			 cv::Point(0, 0));
	std::vector< std::vector< cv::Point > > hull(contoursH.size());
	for (unsigned int i = 0; i < contoursH.size(); ++i) {
		cv::convexHull(cv::Mat(contoursH[i]), hull[i], false);
	}
	cv::Mat removedWBC = cv::Mat::zeros(eroded.size(), CV_8UC1);
	cv::drawContours(removedWBC, hull, -1, cv::Scalar(255), -1);

	// cv::Mat removedWBC = eroded.clone();
	cv::Mat dst(removedWBC.size(), CV_8UC1);
	dst = cv::Scalar(0);

	cv::Mat im_floodfill = removedWBC.clone();
	cv::Mat im_floodfill_inv;
	cv::floodFill(im_floodfill, cv::Point(0,0), cv::Scalar(255));
	cv::bitwise_not(im_floodfill, im_floodfill_inv);

	removedWBC = (removedWBC | im_floodfill_inv);

	std::vector< std::vector< cv::Point > > contours;
	std::vector< cv::Vec4i > hierarchy;
	cv::Mat ctrSearchImg = removedWBC.clone();
	cv::findContours(ctrSearchImg, contours, hierarchy,
			 cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE );
	double maxArea = houghMaxRad*houghMaxRad*3.14;

	int idx = 0;
	for ( ; idx >= 0; idx = hierarchy[idx][0]) {
		const std::vector< cv::Point >& c = contours[idx];
		double area = fabs(cv::contourArea(cv::Mat(c)));
		if (area < maxArea) {
			cv::drawContours(dst,
					 contours,
					 idx,
					 cv::Scalar(255),
					 cv::FILLED,
					 cv::LINE_8,
					 hierarchy);
		}
	}

	if (RBCdetection) {
		/* Detect RBCs on smoothed grayscale image */
		cv::medianBlur(grayImg, grayImg, M_BLUR_SIZE);

		std::vector< cv::Vec3f > RBCs;
		cv::HoughCircles(grayImg, RBCs, cv::HOUGH_GRADIENT, 1,
				 houghMinDist,
				 houghHThresh, houghLThresh,
				 houghMinRad, houghMaxRad);

		cv::Mat dstRBC(dst.rows, dst.cols, CV_8U);
		dstRBC = cv::Scalar(0);

		for (size_t i = 0; i < RBCs.size(); ++i) {
			cv::Vec3i c = RBCs[i];
			cv::Mat tmp(dst.rows, dst.cols, CV_8U);
			tmp = cv::Scalar(0);
			cv::circle(tmp, cv::Point(c[0], c[1]),
				   c[2], cv::Scalar(255),
				   -1, cv::LINE_8);

			cv::Mat roi_rbc = tmp(cv::Range(std::max(c[1]-c[2],
								 0),
							std::min(c[1]+c[2],
								 dst.rows-1)),
					      cv::Range(std::max(c[0]-c[2],
								 0),
							std::min(c[0]+c[2],
								 dst.cols-1)));
			cv::Mat roi_detection = dst(cv::Range(std::max(c[1]-c[2],
									  0),
								 std::min(c[1]+c[2],
									  dst.rows-1)),
						       cv::Range(std::max(c[0]-c[2],
									  0),
								 std::min(c[0]+c[2],
									  dst.cols-1)));
			cv::Mat roi_region;
			cv::bitwise_and(roi_rbc, roi_detection, roi_region);
			if (cv::countNonZero(roi_region) > 0) {
				cv::circle(dstRBC,
					   cv::Point(c[0], c[1]),
					   c[2]+(houghMaxRad/10),
					   cv::Scalar(255),
					   -1,
					   cv::LINE_8);
			}
		}

		// cv::namedWindow("grayImg", CV_WINDOW_NORMAL);
		// cv::imshow("grayImg", grayImg);

		// cv::namedWindow("dst", CV_WINDOW_NORMAL);
		// cv::imshow("dst", dst);

		// cv::namedWindow("dstRBC", CV_WINDOW_NORMAL);
		// cv::imshow("dstRBC", dstRBC);

		// cv::waitKey();

		dst = dstRBC;
	}
	//
	// cv::namedWindow("inColor", CV_WINDOW_NORMAL);
	// cv::imshow("inColor", colorImg);
	// // cv::namedWindow("grayColor", CV_WINDOW_NORMAL);
	// // cv::imshow("grayColor", grayImg);
	// cv::namedWindow("thresholdedBinImage", CV_WINDOW_NORMAL);
	// cv::imshow("thresholdedBinImage", thresholdedBinImage);
	// cv::namedWindow("er", CV_WINDOW_NORMAL);
	// cv::imshow("er", eroded);
	// /* Mask out incomplete RBCs on the border */
	// /* Mask out replicated border*/

	cv::resize(dst, dst, cv::Size(mask.cols, mask.rows), 0, 0,
		   cv::INTER_NEAREST);
	cv::bitwise_and(dst, mask, dst);

	cv::copyMakeBorder(dst, dst,
			   borderSize, borderSize, borderSize, borderSize,
			   cv::BORDER_CONSTANT, cv::Scalar(0));

	// cv::Mat replicated;
	// cv::cvtColor(dst, replicated, CV_GRAY2BGR);
	// cv::Mat overlayed;
	// cv::Mat enlargedColorImg;
	// cv::resize(colorImg, enlargedColorImg,
	// 	   cv::Size(mask.cols, mask.rows), 0, 0,
	// 	   cv::INTER_NEAREST);
	// cv::copyMakeBorder(enlargedColorImg, enlargedColorImg,
	// 		   borderSize, borderSize, borderSize, borderSize,
	// 		   cv::BORDER_REPLICATE);

	// cv::copyMakeBorder(enlargedColorImg, enlargedColorImg,
	// 		   borderSize, borderSize, borderSize, borderSize,
	// 		   cv::BORDER_REPLICATE);
	// cv::addWeighted(enlargedColorImg, 0.4, replicated, 0.6, 0.0, overlayed);
	// cv::namedWindow("overlayed", CV_WINDOW_NORMAL);
	// cv::imshow("overlayed", overlayed);
	// int cnt = 0;
	// for (int r = 0; r < dst.rows; ++r) {
	// 	for (int c = 0; c < dst.cols; ++c) {
	// 		if (dst.at<uchar>(r,c) > 0) {
	// 			++cnt;
	// 		}
	// 	}
	// }
	// fprintf(stderr, "Have %d whites instead of %d\n",
	// 	cnt, dst.rows*dst.cols);
	// cv::waitKey();

	sampleSet eDst;
	for (int r = 0; r < dst.rows; ++r) {
		for (int c = 0; c < dst.cols; ++c) {
			if (dst.at< uchar >(r, c) > 0) {
				eDst.push_back(samplePos(imageID, r, c, 0));
			}
		}
	}

	ePoints[imageID] = eDst;
}

#ifdef MOVABLE_TRAIN

int
Dataset::addGt(const unsigned int imageID, const cv::Mat &src)
{
	/* Enlarge the original GT image and store it */
	cv::Mat oSrc;
	cv::copyMakeBorder(src, oSrc,
			   borderSize, borderSize, borderSize, borderSize,
			   cv::BORDER_CONSTANT, IGN_GT_CLASS);
	EMat oTmp(oSrc.rows, oSrc.cols);
	cv::cv2eigen(oSrc, oTmp);
	originalGts[imageID] = oTmp;

	for (unsigned int i = 0; i < gtPairsNo; ++i) {
		cv::Mat tmp(src.rows, src.cols, CV_32FC1);
		tmp = IGN_GT_CLASS;

		/* Scan each pixel and set the GT value accordingly */
		for (unsigned int r = 0; r < (unsigned int)src.rows; ++r) {
			for (unsigned int c = 0; c < (unsigned int)src.cols; ++c) {
				if (src.at< float >(r, c) == gtPairValues[i].first) {
					tmp.at< float >(r, c) = NEG_GT_CLASS;
				}

				if (src.at< float >(r, c) == gtPairValues[i].second) {
					tmp.at< float >(r, c) = POS_GT_CLASS;
				}
			}
		}

		cv::copyMakeBorder(tmp, tmp,
				   borderSize, borderSize, borderSize, borderSize,
				   cv::BORDER_CONSTANT, IGN_GT_CLASS);

		EMat eTmp(tmp.rows, tmp.cols);
		cv::cv2eigen(tmp, eTmp);
		gts[i][imageID] = eTmp;

#ifdef VISUALIZE_IMG_DATA
		cv::namedWindow("InGt", cv::WINDOW_NORMAL);
		cv::imshow("InGt", src);
		cv::imwrite("/home/rob/src.png", tmp);
		cv::namedWindow("OutGt", cv::WINDOW_NORMAL);
		cv::imshow("OutGt", tmp);
		cv::imwrite("/home/rob/out.png", tmp);
		cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */
	}

	return EXIT_SUCCESS;
}
#endif /* MOVABLE_TRAIN */

#ifdef MOVABLE_TRAIN
int
Dataset::addImage(const unsigned int imageID,
		  const std::string &imgPath,
		  const std::string &maskPath,
		  const std::string &gtPath)
#else /* !MOVABLE_TRAIN */
int
Dataset::addImage(const unsigned int imageID,
		  const std::string &imgPath,
		  const std::string &maskPath)
#endif /* MOVABLE_TRAIN */
{
	CHECK_FILE_EXISTS(imgPath.c_str());
	CHECK_FILE_EXISTS(maskPath.c_str());
#ifdef MOVABLE_TRAIN
	CHECK_FILE_EXISTS(gtPath.c_str());
#endif /* MOVABLE_TRAIN */

	/* Groundtruth and mask are assumed to be grayscale */
	cv::Mat mask = cv::imread(maskPath.c_str(), CV_LOAD_IMAGE_GRAYSCALE);

	/* Rescale mask. Its size will be used to alter the size of the input
	   image and the gt */
	originalSizes[imageID] = std::make_pair(mask.rows, mask.cols);

	cv::resize(mask, mask, cv::Size(0, 0),
		   1.0/(double)imgRescaleFactor,
		   1.0/(double)imgRescaleFactor,
		   cv::INTER_NEAREST);

	cv::Mat tmp;
	mask.convertTo(tmp, CV_32FC1);
	addMask(imageID, tmp);

	imagePaths[imageID] = imgPath;
	/* Always read a color image, it will be the loop over the operations
	   that will grab the different component and eventually keep the
	   grayscale one only */
	cv::Mat img = cv::imread(imgPath.c_str(), CV_LOAD_IMAGE_COLOR);
	/* Use the image as a source to the algorithm that finds the candidate
	   points for classification and stores them in a mask that
	   will be later used for the actual classification */
	if (fastClassifier) {
		computeCandidatePointsMask(imageID, img, mask);
	}

	cv::resize(img, img, cv::Size(mask.cols, mask.rows), 0, 0,
		   cv::INTER_LANCZOS4);

	/* Replicate image borders */
	cv::copyMakeBorder(img, img,
			   borderSize, borderSize, borderSize, borderSize,
			   cv::BORDER_REPLICATE);

	/* Convert image to float and rescale it in [0, 1] */
	img.convertTo(img, CV_32FC3);
	img = img/255;

	/* Now alter the image according to user specifications, creating the
	   corresponding channels */
	for (unsigned int i = 0; i < imageOps.size(); ++i) {
		EMat tmp;
		imageOps[i](img, tmp, (void *)&borderSize);
		data[i][imageID] = tmp;
	}

	/* Check that all sizes are consistent */
	const unsigned int rowsNo = masks[imageID].rows();
	const unsigned int colsNo = masks[imageID].cols();

	for (unsigned int i = 0; i < imageOps.size(); ++i) {
		if (data[i][imageID].rows() != rowsNo ||
		    data[i][imageID].cols() != colsNo) {
			log_err("Invalid image size -- image %d, channel %d",
				imageID+1, i);
			return -EXIT_FAILURE;
		}
	}

#ifdef MOVABLE_TRAIN
	cv::Mat gt = cv::imread(gtPath.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
	gt.convertTo(tmp, CV_32FC1);
	cv::resize(tmp, tmp, cv::Size(mask.cols, mask.rows), 0, 0,
		   cv::INTER_NEAREST);
	addGt(imageID, tmp);
	for (unsigned int i = 0; i < gtPairsNo; ++i) {
		if (gts[i][imageID].rows() != rowsNo ||
		    gts[i][imageID].cols() != colsNo) {
			log_err("Invalid GT size -- image %d, gt pair %d",
				imageID+1, i);
			return -EXIT_FAILURE;
		}
	}
#endif /* MOVABLE_TRAIN */

	return EXIT_SUCCESS;
}

#ifdef MOVABLE_TRAIN
unsigned int
Dataset::collectAllSamplePositions(const gtVector &gt,
				   const int sampleClass,
				   std::vector< sampleSet > &availableSamples,
				   std::vector< int > &samplesPerImageNo) const
{
	availableSamples.resize(imagesNo);
	samplesPerImageNo.resize(imagesNo);

#pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < imagesNo; ++i) {
		/* Local aliases to ease manipulations */
		const EMat &gtImg = gt[i];
		const EMat &maskImg = masks[i];
		sampleSet &samples = availableSamples[i];
		samples.clear();

		/* It is not worth doing the multiplication of the two matrices
		   just to do a single if-comparison, possibly cheaper to do
		   perform the two individual comparisons */
		for (unsigned int r = 0; r < (unsigned int)gtImg.rows(); ++r) {
			for (unsigned int c = 0;
			     c < (unsigned int)gtImg.cols(); ++c) {
				if (maskImg(r, c) == MASK_INCLUDED &&
				    gtImg(r, c) == sampleClass) {
					samples.push_back(samplePos(i, r, c,
								    sampleClass));
				}
			}
		}
		samplesPerImageNo[i] = samples.size();
	}

	/* Count the total number of available samples */
	unsigned int availableSamplesNo = 0;
	for (unsigned int i = 0; i < imagesNo; ++i) {
		availableSamplesNo += samplesPerImageNo[i];
	}

	return availableSamplesNo;
}

void
Dataset::createGtPairs(std::vector< int > gtValues)
{
	gtPairValues.clear();
	gts.clear();

	/* Remove duplicates */
	std::sort(gtValues.begin(), gtValues.end());
	auto last = std::unique(gtValues.begin(), gtValues.end());
	gtValues.erase(last, gtValues.end());

	for (unsigned int i = 0; i < gtValues.size()-1; ++i) {
		for (unsigned int j = i+1; j < gtValues.size(); ++j) {
			gtPairValues.push_back(std::pair< int, int >(gtValues[i],
								     gtValues[j]));
		}
	}
	gtPairsNo = gtPairValues.size();
	gts.resize(gtPairsNo);
}

int
Dataset::getSamplePositions(const int sampleClass,
			    const unsigned int gtPair,
			    const unsigned int samplesNo,
			    sampleSet &samplePositions) const
{
	if (sampleClass != POS_GT_CLASS && sampleClass != NEG_GT_CLASS) {
		return -EXIT_FAILURE;
	}

	return getAvailableSamples(gts[(unsigned int)gtPair],
				   sampleClass,
				   samplesNo,
				   samplePositions);
}

int
Dataset::getAvailableSamples(const gtVector &gt,
			     const int sampleClass,
			     const unsigned int samplesNo,
			     sampleSet &samplePositions) const
{
	/* Empty the sample set first, to guard against users not checking the
	   return value */
	samplePositions.clear();

	/* Per-image available samples */
	std::vector< sampleSet > availableSamples;
	std::vector< int > samplesPerImageNo;
	const unsigned int availableSamplesNo =
		collectAllSamplePositions(gt,
					  sampleClass,
					  availableSamples,
					  samplesPerImageNo);

	const unsigned int returnedSamplesNo =
		samplesNo <= availableSamplesNo ? samplesNo : availableSamplesNo;
	samplePositions.resize(returnedSamplesNo);

	/* Grab the individual samples from images, sampling according to the
	   distribution over the images (that is, more elements of the requested
	   class in an image will provoke more sampling from that image) */
	std::vector< unsigned int > randSamples =
		randomWeightedSamplingWithReplacement(returnedSamplesNo,
						      samplesPerImageNo);
	for (unsigned int i = 0; i < returnedSamplesNo; ++i) {
		const unsigned int imgNo = randSamples[i];
		if (samplesPerImageNo[imgNo] > 0) {
			samplePositions[i] = availableSamples[imgNo]
				[(unsigned int)rand() % samplesPerImageNo[imgNo]];
		} else {
			/* Cannot get a sample from this image, resample */
			--i;
		}
	}
	return (int)returnedSamplesNo;
}

#endif /* MOVABLE_TRAIN */

#ifdef MOVABLE_TRAIN
int
Dataset::loadPaths(const Parameters &params,
		   std::vector< std::string > &img_paths,
		   std::vector< std::string > &mask_paths,
		   std::vector< std::string > &gt_paths)
#else /* !MOVABLE_TRAIN */
int
Dataset::loadPaths(const Parameters &params,
		   std::vector< std::string > &img_paths,
		   std::vector< std::string > &mask_paths)
#endif /* MOVABLE_TRAIN */
{
	if (loadPathFile(params.datasetPath, params.imgPathsFName,
			 img_paths) != EXIT_SUCCESS) {
		log_err("Error encountered while loading IMG paths file");
		return -EXIT_FAILURE;
	}
	/* Push image names */
	imageNames.clear();
	for (unsigned int i = 0; i < img_paths.size(); ++i) {
		std::string imgName = std::string(basename(img_paths[i].c_str()));
		std::string tmpName = imgName;
		int count = 0;
		while (std::find(imageNames.begin(), imageNames.end(),
				 tmpName) != imageNames.end()) {
			tmpName = imgName + "_" + std::to_string(count);
			count++;
		}
		imageNames.push_back(tmpName);
	}

	/* Check for feedback images */
	feedbackImagesFlag.clear();
	feedbackImagesFlag.resize(img_paths.size(), false);
	for (unsigned int i = 0; i < img_paths.size(); ++i) {
		if(img_paths[i].substr(0, 2) == "* ") {
			log_info("\tImage %d (%s) is marked as feedback!",
				 i, imageNames[i].c_str());
			feedbackImagesFlag[i] = true;
			img_paths[i] = img_paths[i].substr(2);
		}
	}

	if (loadPathFile(params.datasetPath, params.maskPathsFName,
			 mask_paths) != EXIT_SUCCESS) {
		log_err("Error encountered while loading MASK paths file");
		return -EXIT_FAILURE;
	}

#ifdef MOVABLE_TRAIN
	if (loadPathFile(params.datasetPath, params.gtPathsFName,
			 gt_paths) != EXIT_SUCCESS) {
		log_err("Error encountered while loading GT paths file");
		return -EXIT_FAILURE;
	}
#endif /* MOVABLE_TRAIN */

	return EXIT_SUCCESS;
}

int
Dataset::imageGrayCh(const cv::Mat &src, EMat &dst, const void *opaque)
{
	cv::Mat gray;
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::cvtColor(src, gray, CV_BGR2GRAY);
	cv::Mat imgCenter = gray(cv::Range(borderSize, gray.rows-borderSize+1),
				 cv::Range(borderSize, gray.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	gray -= mean[0];
	gray /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(gray.rows, gray.cols);
	cv::cv2eigen(gray, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("grayCh", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(gray, &min, &max);
	if (max-min > 1e-4) {
		gray = (gray-min)/(max-min);
	} else {
		gray.setTo(cv::Scalar(0));
	}
	cv::imshow("grayCh", gray);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::imageGreenCh(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> colorCh(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::split(src, colorCh);

	cv::Mat greenCh = colorCh[1];
	cv::Mat imgCenter = greenCh(cv::Range(borderSize,
					      greenCh.rows-borderSize+1),
				    cv::Range(borderSize,
					      greenCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	greenCh -= mean[0];
	greenCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(greenCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("greenCh", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(greenCh, &min, &max);
	if (max-min > 1e-4) {
		greenCh = (greenCh-min)/(max-min);
	} else {
		greenCh.setTo(cv::Scalar(0));
	}
	cv::imshow("greenCh", greenCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::imageRedCh(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> colorCh(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::split(src, colorCh);

	cv::Mat redCh = colorCh[2];

	cv::Mat imgCenter = redCh(cv::Range(borderSize,
					    redCh.rows-borderSize+1),
				  cv::Range(borderSize,
					    redCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	redCh -= mean[0];
	redCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(redCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("RED", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(redCh, &min, &max);
	if (max-min > 1e-4) {
		redCh = (redCh-min)/(max-min);
	} else {
		redCh.setTo(cv::Scalar(0));
	}
	cv::imshow("RED", redCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::imageBlueCh(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> colorCh(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::split(src, colorCh);

	cv::Mat blueCh = colorCh[0];

	cv::Mat imgCenter = blueCh(cv::Range(borderSize,
					     blueCh.rows-borderSize+1),
				   cv::Range(borderSize,
					     blueCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	blueCh -= mean[0];
	blueCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(blueCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("BLUE", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(blueCh, &min, &max);
	if (max-min > 1e-4) {
		blueCh = (blueCh-min)/(max-min);
	} else {
		blueCh.setTo(cv::Scalar(0));
	}
	cv::imshow("BLUE", blueCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::imageHueCh(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> hsv_channels(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::Mat hsv_image;
	cv::cvtColor(src, hsv_image, cv::COLOR_BGR2HSV);
	cv::split(hsv_image, hsv_channels);

	cv::Mat hueCh = hsv_channels[0];

	cv::Mat imgCenter = hueCh(cv::Range(borderSize,
					    hueCh.rows-borderSize+1),
				  cv::Range(borderSize,
					    hueCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	hueCh -= mean[0];
	hueCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(hueCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("HUE", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(hueCh, &min, &max);
	if (max-min > 1e-4) {
		hueCh = (hueCh-min)/(max-min);
	} else {
		hueCh.setTo(cv::Scalar(0));
	}
	cv::imshow("HUE", hueCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::imageLCh(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> lab_channels(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::Mat lab_image;
	cv::cvtColor(src, lab_image, cv::COLOR_BGR2Lab);
	cv::split(lab_image, lab_channels);

	cv::Mat LCh = lab_channels[0];

	cv::Mat imgCenter = LCh(cv::Range(borderSize,
					  LCh.rows-borderSize+1),
				cv::Range(borderSize,
					  LCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	LCh -= mean[0];
	LCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(LCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("L", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(LCh, &min, &max);
	if (max-min > 1e-4) {
		LCh = (LCh-min)/(max-min);
	} else {
		LCh.setTo(cv::Scalar(0));
	}
	cv::imshow("L", LCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::imageACh(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> lab_channels(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::Mat lab_image;
	cv::cvtColor(src, lab_image, cv::COLOR_BGR2Lab);
	cv::split(lab_image, lab_channels);

	cv::Mat ACh = lab_channels[0];

	cv::Mat imgCenter = ACh(cv::Range(borderSize,
					  ACh.rows-borderSize+1),
				cv::Range(borderSize,
					  ACh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	ACh -= mean[0];
	ACh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(ACh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("A", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(ACh, &min, &max);
	if (max-min > 1e-4) {
		ACh = (ACh-min)/(max-min);
	} else {
		ACh.setTo(cv::Scalar(0));
	}
	cv::imshow("A", ACh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::imageBCh(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> lab_channels(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::Mat lab_image;
	cv::cvtColor(src, lab_image, cv::COLOR_BGR2Lab);
	cv::split(lab_image, lab_channels);

	cv::Mat BCh = lab_channels[0];

	cv::Mat imgCenter = BCh(cv::Range(borderSize,
					  BCh.rows-borderSize+1),
				cv::Range(borderSize,
					  BCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	BCh -= mean[0];
	BCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(BCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("B", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(BCh, &min, &max);
	if (max-min > 1e-4) {
		BCh = (BCh-min)/(max-min);
	} else {
		BCh.setTo(cv::Scalar(0));
	}
	cv::imshow("B", BCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::imageSaturCh(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> hsv_channels(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::Mat hsv_image;
	cv::cvtColor(src, hsv_image, cv::COLOR_BGR2HSV);
	cv::split(hsv_image, hsv_channels);

	cv::Mat saturCh = hsv_channels[1];

	cv::Mat imgCenter = saturCh(cv::Range(borderSize,
					      saturCh.rows-borderSize+1),
				    cv::Range(borderSize,
					      saturCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	saturCh -= mean[0];
	saturCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(saturCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("SATUR", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(saturCh, &min, &max);
	if (max-min > 1e-4) {
		saturCh = (saturCh-min)/(max-min);
	} else {
		saturCh.setTo(cv::Scalar(0));
	}
	cv::imshow("SATUR", saturCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::imageValueCh(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> hsv_channels(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::Mat hsv_image;
	cv::cvtColor(src, hsv_image, cv::COLOR_BGR2HSV);
	cv::split(hsv_image, hsv_channels);

	cv::Mat valueCh = hsv_channels[2];

	cv::Mat imgCenter = valueCh(cv::Range(borderSize,
					      valueCh.rows-borderSize+1),
				    cv::Range(borderSize,
					      valueCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	valueCh -= mean[0];
	valueCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(valueCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("VALUE", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(valueCh, &min, &max);
	if (max-min > 1e-4) {
		valueCh = (valueCh-min)/(max-min);
	} else {
		valueCh.setTo(cv::Scalar(0));
	}
	cv::imshow("VALUE", valueCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::gaussianFiltering(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> colorCh(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::split(src, colorCh);

	cv::Mat greenCh = colorCh[1];
	cv::GaussianBlur(greenCh, greenCh, cv::Size(0, 0), 1);

	cv::Mat imgCenter = greenCh(cv::Range(borderSize,
					      greenCh.rows-borderSize+1),
				    cv::Range(borderSize,
					      greenCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	greenCh -= mean[0];
	greenCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(greenCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("Gaussian", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(greenCh, &min, &max);
	if (max-min > 1e-4) {
		greenCh = (greenCh-min)/(max-min);
	} else {
		greenCh.setTo(cv::Scalar(0));
	}
	cv::imshow("Gaussian", greenCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::laplacianFiltering(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> colorCh(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::split(src, colorCh);

	cv::Mat greenCh = colorCh[1];
	cv::Laplacian(greenCh, greenCh, CV_32FC1, 9);

	cv::Mat imgCenter = greenCh(cv::Range(borderSize,
					      greenCh.rows-borderSize+1),
				    cv::Range(borderSize,
					      greenCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	greenCh -= mean[0];
	greenCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(greenCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("Laplacian", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(greenCh, &min, &max);
	if (max-min > 1e-4) {
		greenCh = (greenCh-min)/(max-min);
	} else {
		greenCh.setTo(cv::Scalar(0));
	}
	cv::imshow("Laplacian", greenCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::medianFiltering(const cv::Mat &src, EMat &dst, const void *opaque)
{
	cv::Mat gray;
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::cvtColor(src, gray, CV_BGR2GRAY);
	cv::medianBlur(gray, gray, 3);
	cv::Mat imgCenter = gray(cv::Range(borderSize, gray.rows-borderSize+1),
				 cv::Range(borderSize, gray.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	gray -= mean[0];
	gray /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(gray.rows, gray.cols);
	cv::cv2eigen(gray, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("medFilt", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(gray, &min, &max);
	if (max-min > 1e-4) {
		gray = (gray-min)/(max-min);
	} else {
		gray.setTo(cv::Scalar(0));
	}
	cv::imshow("medFilt", gray);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::sobelDrvX(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> colorCh(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::split(src, colorCh);

	cv::Mat greenCh = colorCh[1];
	cv::Sobel(greenCh, greenCh, CV_32FC1, 1, 0, 5);

	cv::Mat imgCenter = greenCh(cv::Range(borderSize,
					      greenCh.rows-borderSize+1),
				    cv::Range(borderSize,
					      greenCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	greenCh -= mean[0];
	greenCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(greenCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("sobelDrvX", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(greenCh, &min, &max);
	if (max-min > 1e-4) {
		greenCh = (greenCh-min)/(max-min);
	} else {
		greenCh.setTo(cv::Scalar(0));
	}
	cv::imshow("sobelDrvX", greenCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::sobelDrvY(const cv::Mat &src, EMat &dst, const void *opaque)
{
	std::vector< cv::Mat> colorCh(3);
	const unsigned int borderSize = *((unsigned int *)opaque);

	cv::split(src, colorCh);

	cv::Mat greenCh = colorCh[1];
	cv::Sobel(greenCh, greenCh, CV_32FC1, 0, 1, 5);

	cv::Mat imgCenter = greenCh(cv::Range(borderSize,
					      greenCh.rows-borderSize+1),
				    cv::Range(borderSize,
					      greenCh.cols-borderSize+1));
	cv::Scalar mean;
	cv::Scalar std_dev;
	cv::meanStdDev(imgCenter, mean, std_dev);
	greenCh -= mean[0];
	greenCh /= (std_dev[0]+ 10*std::numeric_limits< float >::epsilon());

	dst.resize(src.rows, src.cols);
	cv::cv2eigen(greenCh, dst);

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("ImgColor", cv::WINDOW_NORMAL);
	cv::imshow("ImgColor", src);
	cv::namedWindow("sobelDrvX", cv::WINDOW_NORMAL);
	double min, max;
	cv::minMaxLoc(greenCh, &min, &max);
	if (max-min > 1e-4) {
		greenCh = (greenCh-min)/(max-min);
	} else {
		greenCh.setTo(cv::Scalar(0));
	}
	cv::imshow("sobelDrvX", greenCh);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	return EXIT_SUCCESS;
}

int
Dataset::addMask(const unsigned int imageID,
		 cv::Mat &src)
{
	cv::copyMakeBorder(src, src,
			   borderSize, borderSize, borderSize, borderSize,
			   cv::BORDER_CONSTANT, MASK_EXCLUDED);

	cv::Mat dst = src;

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("InMask", cv::WINDOW_NORMAL);
	cv::imshow("InMask", src);
	cv::namedWindow("OutMask", cv::WINDOW_NORMAL);
	cv::imshow("OutMask", dst);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	EMat eDst(dst.rows, dst.cols);
	cv::cv2eigen(dst, eDst);
	masks[imageID] = eDst;

	return EXIT_SUCCESS;
}

int
Dataset::loadPathFile(const std::string &dataset_path, const std::string &fname,
		      std::vector< std::string > &loaded_paths)
{
	std::ifstream file(dataset_path + std::string("/") + fname);

	if (!file.is_open()) {
		log_err("Cannot open paths file %s", fname.c_str());
		return -EXIT_FAILURE;
	}

	std::string str;

	while (std::getline(file, str)) {
		if (str.length() > 0) {
			loaded_paths.push_back(str);
		}
	}

	file.close();

	return EXIT_SUCCESS;
}
