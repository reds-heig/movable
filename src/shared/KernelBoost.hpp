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

#ifndef KERNEL_BOOST_HPP_
#define KERNEL_BOOST_HPP_

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wcast-qual"
#include <Eigen/Core>
#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/ml/ml.hpp>
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

#include "DataTypes.hpp"
#include "logging.hpp"
#include "Parameters.hpp"
#include "Dataset.hpp"
#include "JSONSerializable.hpp"
#include "BoostedClassifier.hpp"

#ifdef MOVABLE_TRAIN
#include "SmoothingMatrices.hpp"
#endif // MOVABLE_TRAIN

/**
 * class KernelBoost - KernelBoost main class, grouping the boosted classifiers
 *                     for each individual gt pair and the final classifier
 *
 * @boostedClassifiers: boosted classifiers for the individual gt pairs
 * @finalClassifier   : boosted classifier performing the final classification
 *                      based on the outcomes of the previous ones
 * @binaryThreshold   : threshold used to obtain the final, binary images
 *                      starting from the classified ones
 */
class KernelBoost : public JSONSerializable {
public:
#ifdef MOVABLE_TRAIN
    /**
     * KernelBoost() - Create a KernelBoost classifier
     *
     * @params : simulation's parameters
     * @SM     : pre-computed smoothing matrices
     * @dataset: simulation's dataset
     */
    KernelBoost(Parameters &params,
                const SmoothingMatrices &SM,
                const Dataset &dataset);

    /**
     * KernelBoost() - Build a KernelBoost classifier starting from its JSON
     *                 description(de-serialization)
     *
     * @descr_json: string containing the JSON's description of the
     *              KernelBoost classifier
     */
    KernelBoost(std::string &descr_json);
#else // !MOVABLE_TRAIN
    /**
     * KernelBoost() - Build a KernelBoost classifier starting from its JSON
     *                 description(de-serialization)
     *
     * @descr_json: string containing the JSON's description of the
     *              KernelBoost classifier
     * @params    : simulation's parameters
     * @dataset   : simulation's dataset
     */
    KernelBoost(std::string &descr_json,
                const Parameters &params,
                Dataset &dataset);

#endif // MOVABLE_TRAIN

    /**
     * KernelBoost() - Create a KernelBoost classifier starting from a JSON
     *                 node
     *
     * @root: JSON's representation root
     */
    KernelBoost(Json::Value &root);

    /**
     * ~KernelBoost() - Deallocate the boosted classifiers
     */
    virtual ~KernelBoost();

    /**
     * KernelBoost() - Copy constructor
     *
     * @obj: source object of the copy
     */
    KernelBoost(const KernelBoost &obj);

    /**
     * operator=() - Assignment operator
     *
     * @rhs: source of the assignment
     *
     * Return: Reference to the resulting KernelBoost classifier
     */
    KernelBoost &
    operator=(const KernelBoost &rhs);

    /**
     * operator==() - Compare two KernelBoost classifiers for equality
     *
     * @kb1: first KernelBoost classifier in the comparison
     * @kb2: second KernelBoost classifier in the comparison
     *
     * Return: true if the two KernelBoost classifiers are identical, false
     *         otherwise
     */
    friend bool
    operator==(const KernelBoost &kb1, const KernelBoost &kb2);

    /**
     * operator!=() - Compare two KernelBoost classifiers for difference
     *
     * @kb1: first KernelBoost classifier in the comparison
     * @kb2: second KernelBoost classifier in the comparison
     *
     * Return: true if the two KernelBoost classifiers are different, false
     *         otherwise
     */
    friend bool
    operator!=(const KernelBoost &kb1, const KernelBoost &kb2);

    /**
     * Serialize() - Serialize a KernelBoost classifier in JSON format
     *
     * @root  : root of the JSON's KernelBoost classifier description
     *
     * @warning: superseded by the function taking param as an argument, as
     *           it allows to set some values for the test routine
     */
    virtual void Serialize(Json::Value &);

    /**
     * Serialize() - Serialize a KernelBoost classifier in JSON format
     *
     * @root  : root of the JSON's KernelBoost classifier description
     * @params: simulation's parameters
     */
    void serialize(Json::Value &root, const Parameters &params);

private:
    std::vector< BoostedClassifier * > boostedClassifiers;
    BoostedClassifier *finalClassifier;
    float binaryThreshold;

    /**
     * Deserialize() - Deserialize a KernelBoost classifier in JSON format
     *
     * @root: root of the JSON's KernelBoost classifier description
     */
    virtual void Deserialize(Json::Value &root);
};

#endif /* KERNEL_BOOST_HPP_ */
