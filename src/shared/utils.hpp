/*******************************************************************************
 ** MOVABLE project - REDS Institute, HEIG-VD, Yverdon-les-Bains (CH) - 2016  **
 **                                                                           **
 ** This file is part of MOVABLE.                                             **
 **                                                                           **
 **  MOVABLE is free software: you can redistribute it and/or modify          **
 **  it under the terms of the GNU General Public License as published by     **
 **  the Free Software Foundation, either version 3 of the License, or        **
 **  (at your option) any later version.                                      **
 **                                                                           **
 **  MOVABLE is distributed in the hope that it will be useful,               **
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of           **
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            **
 **  GNU General Public License for more details.                             **
 **                                                                           **
 **  You should have received a copy of the GNU General Public License        **
 **  along with MOVABLE.  If not, see <http://www.gnu.org/licenses/>.         **
 ******************************************************************************/

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <random>
#include <iomanip>
#include <vector>
#include <fstream>
#include <numeric>

#include <cassert>
#include <ctime>

#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wcast-qual"
#include <Eigen/Core>
#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include <opencv2/core/eigen.hpp>
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

#include "Parameters.hpp"
#include "DataTypes.hpp"

/**
 * checkChannelPresent() - Check if a given channel is requested by the
 *                         user (that is, if the corresponding string
 *                         has been specified in the list of channels)
 *
 * @sought: sought channel
 * @chList: list of requested channels
 *
 * Return: true if the channel was used, false otherwise
 */
bool checkChannelPresent(const std::string &sought,
                         const std::vector< std::string > &chList);

/**
 * computeF1Score() - Compute the F1 score for an image-threshold pair,
 *                    where the thresholded image undergoes morphological
 *                    transformations before the score computation
 *
 * @warning: the function does NOT compute exactly the F1 score, it computes
 *           instead a value that is proportional to it
 *
 * @scoreImage: input score image in [-1, 1]
 * @gt        : corresponding ground-truth in {NEG_GT_CLASS, POS_GT_CLASS}
 * @threshold : desired threshold in [-1, 1]
 *
 * Result: computed F1 score
 */
float computeF1Score(const cv::Mat &scoreImage,
                     const cv::Mat &gt,
                     float threshold);

/**
 * computeMR() - Given an image and the corresponding ground-truth, compute the
 *               misclassification rate
 *
 * @img       : image to evaluate
 * @gt        : corresponding ground-truth
 * @borderSize: size of the border to ignore
 *
 * Return: misclassification rate in [0, 1]
 */
float computeMR(const EMat &img, const EMat &gt, const unsigned int borderSize);

/**
 * cvMatEquals() - Compare two OpenCV matrices for equality
 *
 * @m1: first matrix in the comparison
 * @m2: second matrix in the comparison
 *
 * Return: true if the two matrices have the same content, false otherwise
 */
bool cvMatEquals(const cv::Mat m1, const cv::Mat m2);

/**
 * normalizeImage() - Normalize an image in [-1, 1] after having converted it in
 *                    OpenCV format
 *
 * @resultImage: image obtained from the classification algorithm
 * @bordersize : size of the border to drop
 * @scoreImage : output image, normalized in [-1, 1]
 */
void normalizeImage(const EMat &resultImage,
                    const int borderSize,
                    cv::Mat &scoreImage);

/**
 * randomSamplingWithoutReplacement() - Sample a given number (M) of indexes
 *                                      from a pool (ranging from 0 to N-1)
 *                                      without remplacement
 *
 * Modified version of the RandomSamplingWithoutReplacement() function written
 * by Gael Beaunée - http://gaelbn.com/random-sampling-without-replacement/
 *
 * @M: number of indexes to sample
 * @N: pool size (indexes will run from 0 to N-1)
 *
 * Return: vector with M randomly sampled indexes in [0, N-1]
 */
std::vector< unsigned int >
randomSamplingWithoutReplacement(const unsigned int M,
                                 const unsigned int N);

/**
 * randomWeightedSamplingWithReplacement() - Sample a vector of number of the
 *                                           desired size according to the given
 *                                           distribution
 *
 * @M: number of values to sample
 * @W: weights describing the distribution
 */
