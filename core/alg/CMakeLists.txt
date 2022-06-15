################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015-2019, NextGIS <info@nextgis.com>
# Copyright (C) 2012-2014,2015-2019 Dmitry Baryshnikov
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

set(TARGET_NAME alg)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/marching_squares)
include_directories(${CMAKE_SOURCE_DIR}/core/ogr/ogrsf_frmts)
include_directories(${CMAKE_SOURCE_DIR}/core/frmts/mem)
include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/vrt)

set(LIB_HHEADERS
    gdal_alg.h
    gdal_alg_priv.h
    gdalgrid.h
    gdalpansharpen.h
    gdal_simplesurf.h
    gdalwarper.h
)

set(LIB_HHEADERS_PRIV
    gdalgrid_priv.h
    internal_qhull_headers.h
    gdallinearsystem.h
    gvgcpfit.h
    thinplatespline.h
    armadillo_headers.h
    gdallinearsystem.h
    gdalgeoloc.h
    gdalgeoloc_carray_accessor.h
    gdalgeoloc_dataset_accessor.h
    gdalgeolocquadtree.h
)

set(LIB_CSOURCES
    gdalchecksum.cpp
    gdal_crs.cpp
    gdalcutline.cpp
    gdalmediancut.cpp
#    gdal_nrgcrs.c
    gdalproximity.cpp
    thinplatespline.cpp
    gdalpansharpen.cpp
    gdal_tps.cpp
    gdaltransformer.cpp
    gdaltransformgeolocs.cpp
    gdalsimplewarp.cpp
    gdalwarper.cpp
    gdalwarpkernel.cpp
    gdalwarpoperation.cpp
    gdalrasterize.cpp
    contour.cpp
    delaunay.c
    gdaldither.cpp
    gdalgeoloc.cpp
    gdalgridavx.cpp
    gdalgrid.cpp
    gdalgridsse.cpp
    gdalmatching.cpp
    gdal_octave.cpp
    gdalrasterpolygonenumerator.cpp
    gdal_rpc.cpp
    gdalsievefilter.cpp
    gdal_simplesurf.cpp
    llrasterize.cpp
    polygonize.cpp
    rasterfill.cpp
    gdalapplyverticalshiftgrid.cpp
    gdallinearsystem.cpp
    viewshed.cpp
    gdalgeolocquadtree.cpp
)

find_anyproject(QHULL DEFAULT OFF)
if(QHULL_FOUND)
    add_definitions(-DEXTERNAL_QHULL -DQHULL_INCLUDE_SUBDIR_IS_LIBQHULL=1 -DHAVE_QHULL_LIBQHULL_H=1)
    if(WIN32)
        add_definitions(-Dqh_dllimport)
    endif()
endif()

find_anyproject(Armadillo DEFAULT OFF)
if(ARMADILLO_FOUND)
    add_definitions(-DHAVE_ARMADILLO)
endif()

if(APPLE)
    set(OPENCL_LIBRARY "-framework OpenCL" CACHE STRING "OpenCL library")
    set(OPENCL_INCLUDE_DIRS "" CACHE PATH "OpenCL include directory")
    set(TARGET_LINK_LIB ${TARGET_LINK_LIB} ${OPENCL_LIBRARY})
    set(OPENCL_FOUND TRUE CACHE BOOL "OpenCL found")
else()
    find_anyproject(OpenCL DEFAULT OFF)
    # From opencv
    # set(OPENCL_LIBRARY "" CACHE STRING "OpenCL library")
    # set(OPENCL_INCLUDE_DIR "${OpenCV_SOURCE_DIR}/3rdparty/include/opencl/1.2" CACHE PATH "OpenCL include directory")
    # ocv_install_3rdparty_licenses(opencl-headers "${OpenCV_SOURCE_DIR}/3rdparty/include/opencl/LICENSE.txt")
endif()
if(OPENCL_FOUND)
    add_definitions(-DHAVE_OPENCL)

    set(LIB_HHEADERS_PRIV ${LIB_HHEADERS_PRIV}
        gdalwarpkernel_opencl.h
    )
    set(LIB_CSOURCES ${LIB_CSOURCES}
        gdalwarpkernel_opencl.cpp
    )
endif()

add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

# Build target
set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
set(TARGET_LINK_LIB ${TARGET_LINK_LIB} PARENT_SCOPE)
set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${TARGET_NAME} PARENT_SCOPE)

# Install headers
if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${LIB_HHEADERS} DESTINATION ${INSTALL_INC_DIR} COMPONENT headers)
endif()
