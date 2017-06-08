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
const std::string defaultConfigFName = "train_config.json";

/**
 * class Parameters - Set of parameters for the application
 *
 * @simName         : codename of the simulation
 * @resultsDir      : base path of the results directory
 * @classifierPath  : final classifier's path
 * @datasetPath     : dataset's path
 * @datasetName     : dataset's name
 * @imgPathFName    : name of the file containing image paths
 * @maskPathFName   : name of the file containing mask paths
 * @gtPathFName     : name of the file containing gt paths
 * @regMinVal       : minimum value for the regularization parameter
 * @regMaxVal       : maximum value for the regularization parameter
 * @regValStep      : step between regularization parameter's values
 * @shrinkageFactor : shrinkage factor for the boosting algorithm
 * @gtValues        : list of considered gt values
 * @datasetBalance  : balance the number of positives and negatives in
 *                    the dataset
 * @imgRescaleFactor: rescale input images by this factor (that is, divide each
 *                    image coordinate by this value)
 * @posSamplesNo    : number of positive samples
 * @negSamplesNo    : number of negative samples
 * @randSamplesNo   : number of samples randomly taken for each
 *                    filter learning round
 * @finalSamplesNo  : number of samples for each class used in the training of
 *                    the final classifier
 * @sampleSize      : lateral size (in pixels) of each sample (for
 *                    instance, if this is 51, this means sampling
 *                    51x51 squared around the sample point)
 * @filtersPerChNo  : number of filters to learn for each channel
 * @minFilterSize   : minimum filter size
 * @maxFilterSize   : maximum filter size
 * @nRotations      : number of rotated versions of the training samples that
 *                    have to be considered
 * @houghMinDist    : minimum distance between RBCs for the Hough method
 * @houghHThresh    : higher threshold on Canny's output in the Hough method
 * @houghLThresh    : lower threshold on Canny's output in the Hough method
 * @houghMinRad     : minimum radius for the retained RBCs in the Hough method
 * @houghMaxRad     : maximum radius for the retained RBCs in the Hough method
 * @wlNo            : number of weak-learners to learn
 * @treeDepth       : maximum depth of the regression trees
 * @finalTreeDepth  : depth of the final tree
 * @smoothingValues : list of smoothing values
 * @fastClassifier  : enable fast classification (only candidate points are
 *                    tested)
 * @RBCdetection    : in fast classification mode, enlarge candidate points to
 *                    the RBCs containing them
 * @useAutoContext  : enable the use of AutoContext
 * @intermedResDir  : directories where the intermediate results will be
 *                    stored
 * @finalResDir     : directory where final results will be stored
 * @channelList     : list of channels requested by user
 * @configFName     : path of the configuration file
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

    bool datasetBalance;
    unsigned int imgRescaleFactor;

    unsigned int posSamplesNo;
    unsigned int negSamplesNo;
    unsigned int randSamplesNo;
    unsigned int finalSamplesNo;
    unsigned int sampleSize;
    unsigned int filtersPerChNo;
    unsigned int minFilterSize;
    unsigned int maxFilterSize;
    unsigned int nRotations;

    double houghMinDist;
    double houghHThresh;
    double houghLThresh;
    int houghMinRad;
    int houghMaxRad;

    bool fastClassifier;
    bool RBCdetection;
    bool useAutoContext;

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
     *                file and the command line
     *
     * @argc: command line's argument count
     * @argv: command line's argument list
     */
    Parameters(int argc, char **argv);

private:
    std::string configFName;
};

#endif /* PARAMETERS_HPP_ */
