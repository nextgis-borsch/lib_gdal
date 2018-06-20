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

find_anyproject(TIFF REQUIRED CMAKE_ARGS
    -DSKIP_BUILD_DOCS=ON
    -DSKIP_TOOLS=ON
)
if(WITH_TIFF_EXTERNAL OR BIGTIFF_SUPPORTED)
    #TODO: check cases then HAVE_BIGTIFF False
    set(HAVE_BIGTIFF TRUE)
    # add_definitions(-DTIFFLIB_VERSION_STR=${TIFF_VERSION})
else()
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${TIFF_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${TIFF_LIBRARIES})
    check_function_exists(TIFFScanlineSize64 HAVE_BIGTIFF)
endif()

if(HAVE_BIGTIFF)
    add_definitions(-DBIGTIFF_SUPPORT)
else()
    warning_msg("BigTIFF is not supported")
endif()

find_anyproject(GeoTIFF VERSION 1.2.1 REQUIRED CMAKE_ARGS
    -DBUILD_TESTING=OFF
    -DWITH_UTILITIES=OFF
)
find_anyproject(JPEG DEFAULT ON)
if(JPEG_FOUND)
    add_definitions(-DJPEG_SUPPORTED)
    add_definitions(-DHAVE_LIBJPEG) # Added only after 2.1 release (especially for geotiff.cpp where incorrect #ifdef is used)
endif()

find_anyproject(JPEG12 DEFAULT ON)
if(JPEG12_FOUND)
    add_definitions(-DJPEG12_SUPPORTED)
endif()
find_anyproject(PNG DEFAULT ON)

file(GLOB FORMATS ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(FORMAT ${FORMATS})
    if(EXISTS ${FORMAT}/CMakeLists.txt)
        if(NOT SUPPRESS_VERBOSE_OUTPUT)
            string(ASCII 27 Esc)
            set(Blue        "${Esc}[34m")
            set(ColourReset "${Esc}[m")
            get_filename_component(FORMAT_NAME ${FORMAT} NAME)
            message(STATUS "add raster format: ${Blue}${FORMAT_NAME}${ColourReset}")
        endif()
        add_subdirectory(${FORMAT})
    endif()
endforeach()

# send variables upper
set(TARGET_OBJECTS ${TARGET_OBJECTS} PARENT_SCOPE)
any_project_var_to_parent_scope()
set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} PARENT_SCOPE)
