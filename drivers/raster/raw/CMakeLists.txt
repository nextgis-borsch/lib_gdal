################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015-2017,2019 NextGIS <info@nextgis.com>
# Copyright (C) 2012-2019 Dmitry Baryshnikov
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

option(ENABLE_RAW "Build raw formats" ON)
if(ENABLE_RAW)

    set(TARGET_NAME ras_raw)

    add_definitions(-DFRMT_raw)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_raw" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        atlsci_spheroid.h
        ehdrdataset.h
        envidataset.h
        byndataset.h
    )

    set(LIB_CSOURCES
        ace2dataset.cpp
        doq1dataset.cpp
        envidataset.cpp
        gscdataset.cpp
        iscedataset.cpp
        loslasdataset.cpp
        pauxdataset.cpp
        roipacdataset.cpp
        atlsci_spheroid.cpp
        cpgdataset.cpp
        doq2dataset.cpp
        fastdataset.cpp
        gtxdataset.cpp
        krodataset.cpp
        mffdataset.cpp
        pnmdataset.cpp
        snodasdataset.cpp
        ctable2dataset.cpp
        ehdrdataset.cpp
        hkvdataset.cpp
        landataset.cpp
        ndfdataset.cpp
        btdataset.cpp
        dipxdataset.cpp
        eirdataset.cpp
        genbindataset.cpp
        lcpdataset.cpp
        ntv2dataset.cpp
        rrasterdataset.cpp
        byndataset.cpp
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)

    # install headers
    if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
        install(FILES ${LIB_HHEADERS} DESTINATION ${INSTALL_INC_DIR} COMPONENT headers)
    endif()
else()
    warning_msg("Skip raw")
endif()