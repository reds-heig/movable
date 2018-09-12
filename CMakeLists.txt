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

cmake_minimum_required (VERSION 3.1)
project (MOVABLE)
enable_language (CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake-modules")

set (EXT_PROJECTS_DIR ${CMAKE_SOURCE_DIR}/libs)

find_package (Jsoncpp REQUIRED)
include_directories (${JSONCPP_INCLUDE_DIRS})

find_package (OpenCV REQUIRED)
include_directories (${OpenCV_INCLUDE_DIRS})

include_directories (${EXT_PROJECTS_DIR}/Catch/include)
enable_testing (true)

link_directories (${EXT_PROJECTS_DIR}/liblbfgs/lib/.libs)
link_directories (${EXT_PROJECTS_DIR}/argtable_3.0.1)

set (EIGEN3_INCLUDE_DIR "${EXT_PROJECTS_DIR}/eigen")
if (NOT EIGEN3_INCLUDE_DIR)
    message (FATAL_ERROR "Please point the environment variable EIGEN3_INCLUDE_DIR to the include directory of your Eigen3 installation.")
endif ()
include_directories ("${EIGEN3_INCLUDE_DIR}")

find_package (OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

include_directories (${EXT_PROJECTS_DIR})
include_directories (${EXT_PROJECTS_DIR}/liblbfgs/include)
include_directories (${EXT_PROJECTS_DIR}/argtable_3.0.1)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release" FORCE)
endif (NOT CMAKE_BUILD_TYPE)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (CMAKE_BUILD_TYPE MATCHES Release)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
endif (CMAKE_BUILD_TYPE MATCHES Release)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wcast-align -Wswitch-default -Wundef -O3 -fno-omit-frame-pointer -DDEBUG")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

add_subdirectory (libs)
add_subdirectory (src)
