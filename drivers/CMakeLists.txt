################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015-2016, NextGIS <info@nextgis.com>
# Copyright (C) 2012,2013,2014 Dmitry Baryshnikov
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

set(FORMAT_DEFINITIONS)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vector)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vector/mem)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vector/vrt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vector/mitab)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vector/xplane)

find_anyproject(PostgreSQL NAMES PQ pq libpq) # find libpq

# TODO: do we need optional spatialite library support as its written in
# http://www.gdal.org/frmt_rasterlite.html
find_anyproject(Spatialite DEFAULT ON)
if(SPATIALITE_FOUND)
    add_definitions(-DHAVE_SPATIALITE)

    if(SPATIALITE_VERSION VERSION_GREATER "4.1.1")
        add_definitions(-DSPATIALITE_412_OR_LATER)
    endif()
endif()

add_subdirectory(raster)
add_subdirectory(vector)
add_subdirectory(common)
if(GDAL_ENABLE_GNM)
    add_subdirectory(network)
endif()

add_definitions(${FORMAT_DEFINITIONS})

set(TARGET_NAME drivers_registar)

set(LIB_HHEADERS
    ogrsf_frmts.h
)

set(LIB_HHEADERS_PRIV
    gnm_frmts.h
    ogr_attrind.h
    ogrwarpedlayer.h
    ogrlayerdecorator.h
    ogremulatedtransaction.h
    ogrmutexeddatasource.h
    ogreditablelayer.h
    ogr_gensql.h
    ogrlayerpool.h
    ogrmutexedlayer.h
    ogrunionlayer.h
)

set(LIB_CSOURCES
    gdalallregister.cpp
    gnmregisterall.cpp
    ogremulatedtransaction.cpp
    ogrlayerdecorator.cpp
    ogrmutexeddatasource.cpp
    ogrsfdriver.cpp
    ogr_attrind.cpp
    ogrsfdriverregistrar.cpp
    ogrdatasource.cpp
    ogr_gensql.cpp
    ogrlayerpool.cpp
    ogrmutexedlayer.cpp
    ogrunionlayer.cpp
    ogreditablelayer.cpp
    ogrlayer.cpp
    ogr_miattrind.cpp
    ogrregisterall.cpp
    ogrwarpedlayer.cpp
)

add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

# Send variables upper
set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
any_project_var_to_parent_scope()

# Install headers
if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${LIB_HHEADERS} DESTINATION ${INSTALL_INC_DIR} COMPONENT headers)
endif()
