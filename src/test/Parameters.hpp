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
 * @datasetPath    : dataset's path
 * @datasetName    : dataset's name
 * @imgPathFName   : name of the file containing image paths
 * @maskPathFName  : name of the file containing mask paths
 * @sampleSize     : lateral size (in pixels) of each sample (for
 *                   instance, if this is 51, this means sampling
 *                   51x51 squared around the sample point)
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

	unsigned int sampleSize;

	std::string baseResDir;
	std::vector< std::string > channelList;

	/**
	 * Parameters() - Empty constructor for testing
	 */
	Parameters() { };

	/**
	 * Parameters() - Load a set of parameters from the JSON configuration
	 *                file
	 *
	 * @simName       : name of the current simulation
	 * @classifierPath: path of the trained classifier
	 */
	Parameters(const std::string &simName,
		   const std::string &classifierPath)
		: simName(simName), classifierPath(classifierPath)
	{
		std::ifstream configData("test_config.json",
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

		GET_STRING_PARAM(datasetPath);
		CHECK_DIR_EXISTS(datasetPath.c_str());

		GET_STRING_PARAM(datasetName);

		GET_STRING_PARAM(imgPathsFName);
		CHECK_FILE_EXISTS((datasetPath + std::string("/")
				   + imgPathsFName).c_str());
		GET_STRING_PARAM(maskPathsFName);
		CHECK_FILE_EXISTS((datasetPath + std::string("/")
				   + maskPathsFName).c_str());

		GET_INT_PARAM(sampleSize);
		GET_STRING_ARRAY(channelList);
	}
};

#endif /* PARAMETERS_HPP_ */
