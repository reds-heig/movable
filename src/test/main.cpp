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
#include <fstream>

#include "Parameters.hpp"
#include "Dataset.hpp"
#include "KernelBoost.hpp"
#include "logging.hpp"
#include "utils.hpp"

int
main(int argc, char **argv)
{
	if (argc != 3) {
		log_err("Usage: %s sim_name classifier_path--", argv[0]);
		return -EXIT_FAILURE;
	}

	log_info("Loading parameters...");
	Parameters params(argv[1], argv[2]);

	log_info("Loading dataset...");
	Dataset dataset(params);

	log_info("Creating directories...");
	if (createDirectories(params) == -EXIT_FAILURE) {
		return -EXIT_FAILURE;
	}

	std::ifstream file(argv[2]);
	if (!file.is_open()) {
		log_err("Unable to open classifier file %s", argv[2]);
		return -EXIT_FAILURE;
	}

	log_info("Loading KernelBoost classifier and performing "
		 "classification...");
	std::stringstream KBClassifier_json;
	KBClassifier_json << file.rdbuf();
	file.close();
	std::string KBC_json = KBClassifier_json.str();
	KernelBoost kb(KBC_json, params, dataset);

	return EXIT_SUCCESS;
}
