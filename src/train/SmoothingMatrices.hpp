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

#ifndef SMOOTHING_MATRICES_HPP_
#define SMOOTHING_MATRICES_HPP_

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

#include "DataTypes.hpp"
#include "logging.hpp"

/**
 * class SmoothingMatrices - Smoothing matrices used in filter learning
 *
 * @M		   : set of smoothing matrices
 * @sizes	   : base set of filters sizes used for building the matrices
 * @smoothingValues: base set of lambda values used for building the matrices
 */
class SmoothingMatrices {
public:
	/**
	 * SmoothingMatrices() - Construct a matrix of smoothing matrices for
	 *			 the desired size range (only ODD values are
	 *			 considered!) and smoothing values
	 *
	 * @minSize	   : minimum considered size
	 * @maxSize	   : maximum considered size
	 * @smoothingValues: vector containing the desired smoothing values
	 */
	SmoothingMatrices(const unsigned int minSize,
			  const unsigned int maxSize,
			  const std::vector< float > &smoothingValues);

	/**
	 * getSmoothingMatrix() - Get a reference to the smoothing matrix for a
	 *			  specific size-lambda pair
	 *
	 * @filterSize: considered filter size
	 * @lambda    : smoothing value
	 *
	 * Return: Reference to the desired smoothing matrix
	 */
	const EMat& getSmoothingMatrix(const unsigned int filterSize,
				       const float lambda) const
	{
		const int XPos_size = getPosSize(filterSize);
		const int XPos_lambda = getPosLambda(lambda);

		if (XPos_size < 0 || XPos_lambda < 0) {
			log_err("The requested smoothing matrix does not exist "
				"(filter size = %d, lambda = %f)",
				filterSize, lambda);
			/* Return an empty EMat */
			static EMat nullresult;
			return nullresult;
		}
		return M[XPos_size][XPos_lambda];
	}
private:
	/**
	 * struct PixLoc - Single pixel location in the image
	 *
	 * @r  : row number of the pixel position
	 * @c  : column number of the pixel position
	 * @max: maximum valid value for r and c
	 */
	struct PixLoc {
		int r;
		int c;
		int max;

		/* Store the row and column numbers identifying this pixel, as
		   well as their maximum possible value (that is, the subpatch
		   size) */
		PixLoc(const int r, const int c, const int max) :
			r(r), c(c), max(max) {
			assert(max > 0);
		}

		/* Return true if the pixel is valid (that is, falls inside the
		   subpatch), false otherwise */
		inline bool isValid() const {
			return r >= 0 && c >= 0 && r < max && c < max;
		}
	};

	/**
	 * struct Connection - Put two pixels in a neighboring relation
	 *
	 * @p1: first pixel of the relation (as column offset)
	 * @p2: second pixel of the relation (as column offset)
	 */
	struct Connection {
		const unsigned int p1;
		const unsigned int p2;

		/* Identify the two pixels in the relation starting from their
		   row-column numbers */
		Connection(const PixLoc &a, const PixLoc &b,
			   const unsigned int rows) :
			p1((unsigned int)(a.r + (int)rows * a.c)),
			p2((unsigned int)(b.r + (int)rows * b.c)) {
			assert(a.isValid());
			assert(b.isValid());
			assert(rows > 0);
		};
	};

	std::vector< std::vector< EMat > > M;
	std::vector< unsigned int > sizes;
	std::vector< float > smoothingValues;

	/**
	 * createSmoothingMatrixOnes() - Compute a smoothing matrix given its
	 *				 size and put ones (or minus ones) at
	 *				 the appropriate positions
	 *
	 * @size: size of the subpatch this matrix smooths
	 *
	 * Return: Matrix of the appropriate size with ones in the desired
	 *	   positions
	 */
	EMat createSmoothingMatrixOnes(const unsigned int size) const;

	/**
	 * getPosLambda() - Get the position of the considered matrix according
	 *		    to its smoothing value (the cols int the matrix of
	 *		    values)
	 *
	 * @lambda: considered lambda value
	 *
	 * Return: The desired position if the lambda value is present in the
	 *	   smoothing values vector, -1 otherwise
	 */
	int
	getPosLambda(const float lambda) const {
		std::vector< float >::const_iterator it;
		if ((it = std::find(smoothingValues.begin(),
				    smoothingValues.end(),
				    lambda)) != smoothingValues.end()) {
			return std::distance(smoothingValues.begin(), it);
		}
		return -1;
	}

	/**
	 * getPosSize() - Get the position of the considered matrix according to
	 *		  its size (the rows int the matrix of values)
	 *
	 * @size: considered size
	 *
	 * Return: The desired position if the size is present in the size
	 *	   vector, -1 otherwise
	 */
	int
	getPosSize(const unsigned int size) const {
		std::vector< unsigned int >::const_iterator it;
		if ((it = std::find(sizes.begin(), sizes.end(),
				    size)) != sizes.end()) {
			return std::distance(sizes.begin(), it);
		}
		return -1;
	}
};

#endif /* SMOOTHING_MATRICES_HPP_ */
