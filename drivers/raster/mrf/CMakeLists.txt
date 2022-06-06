################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, dmitry.baryshnikov@nextgis.com
################################################################################
# Copyright (C) 2015-2020, NextGIS <info@nextgis.com>
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

option(ENABLE_MRF "Build Meta Raster Format" ON)
if(ENABLE_MRF AND JPEG_FOUND AND PNG_FOUND AND TIFF_FOUND)
    set(TARGET_NAME ras_mrf)

    add_definitions(-DFRMT_mrf)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_mrf" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    if(WITH_LERC)
        include_directories(${CMAKE_SOURCE_DIR}/core/lib/LercLib)
        add_definitions(-DLERC)

        set(LIB_LERC_HHEADERS
            LERCV1/Lerc1Image.h
        )

        set(LIB_LERC_CSOURCES
            LERC_band.cpp
            LERCV1/Lerc1Image.cpp
        )
    endif()

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV ${LIB_LERC_HHEADERS}
        Packer.h
        Packer_RLE.h
    )

    set(LIB_CSOURCES ${LIB_LERC_CSOURCES}
        Packer_RLE.cpp
        Tif_band.cpp
        PNG_band.cpp
        JPEG_band.cpp
        JPNG_band.cpp
        mrf_band.cpp
        mrf_overview.cpp
        mrf_util.cpp
        marfa_dataset.cpp
    )

    if(JPEG12_FOUND)
        find_file(LIBJPEG_12_PATH NAMES jpeglib.h PATHS ${JPEG12_INCLUDE_DIRS})
        add_definitions(-DJPEG12_SUPPORTED -DLIBJPEG_12_H="${LIBJPEG_12_PATH}")
        set(LIB_CSOURCES ${LIB_CSOURCES}
            JPEG12_band.cpp
        )
    endif()

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip mrf")
endif()
