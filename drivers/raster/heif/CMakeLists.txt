################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov <dmitry.baryshnikov@nextgis.com>
################################################################################
# Copyright (C) 2019, NextGIS <info@nextgis.com>
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

option(ENABLE_HEIF "Build Airbus DS Intelligence format" OFF)
if(ENABLE_HEIF)
    set(TARGET_NAME ras_heif)

    add_definitions(-DFRMT_heif)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_heif" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
    )

    set(LIB_CSOURCES
        heifdataset.cpp
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})
    target_link_libraries(${TARGET_NAME} PRIVATE HEIF::HEIF)

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip heif")  
endif()
