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

option(ENABLE_HDF4 "Build Hierarchical Data Format Release 4 (HDF4) format" OFF)
if(ENABLE_HDF4)
    set(TARGET_NAME ras_hdf4)

    add_definitions(-DFRMT_hdf4)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_hdf4" PARENT_SCOPE)

    find_anyproject(HDF4 REQUIRED)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hdf-eos)
    include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/pds)

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        hdf4compat.h
        hdf4dataset.h
        hdf-eos/ease.h
        hdf-eos/HdfEosDef.h
        hdf-eos/HDFEOSVersion.h
    )

    set(LIB_HDFEOS_CSOURCES
        hdf-eos/EHapi.c
        hdf-eos/GDapi.c
        hdf-eos/SWapi.c
        hdf-eos/gctp_wrap.c
    )

    set(LIB_CSOURCES
        hdf4dataset.cpp
        hdf4imagedataset.cpp
    )

    # TODO: implement hdf-eos as separate target if needed.
    # TODO: implement plugin target if needed.

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_HDFEOS_CSOURCES} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
    set(TARGET_LINK_LIB ${TARGET_LINK_LIB} PARENT_SCOPE)
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} PARENT_SCOPE)
else()
    warning_msg("Skip hdf4")
endif()
