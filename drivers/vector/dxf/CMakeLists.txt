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

option(ENABLE_DXF "Build AutoCAD DXF format" ON)
if(ENABLE_DXF)
    set(TARGET_NAME vec_dxf)

    add_definitions(-DDXF_ENABLED)
	set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DDXF_ENABLED" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/core/alg)

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        ogr_dxf.h
    )

    set(LIB_CSOURCES
        ogrdxfdriver.cpp
        ogrdxfdatasource.cpp
        ogrdxflayer.cpp
		ogrdxfreader.cpp
        ogrdxf_blockmap.cpp
        ogrdxf_dimension.cpp
		ogrdxfwriterds.cpp
        ogrdxfwriterlayer.cpp
        intronurbs.cpp
		ogrdxf_polyline_smooth.cpp
        ogrdxfblockslayer.cpp
		ogrdxfblockswriterlayer.cpp
        ogrdxf_hatch.cpp
		ogr_autocad_services.cpp
        ogrdxf_feature.cpp
        ogrdxf_leader.cpp
        ogrdxf_ocstransformer.cpp
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip dxf")
endif()
