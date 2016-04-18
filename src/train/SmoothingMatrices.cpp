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
#include <vector>
#include <queue>
#include <cassert>

#include "DataTypes.hpp"
#include "SmoothingMatrices.hpp"

/**
 * TRY() - Explore neighboring pixel in the given direction
 *
 * @dx: row variation
 * @dy: column variation
 *
 * Note: requires the variables queue, size, and connections to be defined!
 */
#define TRY(dx, dy)						\
 do {						                \
	 PixLoc newLoc(loc.r + (dx), loc.c + (dy), (int)size);	\
	 if (!newLoc.isValid()) {				\
		 break;						\
	 }							\
	 if (tmp.coeff(newLoc.r, newLoc.c) != 0) {		\
		 break;						\
	 }							\
	 queue.push(newLoc);					\
								\
	 connections.push_back(Connection(loc,			\
					  newLoc, size));	\
 } while (0);

SmoothingMatrices::SmoothingMatrices(const unsigned int minSize,
				     const unsigned int maxSize,
				     const std::vector< float > &smoothingValues)
	: smoothingValues(smoothingValues)
{
	assert(minSize > 1);
	assert(minSize % 2 == 1);
	assert(maxSize >= minSize);
	assert(maxSize % 2 == 1);
	assert(smoothingValues.size() > 0);

	for (unsigned int i = minSize; i <= maxSize; i+=2) {
		sizes.push_back(i);
	}

	const unsigned int nSize = sizes.size();

	M.resize(nSize);

	/* Valgrind will complain a lot about this -- but this is a problem in
	   the way Valgrind detects issues, not a real leak */
#pragma omp parallel for schedule(dynamic)
	for (unsigned int iS = 0; iS < nSize; ++iS) {
		M[iS].resize(smoothingValues.size());

		EMat smOnes = createSmoothingMatrixOnes(sizes[iS]);
		for (unsigned int iL = 0; iL < smoothingValues.size(); ++iL) {
			assert (smoothingValues[iL] > 0);
			M[iS][iL] = smOnes*sqrt(smoothingValues[iL]);
		}
	}
}

EMat
SmoothingMatrices::createSmoothingMatrixOnes(const unsigned int size) const
{
	assert(size > 0);

	const unsigned int connectionsNo = ((size-2) * (size-2) * 4 + 8 +
					    (size-2) * 12) / 2;

	/* Connection vector pairs */
	std::vector< Connection > connections;
	connections.reserve(connectionsNo);

	/* Temporary matrix used to mark already-explored neighborhoods */
	EMat tmp(size, size);
	tmp.setZero();

	/* Exploring queue */
	std::queue< PixLoc > queue;
	queue.push(PixLoc((int)size/2, (int)size/2, (int)size));

	/* Visit all neighboring pixels and put them as pairs in the connections
	   vector */
	while (!queue.empty()) {
		PixLoc loc = queue.front();
		queue.pop();

		if (tmp.coeff(loc.r, loc.c) != 0) {
			/* We've already explored this pixel */
			continue;
		}

		/* Check pixels in a 4-neighborhood */
		TRY( 1,  0);
		TRY( 0,  1);
		TRY(-1,  0);
		TRY( 0, -1);

		/* Mark as visited */
		tmp.coeffRef(loc.r, loc.c) = 1;
	}

	EMat X(connectionsNo, size * size);
	X.setZero();

	for (unsigned int iR = 0; iR < connectionsNo; ++iR) {
		X.coeffRef(iR, connections[iR].p1) = 1.0;
		X.coeffRef(iR, connections[iR].p2) = -1.0;
	}

	return X;
}
