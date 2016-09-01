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
#include <vector>
#include <cassert>

#include "DataTypes.hpp"
#include "FilterBank.hpp"
#include "utils.hpp"

FilterBank::FilterBank(Json::Value &root)
{
	Deserialize(root);
}

FilterBank::FilterBank(const FilterBank &obj)
{
	this->filters = obj.filters;
}
bool
operator==(const FilterBank &fb1, const FilterBank &fb2)
{
	if (fb1.filters == fb2.filters) {
		return true;
	}
	return false;
}

bool
operator!=(const FilterBank &fb1, const FilterBank &fb2)
{
	return !(fb1 == fb2);
}


#ifdef MOVABLE_TRAIN
FilterBank::FilterBank(const Parameters &params,
		       const SmoothingMatrices &SM,
		       const Dataset &dataset,
		       const unsigned int chNo,
		       const sampleSet &samplePositions,
		       const EVec &weights)
{
	assert (!samplePositions.empty());
	assert ((size_t)weights.size() == samplePositions.size());
	assert (chNo < dataset.getDataChNo());

	const unsigned int minFilterSize = params.minFilterSize;
	const unsigned int maxFilterSize = params.maxFilterSize;
	const unsigned int filtersNo = params.filtersPerChNo;
	const unsigned int randSamplesNo = params.randSamplesNo;
	const unsigned int patchSize = params.sampleSize;
	const std::vector< float > smoothingValues = params.smoothingValues;

	assert (smoothingValues.size() > 0);

	/* Pre-allocate the space for learned filters */
	filters.clear();
	filters.resize(filtersNo);

	/* Compute normalization factors for the weights */
	EVec sqrtW = weights.array().sqrt();

#pragma omp parallel for schedule(dynamic)
	for (unsigned int iF = 0; iF < filtersNo; ++iF) {
		/* Randomly-chosen filter size */
		unsigned int filterSize = minFilterSize +
			(unsigned int)rand() % (maxFilterSize - minFilterSize);
		/* Make the filter size odd */
		if (filterSize % 2 == 0) {
			filterSize++;
		}
		const unsigned int filterArea = filterSize * filterSize;

		/* Maximum row/column value for the filter according to size */
		const unsigned int mRC = patchSize - filterSize;

		const float lambda = smoothingValues[(unsigned int)rand() %
						     smoothingValues.size()];
		/* Upper-left corner of the filter */
		const unsigned int startRow = (unsigned int)rand() % mRC;
		const unsigned int startCol = (unsigned int)rand() % mRC;

		/* Get sample indexes */
		std::vector< unsigned int > samplesIdx =
			randomSamplingWithoutReplacement(randSamplesNo,
							 samplePositions.size());

		/* Selected smoothing value */
		const EMat &smMat = SM.getSmoothingMatrix(filterSize, lambda);

		/* Size of the final X matrix (obtained by concatenating the
		   randomly-taken samples with the smoothing matrix) */
		const unsigned int XSize = randSamplesNo + smMat.rows();

		/* Grab samples */
		EMat samples;
		dataset.getSampleMatrix(samplePositions, samplesIdx, chNo,
					startRow, startCol, filterSize,
					samples);

		/* Compute the per-feature mean and standard deviation */
		ERowVector mean = samples.colwise().sum() / randSamplesNo;
		ERowVector std(filterArea);
		for (unsigned iF = 0; iF < mean.cols(); ++iF) {
			std.coeffRef(iF) =
				sqrt((samples.col(iF).array() -
				      mean.coeff(iF)).square().sum()
				     / randSamplesNo);
		}

		/* Create weight matrix */
		EVec W(XSize);
		/* Create labels matrix */
		EVec Y(XSize);

		/* Weight normalization */
		float sampledSumW = 0;
		for (unsigned int iX = 0; iX < randSamplesNo; ++iX) {
			sampledSumW += weights.coeff(samplesIdx[iX]);
		}
		/* Normalize weights to make them sum up to N */
		const float normSumW = sqrt(randSamplesNo / sampledSumW);

		/* The first part of the weight matrix has, on the diagonal,
		   samples-dependent values, while the lower part takes just the
		   square root of the regularization value.
		   We set, in the same loop, the labels matrix by using the
		   newly-set weight to give an appropriate value to the label */
		for (unsigned int iX = 0; iX < randSamplesNo; ++iX) {
			W.coeffRef(iX) = sqrtW.coeff(samplesIdx[iX])*normSumW;
			Y.coeffRef(iX) =
				W.coeffRef(iX) *
				samplePositions[samplesIdx[iX]].label;
		}
		W.tail(smMat.rows()).setConstant(sqrt(lambda));
		/* Smoothing values have no label */
		Y.tail(smMat.rows()).setZero();

		/* Allocate the matrix used in the computations by concatenating
		   the rescaled samples and the smoothing values */
		EMat X(XSize, filterArea);
		X.block(0, 0, randSamplesNo, filterArea) =
			((samples.rowwise() - mean).array().rowwise() /
			 std.array()).colwise() *
			W.head(randSamplesNo).array();
		X.block(randSamplesNo, 0, smMat.rows(), filterArea) = smMat;

		/* Compute the filter */
		filters[iF].X = ((X.transpose()*X).inverse() *
				 X.transpose() * Y).array() /
			std.transpose().array();
		/* Normalize (filter sum = 1) */
		filters[iF].X /= filters[iF].X.array().sum();

		/* Set the values in the destination filter structure */
		filters[iF].chNo = chNo;
		filters[iF].row = startRow;
		filters[iF].col = startCol;
		filters[iF].size = filterSize;

		/* Copy filter's elements in the cv::Mat's square filter */
		cv::Mat tmpSqX(filterSize, filterSize, CV_32FC1);
		for (unsigned int r = 0; r < filterSize; ++r) {
			for (unsigned int c = 0; c < filterSize; ++c) {
				tmpSqX.at< float >(r, c) =
					filters[iF].X(r*filterSize + c);
			}
		}
		filters[iF].Xsq = tmpSqX;

		/* During learning we keep the filters as columns */
		assert(filters[iF].X.rows() == filterArea);
		assert(filters[iF].X.cols() == 1);
	}
}

