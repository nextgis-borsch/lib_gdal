################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Mikhail Gusev, gusevmihs@gmail.com
################################################################################
# Copyright (C) 2015-2017, NextGIS <info@nextgis.com>
# Copyright (C) 2016 Mikhail Gusev
# Copyright (C) 2020 Dmitry Baryshnikov <polimax@mail.ru>
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

option(ENABLE_PDS_ISIS2_ISIS3_VICAR "Build NASA Planetary Data System & ISIS2 & ISIS3 & VICAR format" ON)
if(ENABLE_PDS_ISIS2_ISIS3_VICAR)
    set(TARGET_NAME ras_pds)

    add_definitions(-DFRMT_pds)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_pds" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/raw)
    include_directories(${CMAKE_SOURCE_DIR}/drivers/vector/geojson)
    include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/vrt)
    include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/gtiff)

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        vicarkeywordhandler.h
        ${CMAKE_SOURCE_DIR}/core/gcore/rawdataset.h
        pds4dataset.h
    )

    set(LIB_CSOURCES
        pdsdataset.cpp
        isis2dataset.cpp
        isis3dataset.cpp
        vicardataset.cpp
        vicarkeywordhandler.cpp
        pds4dataset.cpp
        pds4vector.cpp
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip pds")
endif()
