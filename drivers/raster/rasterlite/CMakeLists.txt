################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Mikhail Gusev, gusevmihs@gmail.com
################################################################################
# Copyright (C) 2015-2016, NextGIS <info@nextgis.com>
# Copyright (C) 2016 Mikhail Gusev
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

CMAKE_DEPENDENT_OPTION(ENABLE_RASTERLITE "Build Rasterlite (Rasters in SQLite DB) format" ON
                       "ENABLE_SQLITE_GPKG" OFF)
if(ENABLE_RASTERLITE)
    set(TARGET_NAME ras_rasterlite)

    add_definitions(-DFRMT_rasterlite)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_rasterlite" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/core/ogr)
    include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/mem)

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
    )

    set(LIB_CSOURCES
        rasterlitedataset.cpp
        rasterlitecreatecopy.cpp
        rasterliteoverviews.cpp
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip rasterlite")
endif()
