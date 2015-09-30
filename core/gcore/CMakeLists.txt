################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015, NextGIS <info@nextgis.com>
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

cmake_minimum_required (VERSION 2.8.10 FATAL_ERROR)

set(LIB_NAME "gcore")
project ("lib${LIB_NAME}")

option(GDAL_USE_CPL "Set ON to use cpl" ON)
if(GDAL_USE_CPL)
    add_definitions(-DUSE_CPL)
endif(GDAL_USE_CPL)

option(GDAL_PAM_ENABLED "Set ON to enable pam" ON)
if(GDAL_PAM_ENABLED)
    add_definitions(-DPAM_ENABLED)
endif(GDAL_PAM_ENABLED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../port)
include_directories(${GDAL_ROOT_BINARY_DIR}/port)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../alg)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ogr)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ogr/ogrsf_frmts)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ogr/ogrsf_frmts/generic)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ogr/ogrsf_frmts/geojson)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../frmts/gtiff)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../frmts/vrt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../frmts/mem)

set(LIB_HHEADERS
    gdalexif.h
# todo: more elegant format's register  gdal_frmts.h
    gdalgeorefpamdataset.h
    gdal.h
    gdaljp2abstractdataset.h
    gdaljp2metadatagenerator.h
    gdaljp2metadata.h
    gdal_mdreader.h
    gdal_priv.h
    gdal_proxy.h
    gdal_rat.h
    gdalsse_priv.h
    gdal_version.h
)

set(LIB_CSOURCES
    gdalmajorobject.cpp
    gdalabstractbandblockcache.cpp
    gdalallvalidmaskband.cpp
    gdalarraybandblockcache.cpp
    gdalclientserver.cpp
    gdalcolortable.cpp
    gdaldataset.cpp
    gdaldefaultasync.cpp
    gdaldefaultoverviews.cpp
    gdaldllmain.cpp
    gdaldriver.cpp
    gdaldrivermanager.cpp
    gdalexif.cpp
    gdalgeorefpamdataset.cpp
    gdalgmlcoverage.cpp
    gdalhashsetbandblockcache.cpp
    gdaljp2abstractdataset.cpp
    gdaljp2box.cpp
    gdaljp2metadata.cpp
    gdaljp2metadatagenerator.cpp
    gdaljp2structure.cpp
    gdalmajorobject.cpp
    gdal_mdreader.cpp
    gdal_misc.cpp
    gdalmultidomainmetadata.cpp
    gdalnodatamaskband.cpp
    gdalnodatavaluesmaskband.cpp
    gdalopeninfo.cpp
    gdaloverviewdataset.cpp
    gdalproxydataset.cpp
    gdalproxypool.cpp
    gdalrasterband.cpp
    gdalrasterblock.cpp
    gdal_rat.cpp
    gdalrescaledalphaband.cpp
    gdalvirtualmem.cpp
    jp2dump.cpp
    overview.cpp
    rasterio.cpp
)

if(GDAL_PAM_ENABLED)
    set(LIB_HHEADERS ${LIB_HHEADERS}
        gdal_pam.h
    )
    set(LIB_CSOURCES ${LIB_CSOURCES}
        gdalpamdataset.cpp
        gdalpamrasterband.cpp
        gdalpamproxydb.cpp
    )
endif(GDAL_PAM_ENABLED)

option(GDAL_USE_JSONC "Set ON to use libjson-c" ON)
if(GDAL_USE_JSONC)
    option(GDAL_USE_JSONC_INTERNAL "Set ON to use internal libjson-c" ON)
    if(GDAL_USE_JSONC_INTERNAL)
        # todo: get json-c from external project
        set(JSONC_FOUND ON)
    else()
        find_package(JSONC REQUIRED)
        if(JSONC_FOUND)
            include_directories(${JSONC_INCLUDE_DIR})
        endif()
    endif()
else()
    message(FATAL_ERROR "libjson-c is required")
endif()

add_subdirectory(mdreader)

add_library(${LIB_NAME} OBJECT ${LIB_HHEADERS} ${LIB_CSOURCES})

if(JSONC_FOUND)
    set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} ${JSONC_LIBRARY})
endif(JSONC_FOUND)

# build target
set(GDAL_TARGET_OBJECTS ${GDAL_TARGET_OBJECTS} $<TARGET_OBJECTS:${LIB_NAME}> PARENT_SCOPE)
if(WIN32)
    set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} Ws2_32.lib PARENT_SCOPE)
else(WIN32)
    set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} PARENT_SCOPE)
endif(WIN32)

# install headers
set(GDAL_INSTALL_HEADERS ${GDAL_INSTALL_HEADERS} ${LIB_HHEADERS} PARENT_SCOPE)
