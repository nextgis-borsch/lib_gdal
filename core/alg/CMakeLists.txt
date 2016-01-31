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

set(TARGET_NAME alg)

include_directories(${CMAKE_SOURCE_DIR}/core/port)
include_directories(${CMAKE_SOURCE_DIR}/core/gcore)
include_directories(${CMAKE_SOURCE_DIR}/core/ogr)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LIB_HHEADERS
    gdal_alg.h
    gdalgrid.h
    gdalpansharpen.h
    gdal_simplesurf.h
    gdalwarper.h
    gvgcpfit.h
    thinplatespline.h
)

set(LIB_HHEADERS_PRIV
    gdal_alg_priv.h    
    gdalgrid_priv.h
    internal_qhull_headers.h
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

find_anyproject(QHULL DEFAULT OFF)

find_anyproject(Armadillo DEFAULT OFF)
if(ARMADILLO_FOUND)
    add_definitions(-DHAVE_ARMADILLO)
endif()

find_anyproject(OpenCL DEFAULT OFF)
if(OPENCL_FOUND)
    add_definitions(-DHAVE_OPENCL)

    set(LIB_HHEADERS_PRIV ${LIB_HHEADERS_PRIV}
        gdalwarpkernel_opencl.h
    )
    set(LIB_CSOURCES ${LIB_CSOURCES}
        gdalwarpkernel_opencl.c
    )
endif()

add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

# build target
set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
set(TARGET_LINK_LIB ${TARGET_LINK_LIB} PARENT_SCOPE)

# install headers
if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${LIB_HHEADERS} DESTINATION ${INSTALL_INC_DIR})
endif() 
