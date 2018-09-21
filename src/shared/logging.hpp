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

#ifndef LOGGING_HPP_
#define LOGGING_HPP_

extern "C" {
#include <stdio.h>
#include <string.h>
#include <errno.h>
}

/* Message helpers found on StackOverflow */
#ifdef DEBUG
#define debug(M, ...)						\
	do {							\
		fprintf(stderr, "[DEBUG] ");			\
		fprintf(stderr, "%s:%d: " M "\n",		\
			__FILE__, __LINE__, ##__VA_ARGS__);	\
	} while(0);
#else // !DEBUG
#define debug(M, ...)
#endif // DEBUG

#ifdef INFO_MSG
#define log_info(M, ...)				\
	do {						\
		fprintf(stderr, "[INFO] ");		\
		fprintf(stderr, M "\n", ##__VA_ARGS__);	\
	} while (0);
#else // !INFO_MSG
#define log_info(M, ...)
#endif // INFO_MSG

#define log_err(M, ...)						\
	do {							\
		fprintf(stderr, "[ERROR] (%s:%d) " M "\n",	\
			__FILE__, __LINE__, ##__VA_ARGS__);	\
	} while (0);
#define log_warn(M, ...)					\
	do {							\
		fprintf(stderr, "[WARN] (%s:%d) " M "\n",	\
			__FILE__, __LINE__,  ##__VA_ARGS__);	\
	} while (0);
#define log_trace(M, ...)

#endif /* LOGGING_HPP_ */
