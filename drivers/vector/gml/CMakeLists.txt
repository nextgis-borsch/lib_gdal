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

option(ENABLE_GML "Build GML format" ON)
if(ENABLE_GML)
    set(TARGET_NAME vec_gml)

    add_definitions(-DGML_ENABLED)
	set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DGML_ENABLED" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    
    if(EXPAT_FOUND)
        include_directories(${EXPAT_INCLUDE_DIRS})
    endif()
    
    # TODO: implement XERCES support here or in upper CMake files.
    # find_anyproject(XERCES)
    #if(XERCES_FOUND)
    #    add_definitions(...)
    #    include_directories(...)
    #endif()
    
    if(SQLITE3_FOUND)
        include_directories(${SQLITE3_INCLUDE_DIRS})
        add_definitions(-DHAVE_SQLITE) # option with this name is used only in GML driver (among all sources)
    endif()
    
    option(OGR_GML_VALIDATION OFF)
    if(OGR_GML_VALIDATION)
        add_definitions(-DOGR_GML_VALIDATION=1)
    endif()

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        ogr_gml.h 
        gmlreader.h 
        gmlreaderp.h
    )

    set(LIB_CORESOURCES
        gmlpropertydefn.cpp 
        gmlfeatureclass.cpp 
        gmlfeature.cpp 
        gmlreader.cpp 
		parsexsd.cpp 
        resolvexlinks.cpp 
        hugefileresolver.cpp 
        gmlutils.cpp 
		gmlreadstate.cpp 
        gmlhandler.cpp 
        trstring.cpp 
        gfstemplate.cpp 
        gmlregistry.cpp
    )
    
    set(LIB_OGRSOURCES
        ogrgmldriver.cpp 
        ogrgmldatasource.cpp 
        ogrgmllayer.cpp
    )
    
    # TODO: implement gmlview target if needed.

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CORESOURCES} ${LIB_OGRSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip gml")  
endif()
