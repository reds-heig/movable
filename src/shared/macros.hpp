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

#ifndef MACROS_HPP_
#define MACROS_HPP_

#define CHECK_DIR_EXISTS(DIR_PATH)                                  \
    do {                                                            \
        if (access(DIR_PATH, F_OK) != 0) {                          \
            log_err("The directory %s does not exist!", DIR_PATH);  \
            throw std::runtime_error("dirNotExists");               \
        }                                                           \
        struct stat status;                                         \
        stat(DIR_PATH, &status);                                    \
        if (!(status.st_mode & S_IFDIR)) {                          \
            log_err("The path %s does not correspond to a "         \
                    "directory!", DIR_PATH);                        \
            throw std::runtime_error("pathNotDirectory");           \
        }                                                           \
    } while (0);

#define CHECK_FILE_EXISTS(FILE_PATH)                            \
    do {                                                        \
        if (access(FILE_PATH, F_OK) != 0) {                     \
            log_err("The file %s does not exist!", FILE_PATH);  \
            throw std::runtime_error("fileNotExists");          \
        }                                                       \
        struct stat status;                                     \
        stat(FILE_PATH, &status);                               \
        if (status.st_mode & S_IFDIR) {                         \
            log_err("The path %s does not correspond to a "     \
                    "file!", FILE_PATH);                        \
            throw std::runtime_error("pathNotFile");            \
        }                                                       \
    } while (0);

#define GET_STRING_PARAM(PARAM_NAME)                                \
    do {                                                            \
        if (!root.isMember(#PARAM_NAME)) {                          \
            log_err("Cannot retrieve parameter %s", #PARAM_NAME);   \
            throw std::runtime_error("invalidParameter");           \
        }                                                           \
        PARAM_NAME = root[#PARAM_NAME].asString();                  \
    } while (0);

#define GET_FLOAT_PARAM(PARAM_NAME)                                 \
    do {                                                            \
        if (!root.isMember(#PARAM_NAME)) {                          \
            log_err("Cannot retrieve parameter %s", #PARAM_NAME);   \
            throw std::runtime_error("invalidParameter");           \
        }                                                           \
        PARAM_NAME = root[#PARAM_NAME].asFloat();                   \
    } while (0);

#define GET_INT_PARAM(PARAM_NAME)                                   \
    do {                                                            \
        if (!root.isMember(#PARAM_NAME)) {                          \
            log_err("Cannot retrieve parameter %s", #PARAM_NAME);   \
            throw std::runtime_error("invalidParameter");           \
        }                                                           \
        PARAM_NAME = root[#PARAM_NAME].asInt();                     \
    } while (0);

#define GET_BOOL_PARAM(PARAM_NAME)                                  \
    do {                                                            \
        if (!root.isMember(#PARAM_NAME)) {                          \
            log_err("Cannot retrieve parameter %s", #PARAM_NAME);   \
            throw std::runtime_error("invalidParameter");           \
        }                                                           \
        PARAM_NAME = root[#PARAM_NAME].asBool();                    \
    } while (0)

#define GET_INT_ARRAY(PARAM_NAME)                                   \
    do {                                                            \
        if (!root.isMember(#PARAM_NAME)) {                          \
            log_err("Cannot retrieve parameter %s", #PARAM_NAME);   \
            throw std::runtime_error("invalidParameter");           \
        }                                                           \
        for (const Json::Value& val : root[#PARAM_NAME]) {          \
            PARAM_NAME.push_back(val.asInt());                      \
        }                                                           \
    } while (0);

#define GET_FLOAT_ARRAY(PARAM_NAME)                                 \
    do {                                                            \
        if (!root.isMember(#PARAM_NAME)) {                          \
            log_err("Cannot retrieve parameter %s", #PARAM_NAME);   \
            throw std::runtime_error("invalidParameter");           \
        }                                                           \
        for (const Json::Value& val : root[#PARAM_NAME]) {          \
            PARAM_NAME.push_back(val.asDouble());                   \
        }                                                           \
    } while (0);

#define GET_STRING_ARRAY(PARAM_NAME)                                \
    do {                                                            \
        if (!root.isMember(#PARAM_NAME)) {                          \
            log_err("Cannot retrieve parameter %s", #PARAM_NAME);   \
            throw std::runtime_error("invalidParameter");           \
        }                                                           \
        for (const Json::Value& val : root[#PARAM_NAME]) {          \
            PARAM_NAME.push_back(val.asString());                   \
        }                                                           \
    } while (0);

#endif /* MACROS_HPP_ */
