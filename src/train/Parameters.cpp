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

#include "Parameters.hpp"

/**
 * Parameters() - Load a set of parameters from the JSON configuration
 *		  file
 *
 * @argc: command line's argument count
 * @argv: command line's argument list
 */
Parameters::Parameters(int argc, char **argv)
{
	/* Command line arguments */
	struct arg_lit *help = arg_litn(NULL,
					"help",
					0, 1,
					"print this help and exit");

	struct arg_file *arg_config = arg_filen(NULL,
						"config-file",
						"config-file-path",
						0, 1,
						"load the specified "
						"configuration file "
						"(default: ./train_config.json)");

	struct arg_str *arg_simName = arg_strn(NULL,
					       "sim-name",
					       "simulation-name",
					       1, 1,
					       "name given to the "
					       "simulation");

	struct arg_end *end = arg_end(20);

	void *argtable[] = { help,
			     arg_config,
			     arg_simName,
			     end };

	/* Verify that argtable entries are successfully allocated */
	if (arg_nullcheck(argtable) != 0) {
		log_err("Error occurred while allocating memory for parameters");
		arg_freetable(argtable,
			      sizeof(argtable)/sizeof(argtable[0]));
		throw std::runtime_error("commandLineParsingError");
	}
	/* Parse command line */
	int errors_no = arg_parse(argc, argv, argtable);

	if (help->count > 0) {
		log_err("Usage: %s", argv[0]);
		arg_print_syntax(stderr, argtable, "\n");
		arg_print_glossary(stderr, argtable, "	%-45s %s\n");
	} else {
		if (errors_no > 0) {
			arg_print_errors(stderr, end, argv[0]);
			log_err("Try '%s --help' for more informations.\n",
				argv[0]);
			arg_freetable(argtable,
				      sizeof(argtable)/sizeof(argtable[0]));
			throw std::runtime_error("commandLineParsingError");
		}

		simName = arg_simName->sval[0];

		if (arg_config->count == 1) {
			configFName = arg_config->filename[0];
		} else {
			configFName = defaultConfigFName;
		}

		arg_freetable(argtable,
			      sizeof(argtable)/sizeof(argtable[0]));

		std::ifstream configData(configFName,
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
}
