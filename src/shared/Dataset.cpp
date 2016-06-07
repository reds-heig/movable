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
	/* Load images */
	for (unsigned int i = 0; i < img_paths.size(); ++i) {
		log_info("\t\tAdding image %d/%d...",
			 (int)i+1, (int)img_paths.size());
		if (addImage(img_paths[i], mask_paths[i],
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
	/* Load images */
	for (unsigned int i = 0; i < img_paths.size(); ++i) {
		log_info("\t\tAdding image %d/%d...",
			 (int)i+1, (int)img_paths.size());
		if (addImage(img_paths[i], mask_paths[i]) != EXIT_SUCCESS) {
			log_err("Error encountered while loading image %d/%d",
				(int)i+1, (int)img_paths.size());
			throw std::runtime_error("imageLoading");
		}
	}
#endif /* MOVABLE_TRAIN */

}

#ifndef TESTS

#ifdef MOVABLE_TRAIN
Dataset::Dataset(const Parameters &params,
		 const Dataset &srcDataset,
		 const std::vector< BoostedClassifier * > &boostedClassifiers)
#else
Dataset::Dataset(const Dataset &srcDataset,
		 const std::vector< BoostedClassifier * > &boostedClassifiers)
#endif /* MOVABLE_TRAIN */

#else

Dataset::Dataset(const Dataset &srcDataset,
		 const std::vector< BoostedClassifier * > &boostedClassifiers)

#endif /* TESTS */
{
	/* Copy values from the source dataset */
	this->data = srcDataset.data;
	this->dataChNo = srcDataset.dataChNo;
	this->imageOps = srcDataset.imageOps;
	this->imagesNo = srcDataset.imagesNo;
	this->imageNames = srcDataset.imageNames;
	this->sampleSize = srcDataset.sampleSize;
	this->borderSize = srcDataset.borderSize;
	this->masks = srcDataset.masks;
#ifdef MOVABLE_TRAIN
	this->gts = srcDataset.gts;
	this->originalGts = srcDataset.originalGts;
	this->gtValues = srcDataset.gtValues;
	this->gtPairValues = srcDataset.gtPairValues;
	this->feedbackImagesFlag = srcDataset.feedbackImagesFlag;
#endif /* MOVABLE_TRAIN */

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

		/* Prepare a vector containing the set of OpenCV matrices
		   corresponding to the available channels */
		std::vector< cv::Mat > chs;
		getChsForImage(i, chs);
		for (unsigned int bc = 0; bc < boostedClassifiers.size(); ++bc) {
			boostedClassifiers[bc]->classifyImage(chs,
							      borderSize,
							      data[dataChNo+bc][i]);

#ifndef TESTS
#ifdef MOVABLE_TRAIN
			saveClassifiedImage(data[dataChNo+bc][i],
					    params.intermedResDir[bc],
					    imageNames[i],
					    borderSize);
#endif /* MOVABLE_TRAIN */
#endif /* TESTS */

		}

#ifndef TESTS
		end = std::chrono::system_clock::now();
		std::chrono::duration< double > elapsed_s = end-start;
		log_info("\t\tImage %d/%d DONE! (took %.3fs)",
			 i+1, imagesNo, elapsed_s.count());
#endif /* TESTS */
	}
	dataChNo += boostedClassifiers.size();

#ifdef MOVABLE_TRAIN
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
#endif /* MOVABLE_TRAIN */
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

#ifdef MOVABLE_TRAIN

int
Dataset::addGt(const cv::Mat &src)
{
	/* Enlarge the original GT image and store it */
	cv::Mat oSrc;
	cv::copyMakeBorder(src, oSrc,
			   borderSize, borderSize, borderSize, borderSize,
			   cv::BORDER_CONSTANT, IGN_GT_CLASS);
	EMat oTmp(oSrc.rows, oSrc.cols);
	cv::cv2eigen(oSrc, oTmp);
	originalGts.push_back(oTmp);

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
		gts[i].push_back(eTmp);

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
Dataset::addImage(const std::string &imgPath,
		  const std::string &maskPath,
		  const std::string &gtPath)
#else
	int
	Dataset::addImage(const std::string &imgPath,
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

	cv::Mat tmp;
	mask.convertTo(tmp, CV_32FC1);
	addMask(tmp);

	/* Always read a color image, it will be the loop over the operations
	   that will grab the different component and eventually keep the
	   grayscale one only */
	cv::Mat img = cv::imread(imgPath.c_str(), CV_LOAD_IMAGE_COLOR);
	img.convertTo(tmp, CV_32FC3);
	img = tmp / 255;

	/* Replicate image borders */
	cv::copyMakeBorder(img, img,
			   borderSize, borderSize, borderSize, borderSize,
			   cv::BORDER_REPLICATE);
	/* Now alter the image according to user specifications, creating the
	   corresponding channels */
	for (unsigned int i = 0; i < imageOps.size(); ++i) {
		EMat tmp;
		imageOps[i](img, tmp, (void *)&borderSize);
		data[i].push_back(tmp);
	}

	/* Check that all sizes are consistent */
	const unsigned int rowsNo = masks[imagesNo].rows();
	const unsigned int colsNo = masks[imagesNo].cols();
	for (unsigned int i = 0; i < imageOps.size(); ++i) {
		if (data[i][imagesNo].rows() != rowsNo ||
		    data[i][imagesNo].cols() != colsNo) {
			log_err("Invalid image size -- image %d, channel %d",
				imagesNo+1, i);
			return -EXIT_FAILURE;
		}
	}

#ifdef MOVABLE_TRAIN
	cv::Mat gt = cv::imread(gtPath.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
	gt.convertTo(tmp, CV_32FC1);
	addGt(tmp);
	for (unsigned int i = 0; i < gtPairsNo; ++i) {
		if (gts[i][imagesNo].rows() != rowsNo ||
		    gts[i][imagesNo].cols() != colsNo) {
			log_err("Invalid GT size -- image %d, gt pair %d",
				imagesNo+1, i);
			return -EXIT_FAILURE;
		}
	}
#endif /* MOVABLE_TRAIN */

	imagesNo++;

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
			for (unsigned int c = 0; c < (unsigned int)gtImg.cols(); ++c) {
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
#else
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
Dataset::addMask(cv::Mat &src)
{
	cv::copyMakeBorder(src, src,
			   borderSize, borderSize, borderSize, borderSize,
			   cv::BORDER_CONSTANT, MASK_EXCLUDED);

#ifdef MOVABLE_TRAIN
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,
						    cv::Size(2*sampleSize + 1,
							     2*sampleSize + 1),
						    cv::Point(sampleSize,
							      sampleSize));
	cv::Mat dst;
	cv::erode(src, dst, element);
#else
	cv::Mat dst = src;
#endif /* MOVABLE_TRAIN */

#ifdef VISUALIZE_IMG_DATA
	cv::namedWindow("InMask", cv::WINDOW_NORMAL);
	cv::imshow("InMask", src);
	cv::namedWindow("OutMask", cv::WINDOW_NORMAL);
	cv::imshow("OutMask", dst);
	cv::waitKey(0);
#endif /* VISUALIZE_IMG_DATA */

	EMat eDst(dst.rows, dst.cols);
	cv::cv2eigen(dst, eDst);
	masks.push_back(eDst);

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
