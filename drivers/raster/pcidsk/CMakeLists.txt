################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, dmitry.baryshnikov@nextgis.com
################################################################################
# Copyright (C) 2018, NextGIS <info@nextgis.com>
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

option(ENABLE_PCIDSK "Build PCI Geomatics Database File Format" ON)
if(ENABLE_PCIDSK AND JPEG_FOUND)
    set(TARGET_NAME ras_pcidsk)

    add_definitions(-DFRMT_pcidsk)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_pcidsk" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sdk)

    add_definitions(-DHAVE_LIBJPEG -DPCIDSK_INTERNAL)

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
    )

    set(SDK_CSOURCES
        sdk/channel/cbandinterleavedchannel.cpp
        sdk/channel/cpcidskchannel.cpp
        sdk/channel/cpixelinterleavedchannel.cpp
        sdk/channel/ctiledchannel.cpp
        sdk/channel/cexternalchannel.cpp
        sdk/core/cpcidskfile.cpp
        sdk/core/libjpeg_io.cpp
        sdk/core/edb_pcidsk.cpp
        sdk/core/metadataset_p.cpp
        sdk/core/pcidskbuffer.cpp
        sdk/core/pcidskcreate.cpp
        sdk/core/pcidskexception.cpp
        sdk/core/pcidskinterfaces.cpp
        sdk/core/pcidskopen.cpp
        sdk/core/pcidsk_pubutils.cpp
        sdk/core/pcidsk_utils.cpp
        sdk/core/cpcidskblockfile.cpp
        sdk/core/pcidsk_raster.cpp
        sdk/core/pcidsk_scanint.cpp
        sdk/segment/cpcidskgeoref.cpp
        sdk/segment/cpcidsksegment.cpp
        sdk/segment/cpcidskvectorsegment.cpp
        sdk/segment/cpcidskvectorsegment_consistencycheck.cpp
        sdk/segment/vecsegheader.cpp
        sdk/segment/vecsegdataindex.cpp
        sdk/segment/metadatasegment_p.cpp
        sdk/segment/cpcidskpct.cpp
        sdk/segment/cpcidskrpcmodel.cpp
        sdk/segment/cpcidskgcp2segment.cpp
        sdk/segment/cpcidskbitmap.cpp
        sdk/segment/cpcidsk_tex.cpp
        sdk/segment/cpcidskblut.cpp
        sdk/segment/cpcidskbpct.cpp
        sdk/segment/cpcidsklut.cpp
        sdk/segment/cpcidskpolymodel.cpp
        sdk/segment/systiledir.cpp
        sdk/core/clinksegment.cpp
        sdk/segment/cpcidsktoutinmodel.cpp
        sdk/segment/cpcidskbinarysegment.cpp
        sdk/segment/cpcidsk_array.cpp
        sdk/segment/cpcidskephemerissegment.cpp
        sdk/blockdir/asciitiledir.cpp
        sdk/blockdir/asciitilelayer.cpp
        sdk/blockdir/binarytiledir.cpp
        sdk/blockdir/binarytilelayer.cpp
        sdk/blockdir/blockdir.cpp
        sdk/blockdir/blockfile.cpp
        sdk/blockdir/blocklayer.cpp
        sdk/blockdir/blocktiledir.cpp
        sdk/blockdir/blocktilelayer.cpp
    )

    set(LIB_CSOURCES ${SDK_CSOURCES}
        pcidskdataset2.cpp ogrpcidsklayer.cpp vsi_pcidsk_io.cpp gdal_edb.cpp
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip pcidsk")
endif()
