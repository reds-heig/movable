################################################################################
## MOVABLE project - REDS Institute, HEIG-VD, Yverdon-les-Bains (CH) - 2016   ##
##                                                                            ##
## This file is part of MOVABLE.                                              ##
##                                                                            ##
##  MOVABLE is free software: you can redistribute it and/or modify           ##
##  it under the terms of the GNU General Public License as published by      ##
##  the Free Software Foundation, either version 3 of the License, or         ##
##  (at your option) any later version.                                       ##
##                                                                            ##
##  MOVABLE is distributed in the hope that it will be useful,                ##
##  but WITHOUT ANY WARRANTY; without even the implied warranty of            ##
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             ##
##  GNU General Public License for more details.                              ##
##                                                                            ##
##  You should have received a copy of the GNU General Public License         ##
##  along with MOVABLE.  If not, see <http://www.gnu.org/licenses/>.          ##
################################################################################

add_definitions (-DINFO_MSG)
add_definitions (-DMOVABLE_TRAIN)
remove_definitions (-DTESTS)

include_directories (../shared)
include_directories (.)

set (SHARED_HDRS
  ../shared/BoostedClassifier.hpp
  ../shared/Dataset.hpp
  ../shared/DataTypes.hpp
  ../shared/FilterBank.hpp
  ../shared/JSONSerializable.hpp
  ../shared/JSONSerializer.hpp
  ../shared/KernelBoost.hpp
  ../shared/logging.hpp
  ../shared/macros.hpp
  ../shared/RegTree.hpp
  ../shared/utils.hpp
  ../shared/WeakLearner.hpp
  )

set (SHARED_SRCS
  ../shared/BoostedClassifier.cpp
  ../shared/Dataset.cpp
  ../shared/FilterBank.cpp
  ../shared/JSONSerializer.cpp
  ../shared/KernelBoost.cpp
  ../shared/RegTree.cpp
  ../shared/utils.cpp
  ../shared/WeakLearner.cpp
  )

set (TRAIN_HDRS
  LineSearch.hpp
  SmoothingMatrices.hpp
  Parameters.hpp
  )

set (TRAIN_SRCS
  SmoothingMatrices.cpp
  main.cpp
  )

CONFIGURE_FILE (train_config.json train_config.json COPYONLY)
add_executable (train_movable ${TRAIN_SRCS} ${TRAIN_HDRS} ${SHARED_SRCS} ${SHARED_HDRS})
target_link_libraries (train_movable ${OpenCV_LIBS} lbfgs ${JSONCPP_LIBS})
