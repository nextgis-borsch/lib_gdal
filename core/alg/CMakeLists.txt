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

set(LIB_NAME "alg")
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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gcore)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ogr)

set(LIB_HHEADERS
    gdal_alg.h
    gdal_alg_priv.h
    gdalgrid.h
    gdalgrid_priv.h
    gdalpansharpen.h
    gdal_simplesurf.h
    gdalwarper.h
    gvgcpfit.h
    thinplatespline.h
)

set(LIB_CSOURCES
    contour.cpp
    delaunay.c
    gdalchecksum.cpp
    gdal_crs.c
    gdalcutline.cpp
    gdaldither.cpp
    gdalgeoloc.cpp
    gdalgridavx.cpp
    gdalgrid.cpp
    gdalgridsse.cpp
    gdalmatching.cpp
    gdalmediancut.cpp
#    gdal_nrgcrs.c
    gdal_octave.cpp
    gdalpansharpen.cpp
    gdalproximity.cpp
    gdalrasterize.cpp
    gdalrasterpolygonenumerator.cpp
    gdal_rpc.cpp
    gdalsievefilter.cpp
    gdal_simplesurf.cpp
    gdalsimplewarp.cpp
    gdal_tps.cpp
    gdaltransformer.cpp
    gdaltransformgeolocs.cpp
    gdalwarper.cpp
    gdalwarpkernel.cpp
    gdalwarpoperation.cpp
    llrasterize.cpp
    polygonize.cpp
    rasterfill.cpp
    thinplatespline.cpp
)

option(GDAL_USE_GEOS "Set ON to use libgeos" ON)
if(GDAL_USE_GEOS)
    option(GDAL_USE_GEOS_INTERNAL "Set ON to use internal libgeos" OFF)
    add_definitions(-DHAVE_GEOS=1)
    if(GDAL_USE_GEOS_INTERNAL)
        # todo: get geos from external project
    else()
        find_package(GEOS REQUIRED)
        if(GEOS_FOUND)
            get_filename_component(GEOS_ROOT ${GEOS_INCLUDE_DIR} PATH)
            include_directories(${GEOS_INCLUDE_DIR})
            include_directories(${GEOS_ROOT}/capi)
        endif()
    endif()
endif()

option(GDAL_USE_ARMADILLO "Set ON to use Armadillo" OFF)
if(GDAL_USE_ARMADILLO)
    find_package(Armadillo REQUIRED)
    if(ARMADILLO_FOUND)
        include_directories(${ARMADILLO_INCLUDE_DIRS})
        add_definitions(-DHAVE_ARMADILLO)
    endif()
endif()


option(GDAL_USE_OPENCL "Set ON to use OpenCL" OFF)
if(GDAL_USE_OPENCL)
    find_package(OpenCL REQUIRED)
    if(OPENCL_FOUND)
        include_directories(${OPENCL_INCLUDE_DIRS})
        add_definitions(-DHAVE_OPENCL)

        set(LIB_HHEADERS ${LIB_HHEADERS}
            gdalwarpkernel_opencl.h
        )

        set(LIB_CSOURCES ${LIB_CSOURCES}
            gdalwarpkernel_opencl.c
        )
    endif()
endif()


add_library(${LIB_NAME} OBJECT ${LIB_HHEADERS} ${LIB_CSOURCES})

if(GEOS_FOUND)
    set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} ${GEOS_LIBRARY})
endif(GEOS_FOUND)

if(OPENCL_FOUND)
    set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} ${OPENCL_LIBRARIES})
endif(OPENCL_FOUND)

if(ARMADILLO_FOUND)
    set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} ${ARMADILLO_LIBRARIES})
endif(ARMADILLO_FOUND)

# build target
set(GDAL_TARGET_OBJECTS ${GDAL_TARGET_OBJECTS} $<TARGET_OBJECTS:${LIB_NAME}> PARENT_SCOPE)
set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} PARENT_SCOPE)

# install headers
set(GDAL_INSTALL_HEADERS ${GDAL_INSTALL_HEADERS} ${LIB_HHEADERS} PARENT_SCOPE)
