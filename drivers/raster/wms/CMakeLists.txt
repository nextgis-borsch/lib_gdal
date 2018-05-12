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

CMAKE_DEPENDENT_OPTION(ENABLE_WMS "Build OGC Web Map Service, and TMS, WorldWind, On Earth tiled, VirtualEarth, ArcGIS REST, IIP) format" ON
                       "CURL_FOUND" OFF)
if(ENABLE_WMS)
    set(TARGET_NAME ras_wms)

    add_definitions(-DFRMT_wms)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_wms" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CURL_INCLUDE_DIRS})

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        gdalhttp.h
        minidriver_tileservice.h
        minidriver_wms.h
        minidriver_worldwind.h
        minidriver_tms.h
        minidriver_tiled_wms.h
        wmsdriver.h
        wmsmetadataset.h
        minidriver_virtualearth.h
        minidriver_arcgis_server.h
        minidriver_mrf.h
    )

    set(LIB_CSOURCES
        gdalwmscache.cpp
        gdalwmsdataset.cpp
        gdalwmsrasterband.cpp
        gdalhttp.cpp
        minidriver.cpp
        wmsutils.cpp
        wmsdriver.cpp
        minidriver_wms.cpp
        minidriver_tileservice.cpp
        minidriver_worldwind.cpp
        minidriver_tms.cpp
        minidriver_tiled_wms.cpp
        wmsmetadataset.cpp
        minidriver_virtualearth.cpp
        minidriver_arcgis_server.cpp
        minidriver_iip.cpp
        minidriver_mrf.cpp
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip wms")
endif()
