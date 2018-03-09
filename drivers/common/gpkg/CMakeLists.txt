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

if(ENABLE_SQLITE_GPKG)
    set(TARGET_NAME com_gpkg)

    if(DEFINED SQLITE_HAS_COLUMN_METADATA)
        add_definitions(-DSQLITE_HAS_COLUMN_METADATA)
    endif()

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/drivers/vector/sqlite)
    include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/mem)
    include_directories(${SQLITE3_INCLUDE_DIRS})

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        ogr_geopackage.h
        ogrgeopackageutility.h
        ${CMAKE_SOURCE_DIR}/drivers/vector/sqlite/ogr_sqlite.h
    )

    set(LIB_CSOURCES
        ogrgeopackagedriver.cpp
        ogrgeopackagedatasource.cpp
        ogrgeopackagelayer.cpp
        ogrgeopackagetablelayer.cpp
        ogrgeopackageselectlayer.cpp
        ogrgeopackageutility.cpp
        gdalgeopackagerasterband.cpp
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip geopackage")
endif()
