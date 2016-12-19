#.rst:
# FindHDFEOS2
# ---------
#
# Find HDFEOS2 kit
#
#   HDFEOS2_INCLUDE_DIRS - where to find HdfEosDef.h, etc.
#   HDFEOS2_LIBRARIES    - List of libraries when using hdf-eos2.
#   HDFEOS2_FOUND        - True if hdf-eos2 kit found.
#
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

# Look for the header file.
find_path(HDFEOS2_INCLUDE_DIRS NAMES HdfEosDef.h)

# Look for the library.
find_library(HDFEOS2_LIBRARY NAMES hdfeos libhdfeos Gctp libGctp)

if (HDFEOS2_INCLUDE_DIR AND EXISTS "${HDFEOS2_INCLUDE_DIR}/HDFEOS2.h")
    file(STRINGS "${HDFEOS2_INCLUDE_DIR}/HDFEOS2.h" HDFEOS2_version_str
         REGEX "^#[\t ]*define[\t ]+JBG_VERSION_(MAJOR|MINOR)[\t ]+[0-9]+$")

    unset(HDFEOS2_VERSION_STRING)
    foreach(VPART MAJOR MINOR)
        foreach(VLINE ${HDFEOS2_version_str})
            if(VLINE MATCHES "^#[\t ]*define[\t ]+JBG_VERSION_${VPART}[\t ]+([0-9]+)$")
                set(HDFEOS2_VERSION_PART "${CMAKE_MATCH_1}")
                if(HDFEOS2_VERSION_STRING)
                    set(HDFEOS2_VERSION_STRING "${HDFEOS2_VERSION_STRING}.${HDFEOS2_VERSION_PART}")
                else()
                    set(HDFEOS2_VERSION_STRING "${HDFEOS2_VERSION_PART}")
                endif()
            endif()
        endforeach()
    endforeach()
endif ()

# handle the QUIETLY and REQUIRED arguments and set HDFEOS2_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HDFEOS2
                                  REQUIRED_VARS HDFEOS2_LIBRARY HDFEOS2_INCLUDE_DIR
                                  VERSION_VAR HDFEOS2_VERSION_STRING)

# Copy the results to the output variables.
if(HDFEOS2_FOUND)
  set(HDFEOS2_LIBRARIES ${HDFEOS2_LIBRARY})
  set(HDFEOS2_INCLUDE_DIRS ${HDFEOS2_INCLUDE_DIR})
endif()

mark_as_advanced(HDFEOS2_INCLUDE_DIR HDFEOS2_LIBRARY)
