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

#ifndef PARAMETERS_HPP_
#define PARAMETERS_HPP_

#include <iostream>
#include <string>
#include <fstream>

#include <unistd.h>
#include "json/json.h"
#include <sys/types.h>
#include <sys/stat.h>

extern "C" {
#include "argtable3.h"
}
#include "DataTypes.hpp"
#include "macros.hpp"
#include "logging.hpp"

/* Default configuration file name */
const std::string defaultConfigFName = "test_config.json";

/**
 * class Parameters - Set of parameters for the application
 *
 * @simName	    : codename of the simulation
 * @resultsDir	    : base path of the results directory
 * @datasetPath	    : dataset's path
 * @datasetName	    : dataset's name
 * @imgPathFName    : name of the file containing image paths
 * @maskPathFName   : name of the file containing mask paths
 * @sampleSize	    : lateral size (in pixels) of each sample (for
 *		      instance, if this is 51, this means sampling
 *		      51x51 squared around the sample point)
 * @imgRescaleFactor: rescale input images by this factor (that is, divide each
 *                    image coordinate by this value)
 * @threshold	    : fixed threshold applied when binarizing the image
 * @baseResDir	    : base directory path for output results
 * @channelList	    : list of channels requested by user
 * @configFName	    : path of the configuration file
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
	unsigned int imgRescaleFactor;

	float threshold;

	std::string baseResDir;
	std::vector< std::string > channelList;

	/**
	 * Parameters() - Empty constructor for testing
	 */
	Parameters() { };

	/**
	 * Parameters() - Load a set of parameters from the JSON configuration
	 *		  file and the command line
	 *
	 * @argc    : command line's argument count
	 * @argv    : command line's argument list
	 */
	Parameters(int argc, char **argv);

private:
	std::string configFName;
};

#endif /* PARAMETERS_HPP_ */
