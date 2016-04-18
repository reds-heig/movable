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

#ifndef WEAK_LEARNER_HPP_
#define WEAK_LEARNER_HPP_

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

#include <chrono>
#include <ctime>

#include "DataTypes.hpp"
#include "logging.hpp"
#include "Parameters.hpp"
#include "Dataset.hpp"
#include "FilterBank.hpp"
#include "RegTree.hpp"
#include "JSONSerializable.hpp"

#ifdef MOVABLE_TRAIN
#include "LineSearch.hpp"
#include "SmoothingMatrices.hpp"
#endif /* MOVABLE_TRAIN */

/**
 * class WeakLearner - Weak learner for the boosting classifier
 *
 * @fb   : filter bank
 * @rt   : regression tree
 * @alpha: weak learner's weight
 * @MR   : weak learner's misclassification rate
 * @loss : weak learner's loss
 */
class WeakLearner : public JSONSerializable {
public:
#ifdef MOVABLE_TRAIN
	/**
	 * WeakLearner() - Create a weak learner starting from parameters and
	 *                 data
	 *
	 * @params         : simulation's parameters
	 * @SM             : pre-computed smoothing matrices
	 * @dataset        : simulation's dataset
	 * @samplePositions: position of the sampling points
	 * @weights        : weight of each individual sampling point (used to
	 *                   update the current's weights too, therefore it is a
	 *                   in/out variable)
	 * @labels         : label of each individual sampling point
	 * @currentResponse: current response of the classifier (used to update
	 *                   this response too, therefore it is a in/out
	 *                   variable)
	 */
	WeakLearner(const Parameters &params,
		    const SmoothingMatrices &SM,
		    const Dataset &dataset,
		    const sampleSet &samplePositions,
		    EVec &weights,
		    const EVec &labels,
		    EVec &currentResponse);
#endif /* MOVABLE_TRAIN */

	/**
	 * WeakLearner() - Build a weak learner starting from its JSON
	 *                 description(de-serialization)
	 *
	 * @descr_json: string containing the JSON's description of the weak
	 *              learner
	 */
	WeakLearner(std::string &descr_json);

	/**
	 * WeakLearner() - Create a weak learner starting from a JSON node
	 *
	 * @root: JSON's representation root
	 */
	WeakLearner(Json::Value &root)
	{
		Deserialize(root);
	}

	/**
	 * WeakLearner() - Copy constructor
	 *
	 * @obj: source object of the copy
	 */
	WeakLearner(const WeakLearner &obj)
	{
		/* Delete previous filter bank and regression tree, then
		   instantiate the new ones from the copied object */
		delete fb;
		delete rt;
		this->fb = new FilterBank(*(obj.fb));
		this->rt = new RegTree(*(obj.rt));
		this->alpha = obj.alpha;
		this->MR = obj.MR;
		this->loss = obj.loss;
	}

	/**
	 * ~WeakLearner() - Deallocate the filter bank and the regression tree
	 */
	virtual ~WeakLearner()
	{
		delete fb;
		delete rt;
	}

	/**
	 * operator=() - Assignment operator
	 *
	 * @rhs: source of the assignment
	 *
	 * Return: Reference to the resulting weak learner
	 */
	WeakLearner &
	operator=(const WeakLearner &rhs)
	{
		if (this != &rhs) {
			delete fb;
			delete rt;

			fb = new FilterBank(*(rhs.fb));
			rt = new RegTree(*(rhs.rt));
			alpha = rhs.alpha;
			MR = rhs.MR;
			loss = rhs.loss;
		}

		return *this;
	}

	/**
	 * operator==() - Compare two weak learners for equality
	 *
	 * @wl1: first weak learner in the comparison
	 * @wl2: second weak learner in the comparison
	 *
	 * Return: true if the two weak learners are identical, false otherwise
	 */
	friend bool
	operator==(const WeakLearner &wl1, const WeakLearner &wl2)
	{
		return ((fabs(wl1.MR-wl2.MR) < 1e-6) &&
			(fabs(wl1.loss-wl2.loss) < 1e-6) &&
			(fabs(wl1.alpha-wl2.alpha) < 1e-9) &&
			(*(wl1.fb) == *(wl2.fb)) &&
			(*(wl1.rt) == *(wl2.rt)));
	}

	/**
	 * operator!=() - Compare two weak learners for difference
	 *
	 * @wl1: first weak learner in the comparison
	 * @wl2: second weak learner in the comparison
	 *
	 * Return: true if the two weak learners are different, false otherwise
	 */
	friend bool
	operator!=(const WeakLearner &wl1, const WeakLearner &wl2)
	{
		return !(wl1 == wl2);
	}

#ifdef MOVABLE_TRAIN

	/**
	 * evaluate() - Evaluate a weak learner on a given set of samples
	 *
	 * @dataset     : source dataset for the samples
	 * @samplePoints: sampling points
	 *
	 * @predictions : resulting (weighted) predictions for the current weak
	 *                learner on the considered data
	 */
	void evaluate(const Dataset &dataset,
		      const sampleSet &samplePositions,
		      EVec &predictions) const
	{
		EMat features;
		fb->evaluateFilters(dataset, samplePositions, features);
		rt->predict(features, predictions);
		predictions *= alpha;
	}

	/**
	 * getChCount() - Get the fraction of filters for each specific channel
	 *
	 * @count: output filter count
	 */
	void getChCount(std::vector< int > &count)
	{
		fb->getChCount(count);
	}

#endif /* MOVABLE_TRAIN */

	/**
	 * evaluateOnImage() - Evaluate a weak learner on a given image
	 *
	 * @imgVec    : vector containing the channels associated with the image
	 * @borderSize: size of the border that has to be excluded from the
	 *              result
	 *
	 * @wlResponse: response produced by the weak learner, already weighted
	 *              by the weak learner's importance
	 */
	void evaluateOnImage(const std::vector< cv::Mat > &imgVec,
			     const unsigned int borderSize,
			     EMat &wlResponse) const
	{
		EMat features;
		fb->evaluateFiltersOnImage(imgVec, borderSize, features);
		EVec treeResponse;
		rt->predict(features, treeResponse);
		wlResponse = Eigen::Map< EMat >(treeResponse.data(),
						imgVec[0].rows-2*borderSize,
						imgVec[0].cols-2*borderSize);
		wlResponse *= alpha;
	}

	/**
	 * getLoss() - Return the weak learner's loss on train data
	 *
	 * Return: Weak learner's loss on the samples used for training it
	 */
	float getLoss() const { return loss; };

	/**
	 * getMR() - Return the weak learner's misclassification rate on train
	 *           data
	 *
	 * Return: Weak learner's misclassification rate on the samples used for
	 *         training it
	 */
	float getMR() const { return MR; };

	/**
	 * Serialize() - Serialize a weak learner in JSON format
	 *
	 * @root: root of the JSON's weak learner description
	 */
	virtual void Serialize(Json::Value &root);

private:
	FilterBank *fb;
	RegTree *rt;
	double alpha;

	float MR;
	float loss;

	/**
	 * Deserialize() - Deserialize a weak learner in JSON format
	 *
	 * @root: root of the JSON's weak learner description
	 */
	virtual void Deserialize(Json::Value &root);

};

#endif /* WEAK_LEARNER_HPP_ */
