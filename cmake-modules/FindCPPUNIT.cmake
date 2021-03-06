# <<<<<<< HEAD
# Find the CppUnit includes and library
#
# This module defines
# CPPUNIT_INCLUDE_DIR, where to find tiff.h, etc.
# CPPUNIT_LIBRARIES, the libraries to link against to use CppUnit.
# CPPUNIT_FOUND, If false, do not try to use CppUnit.

# also defined, but not for general use are
# CPPUNIT_LIBRARY, where to find the CppUnit library.
# CPPUNIT_DEBUG_LIBRARY, where to find the CppUnit library in debug
# mode.

SET(CPPUNIT_FOUND "NO")

FIND_PATH(CPPUNIT_INCLUDE_DIR cppunit/TestCase.h /usr/local/include /usr/include)

# With Win32, important to have both
IF(WIN32)
  FIND_LIBRARY(CPPUNIT_LIBRARY cppunit
               ${CPPUNIT_INCLUDE_DIR}/../lib
               /usr/local/lib
               /usr/lib)
  FIND_LIBRARY(CPPUNIT_DEBUG_LIBRARY cppunitd
               ${CPPUNIT_INCLUDE_DIR}/../lib
               /usr/local/lib
               /usr/lib)
ELSE(WIN32)
  # On unix system, debug and release have the same name
  FIND_LIBRARY(CPPUNIT_LIBRARY cppunit
               ${CPPUNIT_INCLUDE_DIR}/../lib
               /usr/local/lib
               /usr/lib)
  FIND_LIBRARY(CPPUNIT_DEBUG_LIBRARY cppunit
               ${CPPUNIT_INCLUDE_DIR}/../lib
               /usr/local/lib
               /usr/lib)
ENDIF(WIN32)

IF(CPPUNIT_INCLUDE_DIR)
  IF(CPPUNIT_LIBRARY)
    SET(CPPUNIT_FOUND "YES")
    SET(CPPUNIT_LIBRARIES ${CPPUNIT_LIBRARY} ${CMAKE_DL_LIBS})
    SET(CPPUNIT_DEBUG_LIBRARIES ${CPPUNIT_DEBUG_LIBRARY} ${CMAKE_DL_LIBS})
  ELSE (CPPUNIT_LIBRARY)
    IF (CPPUNIT_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could not find library CppUnit.")
    ENDIF (CPPUNIT_FIND_REQUIRED)
  ENDIF(CPPUNIT_LIBRARY)
ELSE(CPPUNIT_INCLUDE_DIR)
  IF (CPPUNIT_FIND_REQUIRED)
    MESSAGE(SEND_ERROR "Could not find library CppUnit.")
  ENDIF(CPPUNIT_FIND_REQUIRED)
ENDIF(CPPUNIT_INCLUDE_DIR)
# =======
# =======
# # http://www.cmake.org/pipermail/cmake/2006-October/011446.html
# #
# # Find the CppUnit includes and library
# #
# # This module defines
# >>>>>>> 538e700ebda1123fbfa25faa908f6e5ed7af101c
# # CPPUNIT_INCLUDE_DIRS, where to find tiff.h, etc.
# # CPPUNIT_LIBRARIES, the libraries to link against to use CppUnit.
# # CPPUNIT_FOUND, If false, do not try to use CppUnit.

# INCLUDE(FindPkgConfig)
# PKG_CHECK_MODULES(CPPUNIT "cppunit")
# IF(NOT CPPUNIT_FOUND)
# FIND_PATH(CPPUNIT_INCLUDE_DIRS cppunit/TestCase.h
#   /usr/local/include
#   /usr/include
# )

# FIND_LIBRARY(CPPUNIT_LIBRARIES cppunit
#            ${CPPUNIT_INCLUDE_DIRS}/../lib
#            /usr/local/lib
#            /usr/lib)
# IF(CPPUNIT_INCLUDE_DIRS)
#   IF(CPPUNIT_LIBRARIES)
#     SET(CPPUNIT_FOUND "YES")
#     SET(CPPUNIT_LIBRARIES ${CPPUNIT_LIBRARIES} ${CMAKE_DL_LIBS})
#   ENDIF(CPPUNIT_LIBRARIES)
# ENDIF(CPPUNIT_INCLUDE_DIRS)

# INCLUDE(FindPackageHandleStandardArgs)

# FIND_PACKAGE_HANDLE_STANDARD_ARGS(CPPUNIT DEFAULT_MSG CPPUNIT_LIBRARIES CPPUNIT_INCLUDE_DIRS)

# ENDIF(NOT CPPUNIT_FOUND)
# <<<<<<< HEAD
# >>>>>>> 538e700ebda1123fbfa25faa908f6e5ed7af101c
# =======
# >>>>>>> 538e700ebda1123fbfa25faa908f6e5ed7af101c
