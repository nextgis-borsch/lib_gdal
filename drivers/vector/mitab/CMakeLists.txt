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

set(TARGET_NAME vec_mitab)

add_definitions(-DTAB_ENABLED)
set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DTAB_ENABLED" PARENT_SCOPE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LIB_HHEADERS
)

set(LIB_HHEADERS_PRIV
    mitab_utils.h
    mitab_geometry.h
    mitab_ogr_driver.h
    mitab.h
    mitab_priv.h
)

set(LIB_CSOURCES
    mitab_feature_mif.cpp
    mitab_imapinfofile.cpp    
    mitab_mapindexblock.cpp   
    mitab_ogr_datasource.cpp  
    mitab_spatialref.cpp   
    mitab_utils.cpp
    mitab_bounds.cpp    
    mitab_geometry.cpp     
    mitab_indfile.cpp         
    mitab_mapobjectblock.cpp  
    mitab_ogr_driver.cpp      
    mitab_tabfile.cpp
    mitab_coordsys.cpp
    mitab_mapcoordblock.cpp   
    mitab_maptoolblock.cpp
    mitab_tabseamless.cpp
    mitab_datfile.cpp
    mitab_mapfile.cpp         
    mitab_middatafile.cpp   
    mitab_tabview.cpp
    mitab_feature.cpp   
    mitab_idfile.cpp       
    mitab_mapheaderblock.cpp  
    mitab_miffile.cpp         
    mitab_rawbinblock.cpp     
    mitab_tooldef.cpp
)

add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

# build target
set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)

 