template <typename T>
std::vector< unsigned int >
randomWeightedSamplingWithReplacement(const unsigned int M,
                                      const std::vector< T > &W)
{
    assert (W.size() > 0);

    std::mt19937 gen(std::time(0));
    std::vector< unsigned int > sampleList(W.size());
    std::iota(sampleList.begin(), sampleList.end(), 0);

    std::piecewise_constant_distribution< > dist(std::begin(sampleList),
                                                 std::end(sampleList),
                                                 std::begin(W));
    std::vector< unsigned int > randomSamples(M);
    for (unsigned int i = 0; i < M; ++i) {
        randomSamples[i] = static_cast< unsigned int >(dist(gen));
    }

    assert (randomSamples.size() == M);

    return randomSamples;
}

/**
 * removeSmallBlobs() - Equivalent of Matlab's bwareaopen(), taken from
 *                      http://opencv-code.com/quick-tips/code-replacement-for-matlabs-bwareaopen/
 *
 * @img : image where small blobs have to be filtered out, 8 bits/1ch and with
 *        values {0, 255} only
 * @size: size of the smallest blob to keep
 */
void removeSmallBlobs(cv::Mat& img, float size);

/**
 * saveClassifiedImage() - Save the result of a classification as an image on
 *                         disk
 *
 * @classResult: classification result to store
 * @dirPath    : path of the destination directory
 * @imgName    : name of the destination image (it is the same as the original
 *               image name)
 * @borderSize : size of the border to drop
 */
void saveClassifiedImage(const EMat &classResult,
                         const std::string &dirPath,
                         const std::string &imgName,
                         const unsigned int borderSize);

/**
 * saveThresholdedImage() - Save the thresholded result to disk
 *
 * @classResult : classification result to store
 * @mask        : mask to apply
 * @threshold   : threshold to apply
 * @dirPath     : path of the destination directory
 * @imgName     : name of the destination image (it is the same as the original
 *                image name)
 * @originalSize: original image dimensions
 * @borderSize  : size of the border to drop
 */
void
saveThresholdedImage(const cv::Mat &classResult,
                     const EMat &mask,
                     const float threshold,
                     const std::string &dirPath,
                     const std::string &imgName,
                     const std::pair< int, int >& originalSize,
                     const unsigned int borderSize);

/**
 * saveOverlayedImage() - Save the thresholded result, overlayed to the input
 *                        image, to disk
 *
 * @imageFName      : input image path
 * @dirPath         : path of the destination directory
 * @imgName         : name of the destination image (it is the same as the
 *                    original image name)
 */
void
saveOverlayedImage(const std::string &imageFName,
                   const std::string &dirPath,
                   const std::string &imgName);

/**
 * splitSampleSet() - Split a set of samples in two distinct groups of the
 *                    desired size, one for learning of the filters and the
 *                    other for learning the tree
 *
 * @samples        : original sample set
 * @Y              : corresponding set of labels
 * @W              : corresponding weights
 * @subsetSamplesNo: cardinality of the desired subsets
 *
 * @samples_fl     : extracted subset for filter learning
 * @samples_tree   : extracted subset for tree learning
 * @Y_fl           : labels of the filter learning subset
 * @Y_tree         : labels of the tree learning subset
 * @W_fl           : weights of the filter learning subset
 * @W_tree         : weights of the tree learning subset
 *
 * Return: -EXIT_FAILURE in case of error, EXIT_SUCCESS otherwise
 */
int splitSampleSet(const sampleSet &samples,
                   const EVec &Y,
                   const EVec &W,
                   const unsigned int subsetSamplesNo,
                   sampleSet &samples_fl,
                   sampleSet &samples_tree,
                   EVec &Y_fl,
                   EVec &Y_tree,
                   EVec &W_fl,
                   EVec &W_tree);

#ifdef MOVABLE_TRAIN
/**
 * createDirectories() - Create the set of directories needed by the simulation
 *
 * @params : simulation's parameters
 * @dataset: input dataset
 *
 * Return: -EXIT_FAILURE if an error occurred, EXIT_SUCCESS otherwise
 */
class Dataset;
int createDirectories(Parameters &params, const Dataset &dataset);
#else // !MOVABLE_TRAIN
/**
 * createDirectories() - Create the set of directories needed by the simulation
 *
 * @params : simulation's parameters
 *
 * Return: -EXIT_FAILURE if an error occurred, EXIT_SUCCESS otherwise
 */
int createDirectories(Parameters &params);

#endif // MOVABLE_TRAIN

#endif /* UTILS_HPP_ */
