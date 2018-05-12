################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Mikhail Gusev, gusevmihs@gmail.com
################################################################################
# Copyright (C) 2015-2017, NextGIS <info@nextgis.com>
# Copyright (C) 2017 Mikhail Gusev
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
################################################################################

option(ENABLE_OPENJPEG "Build OPENJPEG (JPEG2000 via OpenJPEG library) format" OFF)
if(ENABLE_OPENJPEG)
    set(TARGET_NAME ras_openjpeg)

    find_anyproject(OpenJPEG REQUIRED)

    if(NOT OPENJPEG_VERSION_NUM)
        set(MAJOR_VERSION 0)
        set(MINOR_VERSION 0)
        set(REV_VERSION 0)

        file(GLOB_RECURSE OPJ_CONFIGS "${OPENJPEG_INCLUDE_DIRS}/opj_config.h")
        set(OPJ_CONFIG)
        if(OPJ_CONFIGS)
            list(GET OPJ_CONFIGS 0 OPJ_CONFIG)
        endif()
        if(EXISTS ${OPJ_CONFIG})
            file(READ ${OPJ_CONFIG} VERSION_H_CONTENTS)

            string(REGEX MATCH "OPJ_VERSION_MAJOR[ \t]+([0-9]+)"
              MAJOR_VERSION ${VERSION_H_CONTENTS})
            string (REGEX MATCH "([0-9]+)"
              MAJOR_VERSION ${MAJOR_VERSION})
            string(REGEX MATCH "OPJ_VERSION_MINOR[ \t]+([0-9]+)"
              MINOR_VERSION ${VERSION_H_CONTENTS})
            string (REGEX MATCH "([0-9]+)"
              MINOR_VERSION ${MINOR_VERSION})
            string(REGEX MATCH "OPJ_VERSION_BUILD[ \t]+([0-9]+)"
              REV_VERSION ${VERSION_H_CONTENTS})
            string (REGEX MATCH "([0-9]+)"
              REV_VERSION ${REV_VERSION})

            unset(VERSION_H_CONTENTS)
        endif()

        set(OPENJPEG_VERSION_STRING "${MAJOR_VERSION}.${MINOR_VERSION}.${REV_VERSION}")
        math(EXPR OPENJPEG_VERSION_NUM "${MAJOR_VERSION} * 10000 + ${MINOR_VERSION} * 100 + ${REV_VERSION}")

    endif()

    if(OPENJPEG_VERSION_NUM LESS 20000)
        warning_msg("Skip openjpeg")
        # message(STATUS "OPENJPEG_VERSION_NUM set to ${OPENJPEG_VERSION_NUM}")
    else()
        add_definitions(-DFRMT_openjpeg -DOPENJPEG_VERSION=${OPENJPEG_VERSION_NUM})
        set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_openjpeg" PARENT_SCOPE)

        include_directories(${CMAKE_CURRENT_SOURCE_DIR})
        include_directories(${CMAKE_SOURCE_DIR}/drivers/raster) # for /vrt/vrtdataset.h
        include_directories(${OPENJPEG_INCLUDE_DIRS}/openjpeg-${MAJOR_VERSION}.${MINOR_VERSION})

        # TODO: maybe add openjpeg library version to definitions (not required for now).

        set(LIB_HHEADERS
        )

        set(LIB_HHEADERS_PRIV
        )

        set(LIB_CSOURCES
            openjpegdataset.cpp
        )

        add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

        # build target
        set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
        set(TARGET_LINK_LIB ${TARGET_LINK_LIB} PARENT_SCOPE)
        set(DEPENDENCY_LIB ${DEPENDENCY_LIB} PARENT_SCOPE)
    endif()
else()
    warning_msg("Skip openjpeg")
endif()