FilterBank::FilterBank(const std::vector< FilterBank > &filterBanks,
		       const std::vector< unsigned int > &featsIdxs)
{
	const unsigned int featsNo = featsIdxs.size();
	filters.resize(featsNo);

	unsigned int startFilter = 0;
	unsigned int filterCount = 0;
	for (unsigned int f = 0; f < featsNo; ++f) {
		while (featsIdxs[f] >=
		       filterCount + filterBanks[startFilter].filters.size()) {
			filterCount += filterBanks[startFilter].filters.size();
			++startFilter;
		}
		filters[f] =
			filterBanks[startFilter].filters[featsIdxs[f]-filterCount];
	}
}

FilterBank::FilterBank(std::string &descr_json)
{
	Json::Value root;
	Json::Reader reader;

	if (!reader.parse(descr_json, root)) {
		throw std::runtime_error("invalidJSONDescription");
	}
	Deserialize(root);
}
#endif /* MOVABLE_TRAIN */

void
FilterBank::evaluateFilters(const Dataset &dataset,
			    const sampleSet &samplePositions,
			    EMat& features) const
{
	assert (!samplePositions.empty());

	features.resize(samplePositions.size(), filters.size());

	std::vector< unsigned int > samplesIdx(samplePositions.size());
	std::iota(samplesIdx.begin(), samplesIdx.end(), 0);

#pragma omp parallel for schedule(dynamic)
	for (unsigned int iF = 0; iF < filters.size(); ++iF) {
		EMat samples;
		dataset.getSampleMatrix(samplePositions,
					samplesIdx,
					filters[iF].chNo,
					filters[iF].row,
					filters[iF].col,
					filters[iF].size,
					samples);
		features.col(iF) = samples * filters[iF].X;
	}
}

void
FilterBank::evaluateFiltersOnImage(const std::vector< cv::Mat > &imgVec,
				   const unsigned int borderSize,
				   EMat& features) const
{
	assert (!imgVec.empty());

	const unsigned int nRows = imgVec[0].rows-2*borderSize;
	const unsigned int nCols = imgVec[0].cols-2*borderSize;
	EMat featTmp;
	featTmp.resize(filters.size(), nRows*nCols);

#pragma omp parallel for schedule(dynamic)
	for (unsigned int iF = 0; iF < filters.size(); ++iF) {
		cv::Mat tmp;
		cv::filter2D(imgVec[filters[iF].chNo], tmp,
			     -1, filters[iF].Xsq);
		const unsigned int startRow = borderSize+filters[iF].row+
			floor(filters[iF].size/2);
		const unsigned int startCol = borderSize+filters[iF].col+
			floor(filters[iF].size/2);
		/* We copy data since we cannot reshape a cv::Range
		   (non-contiguous in memory) */
		for (unsigned int r = 0; r < nRows; ++r) {
			for (unsigned int c = 0; c < nCols; ++c) {
				featTmp(iF, r*nCols+c) =
					tmp.at< float >(r+startRow, c+startCol);
			}
		}
	}
	features = featTmp.transpose();
}

void
FilterBank::getChCount(std::vector< int > &count)
{
	for (unsigned int iF = 0; iF < filters.size(); ++iF) {
		count[filters[iF].chNo]++;
	}
}

void
FilterBank::Serialize(Json::Value &root)
{
	Json::Value filters_json(Json::arrayValue);
	for (unsigned int i = 0; i < filters.size(); ++i) {
		Json::Value f(Json::objectValue);

		f["chNo"] = filters[i].chNo;
		f["row"] = filters[i].row;
		f["col"] = filters[i].col;
		f["size"] = filters[i].size;
		/* Store just the Eigen's version of the filter (the OpenCV one
		   will be rebuilt from it) */
		for (unsigned int r = 0; r < filters[i].X.rows(); ++r) {
			f["X"].append(filters[i].X(r, 0));
		}

		filters_json.append(f);
	}

	root["filters"] = filters_json;
}

void
FilterBank::Deserialize(Json::Value &root)
{
	for (Json::Value::iterator it = root["filters"].begin();
	     it != root["filters"].end(); ++it) {
		filter flt;
		flt.chNo = (*it)["chNo"].asInt();
		flt.row = (*it)["row"].asInt();
		flt.col = (*it)["col"].asInt();
		flt.size = (*it)["size"].asInt();
		flt.X.resize(flt.size*flt.size, 1);
		unsigned int r = 0;
		for (Json::Value::iterator F_it = (*it)["X"].begin();
		     F_it != (*it)["X"].end();
		     ++F_it) {
			flt.X(r++, 0) = (*F_it).asDouble();
		}
		cv::Mat tmpSqX(flt.size, flt.size, CV_32FC1);
		for (unsigned int r = 0; r < flt.size; ++r) {
			for (unsigned int c = 0; c < flt.size; ++c) {
				tmpSqX.at< float >(r, c) =
					flt.X(r*flt.size + c);
			}
		}
		flt.Xsq = tmpSqX;

		filters.push_back(flt);
	}
}
