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

#ifndef PARAMETERS_HPP_
#define PARAMETERS_HPP_

#include <iostream>
#include <string>
#include <fstream>

#include <unistd.h>
#include "json/json.h"
#include <sys/types.h>
#include <sys/stat.h>

#include "DataTypes.hpp"
#include "macros.hpp"
#include "logging.hpp"

/**
 * class Parameters - Set of parameters for the application
 *
 * @simName        : codename of the simulation
 * @resultsDir     : base path of the results directory
 * @classifierPath : final classifier's path
 * @datasetPath    : dataset's path
 * @datasetName    : dataset's name
 * @imgPathFName   : name of the file containing image paths
 * @maskPathFName  : name of the file containing mask paths
 * @gtPathFName    : name of the file containing gt paths
 * @regMinVal      : minimum value for the regularization parameter
 * @regMaxVal      : maximum value for the regularization parameter
 * @regValStep     : step between regularization parameter's values
 * @shrinkageFactor: shrinkage factor for the boosting algorithm
 * @gtValues       : list of considered gt values
 * @useColorImages : enable the use of color images in the dataset
 * @datasetBalance : balance the number of positives and negatives in
 *                   the dataset
 * @posSamplesNo   : number of positive samples
 * @negSamplesNo   : number of negative samples
 * @randSamplesNo  : number of samples randomly taken for each
 *                   filter learning round
 * @finalSamplesNo : number of samples for each class used in the training of
 *                   the final classifier
 * @sampleSize     : lateral size (in pixels) of each sample (for
 *                   instance, if this is 51, this means sampling
 *                   51x51 squared around the sample point)
 * @filtersPerChNo : number of filters to learn for each channel
 * @minFilterSize  : minimum filter size
 * @maxFilterSize  : maximum filter size
 * @wlNo           : number of weak-learners to learn
 * @treeDepth      : maximum depth of the regression trees
 * @finalTreeDepth : depth of the final tree
 * @smoothingValues: list of smoothing values
 * @intermedResDir : directories where the intermediate results will be
 *                   stored
 * @finalResDir    : directory where final results will be stored
 * @channelList    : list of channels requested by user
 */
class Parameters {
public:
	std::string simName;
	std::string resultsDir;
	std::string classifierPath;
	std::string datasetPath;
	std::string datasetName;
	std::string imgPathsFName;
	std::string maskPathsFName;
	std::string gtPathsFName;

	float shrinkageFactor;

	float regMinVal;
	float regMaxVal;
	float regValStep;

	std::vector< int > gtValues;

	bool useColorImages;
	bool datasetBalance;

	unsigned int posSamplesNo;
	unsigned int negSamplesNo;
	unsigned int randSamplesNo;
	unsigned int finalSamplesNo;
	unsigned int sampleSize;
	unsigned int filtersPerChNo;
	unsigned int minFilterSize;
	unsigned int maxFilterSize;
	unsigned int wlNo;
	unsigned int treeDepth;
	unsigned int finalTreeDepth;

	std::vector< std::string > channelList;

	/* Computed values */
	std::vector< float > smoothingValues;
	std::string baseResDir;
	std::vector< std::string > intermedResDir;
	std::string finalResDir;

	/**
	 * Parameters() - Empty constructor for testing
	 */
	Parameters() { };

	/**
	 * Parameters() - Load a set of parameters from the JSON configuration
	 *                file
	 *
	 * @simName: name of the current simulation
	 */
	Parameters(const std::string &simName)
		: simName(simName)
	{
		std::ifstream configData("train_config.json",
					 std::ifstream::binary);
		Json::Reader reader;
		Json::Value root;
		if (!reader.parse(configData, root)) {
			std::cerr << reader.getFormatedErrorMessages() << "\n";
		}

		GET_STRING_PARAM(resultsDir);

		/* Ensure that no previous simulation had the same name */
		baseResDir = resultsDir + std::string("/") + simName;
		if (access(baseResDir.c_str(), F_OK) == 0) {
			log_err("The results directory %s already exist!",
				baseResDir.c_str());
			log_err("Please choose another simulation name.");
			throw std::runtime_error("resultsDirAlreadyExists");
		}
		classifierPath = baseResDir+std::string("/")+"classifier.json";

		GET_STRING_PARAM(datasetPath);
		CHECK_DIR_EXISTS(datasetPath.c_str());

		GET_STRING_PARAM(datasetName);

		GET_STRING_PARAM(imgPathsFName);
		CHECK_FILE_EXISTS((datasetPath + std::string("/")
				   + imgPathsFName).c_str());
		GET_STRING_PARAM(maskPathsFName);
		CHECK_FILE_EXISTS((datasetPath + std::string("/")
				   + maskPathsFName).c_str());
		GET_STRING_PARAM(gtPathsFName);
		CHECK_FILE_EXISTS((datasetPath + std::string("/")
				   + gtPathsFName).c_str());

		GET_FLOAT_PARAM(shrinkageFactor);

		GET_FLOAT_PARAM(regMinVal);
		GET_FLOAT_PARAM(regMaxVal);
		GET_FLOAT_PARAM(regValStep);
		for (float s = regMinVal; s < regMaxVal; s += regValStep) {
			smoothingValues.push_back(s);
		}

		GET_INT_ARRAY(gtValues);

		GET_BOOL_PARAM(useColorImages);
		GET_BOOL_PARAM(datasetBalance);

		GET_INT_PARAM(posSamplesNo);
		GET_INT_PARAM(negSamplesNo);
		GET_INT_PARAM(finalSamplesNo);
		GET_INT_PARAM(randSamplesNo);
		GET_INT_PARAM(sampleSize);
		GET_INT_PARAM(filtersPerChNo);
		GET_INT_PARAM(minFilterSize);
		GET_INT_PARAM(maxFilterSize);
		GET_INT_PARAM(wlNo);
		GET_INT_PARAM(treeDepth);
		GET_INT_PARAM(finalTreeDepth);

		GET_STRING_ARRAY(channelList);
	}
};

#endif /* PARAMETERS_HPP_ */
