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

CMAKE_DEPENDENT_OPTION(ENABLE_NITF_RPFTOC_ECRGTOC "Build NITF (.ntf, .nsf, ...) & GRaster Product Format/RPF (CADRG, CIB) & ECRG Table Of Contents (TOC.xml) formats" ON
    "ENABLE_JPEG" OFF)
if(ENABLE_NITF_RPFTOC_ECRGTOC)
    set(TARGET_NAME ras_nitf)

    add_definitions(-DFRMT_nitf)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_nitf" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/vrt)
    include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/gtiff)

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        ${CMAKE_SOURCE_DIR}/core/gcore/gdal_proxy.h
        ${CMAKE_SOURCE_DIR}/drivers/raster/vrt/vrtdataset.h
        nitflib.h
        #nitfwritejpeg.cpp
    )

    set(LIB_GDALSOURCES
        nitfdataset.cpp
        rpftocdataset.cpp
        nitfwritejpeg.cpp
		nitfwritejpeg_12.cpp
        nitfrasterband.cpp
        ecrgtocdataset.cpp
    )

    set(LIB_NITFSOURCES
        nitffile.c
        nitfimage.c
        mgrs.c
        nitfaridpcm.cpp
		nitfbilevel.cpp
        rpftocfile.cpp
        nitfdes.c
        nitf_gcprpc.cpp
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_GDALSOURCES} ${LIB_NITFSOURCES})

    # TODO: implement nitfdump target if needed.

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip nitf")
endif()
