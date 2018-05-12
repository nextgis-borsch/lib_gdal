################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, dmitry.baryshnikov@nextgis.com
################################################################################
# Copyright (C) 2015-2017, NextGIS <info@nextgis.com>
# Copyright (C) 2016 Dmitry Baryshnikov
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

option(ENABLE_CAD "Build CAD (.dwg) format" ON)
if(ENABLE_CAD)
    set(TARGET_NAME com_cad)

    add_definitions(-DCAD_ENABLED)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DCAD_ENABLED" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    find_anyproject(OpenCAD REQUIRED)

    set(LIB_HHEADERS
        ogr_cad.h
        vsilfileio.h
    )

    set(LIB_CSOURCES
        gdalcaddataset.cpp
        ogrcaddriver.cpp
        ogrcadlayer.cpp
        vsilfileio.cpp
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
    set(TARGET_LINK_LIB ${TARGET_LINK_LIB} PARENT_SCOPE)
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} PARENT_SCOPE)
else()
    warning_msg("Skip cad")
endif()
