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

cmake_minimum_required (VERSION 2.8.10 FATAL_ERROR)

project(GDAL)

# some init settings
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)

# setup paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(GDAL_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(GDAL_ROOT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# set path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# add external project support
include(ExternalProject)
set(ep_base "${GDAL_ROOT_BINARY_DIR}/third-party")
set_property(DIRECTORY PROPERTY "EP_BASE" ${ep_base})
set(EP_URL "https://github.com/nextgis-extra")
    
# setup package meta-data
include(util)
set(GDAL_MAJOR_VERSION -1)
set(GDAL_MINOR_VERSION -1)
set(GDAL_REV_VERSION -1)
check_version(GDAL_MAJOR_VERSION GDAL_MINOR_VERSION GDAL_REV_VERSION)
set(GDAL_VERSION ${GDAL_MAJOR_VERSION}.${GDAL_MINOR_VERSION}.${GDAL_REV_VERSION})
message(STATUS "gdal version=[${GDAL_VERSION}]")

if(WIN32)    
    set(GDAL_LIB_NAME gdal${GDAL_MAJOR_VERSION}${GDAL_MINOR_VERSION})
elseif(UNIX)
    set(GDAL_LIB_NAME gdal)
endif()
message(STATUS "GDAL lib name ... " ${GDAL_LIB_NAME})

set(GDAL_TARGET_OBJECTS "")
set(GDAL_TARGET_LINK_LIB "")
set(GDAL_INSTALL_HEADERS "")

# main directories
add_subdirectory(core)
add_subdirectory(bindings)

add_subdirectory(autotest)

# link library objects
list(REMOVE_DUPLICATES GDAL_TARGET_OBJECTS)

option(GDAL_BUILD_SHARED "Set ON to build shared library" ON)
if(GDAL_BUILD_SHARED)
    add_library(${GDAL_LIB_NAME} SHARED ${GDAL_TARGET_OBJECTS})
else(GDAL_BUILD_SHARED)
    add_library(${GDAL_LIB_NAME} STATIC ${GDAL_TARGET_OBJECTS})
endif(GDAL_BUILD_SHARED)

list(REMOVE_DUPLICATES GDAL_TARGET_LINK_LIB)
target_link_libraries(${GDAL_LIB_NAME} ${GDAL_TARGET_LINK_LIB})

# install
include(install)

# todo: target to build docs




include(summary)
