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

option(GDAL_ENABLE_GNM "Build GNM module" OFF)
if(GDAL_ENABLE_GNM)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    
    set(TARGET_NAME gnm)

    set(LIB_HHEADERS
        gnm_api.h
        gnmgraph.h
        gnm.h
        gnm_priv.h
    )

    set(LIB_CSOURCES
        gnmgenericnetwork.cpp
        gnmgraph.cpp
        gnmlayer.cpp
        gnmnetwork.cpp
        gnmresultlayer.cpp
        gnmrule.cpp
    )


    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
    set(TARGET_LINK_LIB ${TARGET_LINK_LIB} PARENT_SCOPE)

    # install headers
    if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
        install(FILES ${LIB_HHEADERS} DESTINATION ${INSTALL_INC_DIR})
    endif() 
endif()
