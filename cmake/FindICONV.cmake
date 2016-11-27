################################################################################
# Project:  external projects
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015, NextGIS <info@nextgis.com>
# Copyright (C) 2015 Dmitry Baryshnikov
#
# This script is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This script is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this script.  If not, see <http://www.gnu.org/licenses/>.
################################################################################
# - Try to find Iconv
# Once done this will define
#
#  ICONV_FOUND - system has Iconv
#  ICONV_INCLUDE_DIR - the Iconv include directory
#  ICONV_LIBRARIES - Link these to use Iconv
#  ICONV_SECOND_ARGUMENT_IS_CONST - the second argument for iconv() is const
#
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)

if (ICONV_INCLUDE_DIR AND ICONV_LIBRARIES)
  # Already in cache, be silent
  set(ICONV_FIND_QUIETLY TRUE)
endif ()

if(APPLE)
    find_path(ICONV_INCLUDE_DIR iconv.h
             PATHS
             /opt/local/include/
             NO_CMAKE_SYSTEM_PATH
    )

    find_library(ICONV_LIBRARIES NAMES iconv libiconv c
             PATHS
             /opt/local/lib/
             NO_CMAKE_SYSTEM_PATH
    )
endif()

find_path(ICONV_INCLUDE_DIR iconv.h PATHS /opt/local/include /sw/include /usr/include /usr/local/include)

string(REGEX REPLACE "(.*)/include/?" "\\1" ICONV_INCLUDE_BASE_DIR "${ICONV_INCLUDE_DIR}")

find_library(ICONV_LIBRARIES NAMES libiconv iconv libiconv.lib libiconv.dylib)

if(NOT ICONV_LIBRARIES AND UNIX)
    find_library(ICONV_LIBRARIES NAMES c libc)
endif()

if(ICONV_INCLUDE_DIR AND ICONV_LIBRARIES)
    set(CMAKE_REQUIRED_INCLUDES ${ICONV_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${ICONV_LIBRARIES})
    check_c_compiler_flag("-Werror" ICONV_HAVE_WERROR)
    set (CMAKE_C_FLAGS_BACKUP "${CMAKE_C_FLAGS}")
    if(ICONV_HAVE_WERROR)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    endif(ICONV_HAVE_WERROR)
    check_c_source_compiles("
        #include <iconv.h>
        int main(){
        iconv_t conv = 0;
        const char* in = 0;
        size_t ilen = 0;
        char* out = 0;
        size_t olen = 0;
        iconv(conv, &in, &ilen, &out, &olen);
        return 0;
        }
    " ICONV_SECOND_ARGUMENT_IS_CONST )
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS_BACKUP}")
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ICONV DEFAULT_MSG ICONV_INCLUDE_DIR ICONV_LIBRARIES)

# Copy the results to the output variables.
if(ICONV_FOUND)
  set(ICONV_LIBRARY ${ICONV_LIBRARIES})
  set(ICONV_INCLUDE_DIRS ${ICONV_INCLUDE_DIR})
endif()

mark_as_advanced(
    ICONV_INCLUDE_DIR
    ICONV_LIBRARIES
    ICONV_SECOND_ARGUMENT_IS_CONST
)
