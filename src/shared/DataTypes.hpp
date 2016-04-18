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

#ifndef DATATYPES_HPP_
#define DATATYPES_HPP_

#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <Eigen/Core>
#include <Eigen/Dense>
#pragma GCC diagnostic pop

typedef Eigen::Matrix< float, Eigen::Dynamic,
		       Eigen::Dynamic, Eigen::RowMajor > EMat;
typedef Eigen::Matrix< float, Eigen::Dynamic, 1        > EVec;
typedef Eigen::Matrix< float, 1,         Eigen::Dynamic> ERowVector;
typedef Eigen::Matrix< double, Eigen::Dynamic,
		       Eigen::Dynamic, Eigen::RowMajor > EMatD;
typedef Eigen::VectorXd EVecD;

typedef std::vector< EMat > dataVector;
typedef std::vector< dataVector > dataChannels;
typedef std::vector< EMat > maskVector;

#ifdef MOVABLE_TRAIN
typedef std::vector< EMat > gtVector;
typedef std::vector< gtVector > gtPairs;

/**
 * struct samplePos - Sampling position for a patch
 *
 * @imageNo: image number for the sample
 * @row	   : starting row of the sample
 * @col	   : starting column of the sample
 * @label  : label corresponding to the center of the sample
 */
typedef struct samplePos {
	unsigned int imageNo;
	unsigned int row;
	unsigned int col;
	int label;

	/**
	 * samplePos() - Build a sample position from its components
	 *
	 * @imageNo: image number for the sample
	 * @row	   : starting row of the sample
	 * @col	   : starting column of the sample
	 * @label  : label corresponding to the center of the sample
	 */
	samplePos(const unsigned int imgNo,
		  const unsigned int r,
		  const unsigned int c,
		  const int lbl) :
		imageNo(imgNo),	row(r), col(c), label(lbl) { };

	/**
	 * samplePos() - Create an uninitialized sample position element
	 */
	samplePos() { };

	/**
	 * samplePos() - Sample position's copy constructor
	 *
	 * @other: source instance
	 */
	samplePos(const samplePos& other) :
		imageNo(other.imageNo),
		row(other.row), col(other.col),
		label(other.label) { };

	/**
	 * operator==() - Compare two sample positions for equality
	 *
	 * @s1: first sample position in the comparison
	 * @s2: second sample position in the comparison
	 *
	 * Return: true if the two sample positions are identical, false
	 *         otherwise
	 */
	friend bool
	operator==(const samplePos &s1, const samplePos &s2)
	{
		return (s1.imageNo == s2.imageNo &&
			s1.row == s2.row &&
			s1.col == s2.col &&
			s1.label == s2.label);
	}

	/**
	 * operator!=() - Compare two sample positions for difference
	 *
	 * @s1: first sample position in the comparison
	 * @s2: second sample position in the comparison
	 *
	 * Return: true if the two sample positions are different, false
	 *         otherwise
	 */
	friend bool
	operator!=(const samplePos &s1, const samplePos &s2)
	{
		return !(s1 == s2);
	}
} samplePos;

typedef std::vector< samplePos > sampleSet;

#endif /* MOVABLE_TRAIN */

#endif /* DATATYPES_HPP_ */
