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

#ifndef FILTER_BANK_HPP_
#define FILTER_BANK_HPP_

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
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

#include "DataTypes.hpp"
#include "logging.hpp"
#include "Parameters.hpp"
#include "Dataset.hpp"
#include "JSONSerializable.hpp"

#ifdef MOVABLE_TRAIN
#include "SmoothingMatrices.hpp"
#endif // MOVABLE_TRAIN

/**
 * class FilterBank - Filter bank learned from training samples
 *
 * @filters: set of filters composing the filter bank
 */
class FilterBank : public JSONSerializable {
public:
#ifdef MOVABLE_TRAIN
    /**
     * FilterBank() - Build a filter bank by learning the filters from the
     *        given samples according to the parameters
     *
     * @params     : simulation's parameters
     * @SM         : pre-computed smoothing matrices
     * @dataset    : simulation's dataset
     * @chNo       : considered channel number
     * @samplePositions: position of the sampling points
     * @weights    : weight of each individual sampling point
     */
    FilterBank(const Parameters &params,
               const SmoothingMatrices &SM,
               const Dataset &dataset,
               const unsigned int chNo,
               const sampleSet &samplePositions,
               const EVec &weights);

    /**
     * FilterBank() - Create a new filter bank from a set of filter banks
     *        and a list of retained features
     *
     * @filterBanks: set of filter banks from which filters have to be
     *       chosen
     * @featsIdxs  : list of retained features
     *
     * WARNING: the order of the filter banks has to be consistent with the
     *      ordering of the features!
     */
    FilterBank(const std::vector< FilterBank > &filterBanks,
               const std::vector< unsigned int > &featsIdxs);

    /**
     * FilterBank() - Build a filter bank starting from its JSON description
     *        (de-serialization)
     *
     * @descr_json: string containing the JSON's description of the filter
     *      bank
     */
    FilterBank(std::string &descr_json);

#endif // MOVABLE_TRAIN

    /**
     * FilterBank() - Create a filter bank starting from a JSON node
     *
     * @root: JSON's representation root
     */
    FilterBank(Json::Value &root);

    /**
     * FilterBank() - Copy constructor
     *
     * @obj: source object of the copy
     */
    FilterBank(const FilterBank &obj);

    /**
     * operator==() - Compare two filter banks for equality
     *
     * @fb1: first filter bank in the comparison
     * @fb2: second filter bank in the comparison
     *
     * Return: true if the two filter banks are identical, false otherwise
     */
    friend bool
    operator==(const FilterBank &fb1, const FilterBank &fb2);

    /**
     * operator!=() - Compare two filter banks for difference
     *
     * @fb1: first filter bank in the comparison
     * @fb2: second filter bank in the comparison
     *
     * Return: true if the two filter banks are different, false otherwise
     */
    friend bool
    operator!=(const FilterBank &fb1, const FilterBank &fb2);

    /**
     * evaluateFilters() - Evaluate the filters over a set of samples
     *
     * @dataset    : input dataset
     * @samplePositions: vector containing the list of sample positions
     *           where filters have to be evaluated
     *
     * @features       : resulting features computed using the filters
     */

    void evaluateFilters(const Dataset &dataset,
                         const sampleSet &samplePositions,
                         EMat& features) const;

    /**
     * evaluateFiltersOnImage() - Evaluate the filters over the channels of
     *                a specific image
     *
     * @imgVec    : vector containing the channels associated with the image
     * @borderSize: size of the border added to the image before convolution
     *
     * @features: resulting features computed using the filters
     */
    void evaluateFiltersOnImage(const std::vector< cv::Mat > &imgVec,
                                const unsigned int borderSize,
                                EMat& features) const;

    /**
     * getChCount() - Get the fraction of filters for each specific channel
     *
     * @count: output filter count
     */
    void getChCount(std::vector< int > &count);

    /**
     * Serialize() - Serialize a filter bank in JSON format
     *
     * @root: root of the JSON's filter bank description
     */
    virtual void Serialize(Json::Value &root);

private:
    /**
     * struct filter - Learned filter details
     *
     * @chNo   : number of the channel on which the filter was learnt
     * @row    : start row of the filter inside the patch
     * @col    : start column of the filter inside the patch
     * @size   : filter size
     * @X      : filter shaped as a column vector
     * @Xsq    : filter shaped as a square in cv::Mat format
     */
    typedef struct filter {
        unsigned int chNo;
        unsigned int row;
        unsigned int col;
        unsigned int size;
        EMat X;
        cv::Mat Xsq;

        /**
         * operator==() - Compare two filtes for equality
         *
         * @flt1: first filter in the comparison
         * @flt2: second filter in the comparison
         *
         * Return: true if the two filters are identical, false
         *     otherwise
         */
        friend bool
        operator==(const filter &flt1, const filter &flt2)
        {
            if (flt1.chNo == flt2.chNo &&
                flt1.row == flt2.row &&
                flt1.col == flt2.col &&
                flt1.size == flt2.size &&
                flt1.X == flt2.X &&
                cvMatEquals(flt1.Xsq, flt2.Xsq)) {
                return true;
            }
            return false;
        }

        /**
         * operator!=() - Compare two filters for difference
         *
         * @flt1: first filter in the comparison
         * @flt2: second filter in the comparison
         *
         * Return: true if the two filter banks are different, false
         *     otherwise
         */
        friend bool
        operator!=(const filter &flt1, const filter &flt2)
        {
            return !(flt1 == flt2);
        }
    } filter;

    /**
     * Deserialize() - Deserialize a filter bank in JSON format
     *
     * @root: root of the JSON's filter bank description
     */
    virtual void Deserialize(Json::Value &root);

    std::vector< filter > filters;
};

#endif /* FILTER_BANK_HPP_ */
