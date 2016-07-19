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
#include <fstream>

#include "Parameters.hpp"
#include "Dataset.hpp"
#include "SmoothingMatrices.hpp"
#include "KernelBoost.hpp"
#include "logging.hpp"
#include "utils.hpp"

int
main(int argc, char **argv)
{
	/* Initialize RNG */
	srand(12345);

	log_info("Loading parameters...");
	Parameters params(argc, argv);
	if (params.simName == "") {
		/* Requested 'help' in command line args */
		return EXIT_SUCCESS;
	}

	log_info("Loading dataset...");
	Dataset dataset(params);

	log_info("Creating directories...");
	if (createDirectories(params, dataset) == -EXIT_FAILURE) {
		return -EXIT_FAILURE;
	}

	log_info("Pre-allocating smoothing matrices...\n");
	SmoothingMatrices SM(params.minFilterSize,
			     params.maxFilterSize,
			     params.smoothingValues);

	/* Checking that we can open the file, to avoid wasting time without
	   being able to do so later! */
	std::ofstream file(params.classifierPath);
	if (!file.is_open()) {
		log_err("Unable to open destination file %s",
			params.classifierPath.c_str());
		return -EXIT_FAILURE;
	}

	log_info("Learning KernelBoost classifier...");
	KernelBoost kb(params, SM, dataset);

	std::string KBClassifier_json;
	Json::Value root;
	kb.serialize(root, params);
	Json::StyledWriter writer;
	KBClassifier_json = writer.write(root);
	file << KBClassifier_json;
	file.close();

	return EXIT_SUCCESS;
}
