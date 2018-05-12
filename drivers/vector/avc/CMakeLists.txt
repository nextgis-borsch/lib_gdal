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

option(ENABLE_AVC "Build Arc/Info Binary Coverage & Arc/Info .E00 (ASCII) Coverage formats" ON)
if(ENABLE_AVC)
    set(TARGET_NAME vec_avc)

    add_definitions(-DAVCBIN_ENABLED)
	set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DAVCBIN_ENABLED" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/drivers/vector/shape)

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        ogr_avc.h
        avc.h
    )

    set(LIB_OGRSOURCES
        ogravcbindriver.cpp
        ogravcbindatasource.cpp
        ogravcbinlayer.cpp
		ogravclayer.cpp
        ogravcdatasource.cpp
		ogravce00layer.cpp
        ogravce00datasource.cpp
        ogravce00driver.cpp
    )

    set(LIB_AVCSOURCES
        avc_bin.cpp
        avc_binwr.cpp
        avc_e00gen.cpp
        avc_e00parse.cpp
		avc_e00write.cpp
        avc_e00read.cpp
        avc_mbyte.cpp
        avc_misc.cpp
		avc_rawbin.cpp 
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_AVCSOURCES} ${LIB_OGRSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip avc")
endif()
