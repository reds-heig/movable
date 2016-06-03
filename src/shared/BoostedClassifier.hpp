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

#ifndef BOOSTED_CLASSIFIER_HPP_
#define BOOSTED_CLASSIFIER_HPP_

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

#include "DataTypes.hpp"
#include "logging.hpp"
#include "Parameters.hpp"
#include "Dataset.hpp"
#include "JSONSerializable.hpp"
#include "WeakLearner.hpp"

/**
 * class BoostedClassifier - Boosted Classifier main class, grouping all weak
 *			     learners
 *
 * @weakLearners: weak learners
 * @gtPair	: ground truth pair considered by the classifier
 */
class BoostedClassifier : public JSONSerializable {
public:
#ifdef MOVABLE_TRAIN
	/**
	 * BoostedClassifier() - Create a boosted classifier
	 *
	 * @params : simulation's parameters
	 * @SM	   : pre-computed smoothing matrices
	 * @dataset: simulation's dataset
	 * @gtPair : considered ground truth pair
	 */
	BoostedClassifier(const Parameters &params,
			  const SmoothingMatrices &SM,
			  const Dataset &dataset,
			  const unsigned int gtPair);
#endif /* MOVABLE_TRAIN */

	/**
	 * BoostedClassifier() - Build a boosted classifier starting from its
	 *			 JSON description(de-serialization)
	 *
	 * @descr_json: string containing the JSON's description of the boosted
	 *		classifier
	 */
	BoostedClassifier(std::string &descr_json);

	/**
	 * BoostedClassifier() - Create a boosted classifier starting from a
	 *			 JSON node
	 *
	 * @root: JSON's representation root
	 */
	BoostedClassifier(Json::Value &root)
	{
		Deserialize(root);
	}

	/**
	* ~BoostedClassifier() - Deallocate the weak learners
	*/
	virtual ~BoostedClassifier()
	{
		for (unsigned int i = 0; i < weakLearners.size(); ++i) {
			delete weakLearners[i];
		}
	}

	/**
	 * BoostedClassifier() - Copy constructor
	 *
	 * @obj: source object of the copy
	 */
	BoostedClassifier(const BoostedClassifier &obj)
	{
		/* Deallocate previous weak learners */
		for (unsigned int i = 0; i < weakLearners.size(); ++i) {
			delete weakLearners[i];
		}
		/* Resize the weak learners vector */
		this->weakLearners.resize(obj.weakLearners.size());
		/* Create the new set of weak learners from the elements of the
		   copied one */
		for (unsigned int i = 0; i < weakLearners.size(); ++i) {
			weakLearners[i] = new WeakLearner(*(obj.weakLearners[i]));
		}
		gtPair = obj.gtPair;
	}

	/**
	 * operator=() - Assignment operator
	 *
	 * @rhs: source of the assignment
	 *
	 * Return: Reference to the resulting boosted classifier
	 */
	BoostedClassifier &
	operator=(const BoostedClassifier &rhs)
	{
		if (this != &rhs) {
			/* Deallocate previous weak learners */
			for (unsigned int i = 0; i < weakLearners.size(); ++i) {
				delete weakLearners[i];
			}

			/* Resize the weak learners vector */
			this->weakLearners.resize(rhs.weakLearners.size());
			/* Create the new set of weak learners from the elements
			   of the copied one */
			for (unsigned int i = 0; i < weakLearners.size(); ++i) {
				weakLearners[i] =
					new WeakLearner(*(rhs.weakLearners[i]));
			}
			gtPair = rhs.gtPair;
		}

		return *this;
	}

	/**
	 * operator==() - Compare two boosted classifiers for equality
	 *
	 * @wl1: first boosted classifier in the comparison
	 * @wl2: second boosted classifier in the comparison
	 *
	 * Return: true if the two boosted classifiers are identical, false
	 *	   otherwise
	 */
	friend bool
	operator==(const BoostedClassifier &bc1, const BoostedClassifier &bc2)
	{
		if (bc1.weakLearners.size() != bc2.weakLearners.size() ||
		    bc1.gtPair != bc2.gtPair)
			return false;

		for (unsigned int i = 0; i < bc1.weakLearners.size(); ++i) {
			if (*(bc1.weakLearners[i]) != *(bc2.weakLearners[i]))
				return false;
		}

		return true;
	}

	/**
	 * operator!=() - Compare two boosted classifiers for difference
	 *
	 * @wl1: first boosted classifier in the comparison
	 * @wl2: second boosted classifier in the comparison
	 *
	 * Return: true if the two boosted classifiers are different, false
	 *	   otherwise
	 */
	friend bool
	operator!=(const BoostedClassifier &bc1, const BoostedClassifier &bc2)
	{
		return !(bc1 == bc2);
	}

#ifdef MOVABLE_TRAIN
	/**
	 * classify() - Classify a set of samples using the learned classifier
	 *
	 * @dataset    : simulation's dataset
	 * @samplingPos: considered sampling positions
	 *
	 * @predictions: computed predictions
	 */
	void classify(const Dataset &dataset,
		      const sampleSet &samplePositions,
		      EVec &predictions) const
	{
		predictions.resize(samplePositions.size());
		predictions.setZero();

		for (unsigned int w = 0; w < weakLearners.size(); ++w) {
			EVec currentResponse;
			weakLearners[w]->evaluate(dataset,
						  samplePositions,
						  currentResponse);
			predictions += currentResponse;
		}
	}

	/**
	 * getChCount() - Get the fraction of filters for each specific channel
	 *
	 * @count: output filter count
	 */
	void getChCount(std::vector< int > &count)
	{
		for (unsigned int w = 0; w < weakLearners.size(); ++w) {
			weakLearners[w]->getChCount(count);
		}
	}

#endif /* MOVABLE_TRAIN */

	/**
	 * classifyImage() - Classify an image using the learned classifier
	 *
	 * @imgVec    : vector containing the channels associated with the image
	 * @borderSize: size of the border that has to be excluded from the
	 *		result
	 *
	 * @prediction: computed result image
	 */
	void classifyImage(const std::vector< cv::Mat > &imgVec,
			   const unsigned int borderSize,
			   EMat &prediction) const
	{
		prediction.resize(imgVec[0].rows-2*borderSize,
				  imgVec[0].cols-2*borderSize);
		prediction.setZero();

		for (unsigned int w = 0; w < weakLearners.size(); ++w) {
			EMat currentResponse;
			weakLearners[w]->evaluateOnImage(imgVec,
							 borderSize,
							 currentResponse);
			prediction += currentResponse;
		}
	}

	/**
	 * Serialize() - Serialize a boosted classifier in JSON format
	 *
	 * @root: root of the JSON's boosted classifier description
	 */
	virtual void Serialize(Json::Value &root);

private:
	std::vector< WeakLearner * > weakLearners;
	unsigned int gtPair;

	/**
	 * Deserialize() - Deserialize a boosted classifier in JSON format
	 *
	 * @root: root of the JSON's boosted classifier description
	 */
	virtual void Deserialize(Json::Value &root);
};

#endif /* BOOSTED_CLASSIFIER_HPP_ */
