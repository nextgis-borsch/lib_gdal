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

option(ENABLE_GRIB "Build WMO GRIB1/GRIB2 (.grb) format" ON)
if(ENABLE_GRIB)
    set(TARGET_NAME ras_grib)

    add_definitions(-DFRMT_grib)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_grib" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/degrib18/degrib)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/degrib18/g2clib-1.0.4)
    
    # TODO: implement JASPER support when it will be available as an external library.
    #if(JASPER_FOUND) 
    #    add_definitions(-DHAVE_JASPER)
    #endif()
    
    if(PNG_FOUND) 
        add_definitions(-DUSE_PNG)
    endif()

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
    )
    
    set(LIB_CSOURCES
        gribdataset.cpp
    )
    
    # TODO: implement plugin gdal_ERS.so/.dll target if needed.

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    
    file(GLOB CSOURCES ${CMAKE_CURRENT_SOURCE_DIR}/degrib18/degrib/*)
    foreach(CSOURCE ${CSOURCES})
        if(EXISTS ${CSOURCE})
            get_filename_component(CSOURCE_EXT ${CSOURCE} EXT) 
            if((${CSOURCE_EXT} STREQUAL ".c") OR (${CSOURCE_EXT} STREQUAL ".cpp"))
                set(LIB_CSOURCES_DEGRIB ${LIB_CSOURCES_DEGRIB} ${CSOURCE})
            endif() 
        endif()    
    endforeach()
    add_library(${TARGET_NAME}_degrib OBJECT ${LIB_CSOURCES_DEGRIB})
    
    
    file(GLOB CSOURCES ${CMAKE_CURRENT_SOURCE_DIR}/degrib18/g2clib-1.0.4/*)
    foreach(CSOURCE ${CSOURCES})
        if(EXISTS ${CSOURCE})
            get_filename_component(CSOURCE_EXT ${CSOURCE} EXT)  
            if((CSOURCE_EXT STREQUAL ".c") OR (CSOURCE_EXT STREQUAL ".cpp"))
                set(LIB_CSOURCES_G2CLIB ${LIB_CSOURCES_G2CLIB} ${CSOURCE})
            endif() 
        endif()    
    endforeach()    
    add_library(${TARGET_NAME}_g2clib OBJECT ${LIB_CSOURCES_G2CLIB})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} 
        $<TARGET_OBJECTS:${TARGET_NAME}>
        $<TARGET_OBJECTS:${TARGET_NAME}_degrib>
        $<TARGET_OBJECTS:${TARGET_NAME}_g2clib>
        PARENT_SCOPE)
else()
    warning_msg("Skip grib")  
endif()
