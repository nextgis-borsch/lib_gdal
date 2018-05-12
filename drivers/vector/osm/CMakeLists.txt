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

CMAKE_DEPENDENT_OPTION(ENABLE_OSM "Build OpenStreetMap XML and PBF format" ON
                       "SQLITE3_FOUND" OFF)
if(ENABLE_OSM)
    set(TARGET_NAME vec_osm)

    add_definitions(-DOSM_ENABLED)
	set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DOSM_ENABLED" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/drivers/vector/sqlite)
    include_directories(${CMAKE_SOURCE_DIR}/drivers/vector/generic)
    include_directories(${SQLITE3_INCLUDE_DIRS})

    if(EXPAT_FOUND)
        include_directories(${EXPAT_INCLUDE_DIRS})
    endif()

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        ogr_osm.h
        gpb.h
        osm_parser.h
    )

    set(LIB_CSOURCES
        ogrosmdriver.cpp
        ogrosmdatasource.cpp
        ogrosmlayer.cpp
        osm_parser.cpp
        osm2osm.cpp
    )

    # TODO: implement osm2osm target if needed.

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip osm")
endif()
