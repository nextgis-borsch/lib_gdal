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

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)

project(gdal)

set(PACKAGE_NAME GDAL)
set(PACKAGE_NAME GDAL)
set(PACKAGE_BUGREPORT "gdal-dev@lists.osgeo.org" )
set(PACKAGE_URL "http://gdal.org/" )

# some init settings
set(CMAKE_COLOR_MAKEFILE ON)
# set path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE INTERNAL "Installation directory for executables" FORCE)
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE INTERNAL "Installation directory for libraries" FORCE)
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include/gdal" CACHE INTERNAL "Installation directory for headers" FORCE)
set(INSTALL_SHARE_DIR "${CMAKE_INSTALL_PREFIX}/share/gdal" CACHE INTERNAL "Installation directory for share data" FORCE)
set(INSTALL_PKGCONFIG_DIR "${INSTALL_LIB_DIR}/pkgconfig" CACHE INTERNAL "Installation directory for pkgconfig (.pc) files" FORCE)
set(INSTALL_MAN_DIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE INTERNAL "Installation directory for manual pages" FORCE)

# setup package meta-data
include(util)

check_version(GDAL_MAJOR_VERSION GDAL_MINOR_VERSION GDAL_REV_VERSION)
set(VERSION ${GDAL_MAJOR_VERSION}.${GDAL_MINOR_VERSION}.${GDAL_REV_VERSION})

report_version(${PROJECT_NAME} ${VERSION})  

if(UNIX)
    option(GDAL_INSTALL_DATA_IN_VERSION_DIR "Set ON to install GDAL in path with version (i.e. usr/local/share/gdal/1.11" OFF)
    if(GDAL_INSTALL_DATA_IN_VERSION_DIR)
        set(INSTALL_SHARE_DIR "${CMAKE_INSTALL_PREFIX}/share/gdal/${GDAL_VERSION}" CACHE INTERNAL "Installation directory for share data" FORCE)
    endif()
endif()


set(TARGET_OBJECTS)

include(FindAnyProject)

if(WIN32)
    set(TARGET_LINK_LIB ${TARGET_LINK_LIB} Ws2_32.lib)
endif()


find_anyproject(ICONV DEFAULT ON) #default OFF, to set ON add DEFAULT ON
if(ICONV_FOUND)
    set(HAVE_ICONV TRUE)
    add_definitions(-DHAVE_ICONV -DLIBICONV_PLUG)
    if(${ICONV_SECOND_ARGUMENT_IS_CONST})
        set(ICONV_CONST "const")
    endif()

    if(${ICONV_SECOND_ARGUMENT_CPP_IS_CONST})
        set(ICONV_CPP_CONST "const")
    endif()
else()
    message(WARNING "No iconv support")
endif()

find_anyproject(ZLIB DEFAULT ON)
if(ZLIB_FOUND)
    add_definitions(-DHAVE_LIBZ -DZIP_SUPPORT)
else()
    message(WARNING "No zlib support")
endif()

find_anyproject(CURL DEFAULT ON)
if(CURL_FOUND)
    add_definitions(-DHAVE_CURL)
else()
    message(WARNING "No curl support")
endif()

find_anyproject(GEOS DEFAULT ON)
if(GEOS_FOUND)
    add_definitions(-DHAVE_GEOS=1)
    get_filename_component(GEOS_ROOT ${GEOS_INCLUDE_DIR} PATH)
    include_directories(${GEOS_ROOT}/capi)
else()
    message(WARNING "No geos support")
endif()

find_anyproject(PROJ4 DEFAULT ON)
if(PROJ4_FOUND)
    add_definitions(-DHAVE_PROJ) # todo: do we need it? -DPROJ_STATIC)
else()
    message(WARNING "No proj.4 support")
endif()

find_anyproject(LibXml2 DEFAULT ON)
if(LIBXML2_FOUND)
    add_definitions(-DHAVE_LIBXML2)
endif()

#TODO:
#ifeq ($(HAVE_SQLITE),yes)
#CXXFLAGS :=	$(CXXFLAGS) -DSQLITE_ENABLED
#endif

include(configure)

# setup paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# main directories
add_subdirectory(core)
#add_subdirectory(bindings)
add_subdirectory(docs)

# link library objects
list(REMOVE_DUPLICATES TARGET_OBJECTS)

if(BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
    set(LIB_NAME ${PROJECT_NAME})
    add_library(${LIB_NAME} SHARED ${TARGET_OBJECTS})
    if(WIN32)
        set (LIB_VERSION ${GDAL_MAJOR_VERSION})
        if(MSVC)        
            set_target_properties(${LIB_NAME} PROPERTIES IMPORT_SUFFIX "_i.lib")
        endif()
    else ()
      set (LIB_VERSION ${VERSION})
    endif()
    
    set_target_properties (${LIB_NAME} PROPERTIES VERSION ${LIB_VERSION})
    set_target_properties (${LIB_NAME} PROPERTIES SOVERSION ${LIB_VERSION})
else()
    set(LIB_TYPE STATIC)
    set(LIB_NAME ${PROJECT_NAME}static)
    add_library(${LIB_NAME} STATIC ${TARGET_OBJECTS})
endif()

if(UNIX)
    set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
# TODO: Add Apple staff
#    if(NOT APPLE AND BUILD_SHARED_LIBS)
#        set_target_properties(${LIB_NAME} PROPERTIES LINK_FLAGS "-Wl,--version-script,\"${CMAKE_CURRENT_SOURCE_DIR}/zlib.map\"")
#    endif()
elseif(BUILD_SHARED_LIBS AND WIN32)
    set_target_properties(${LIB_NAME} PROPERTIES SUFFIX "${GDAL_MAJOR_VERSION}${GDAL_MINOR_VERSION}.dll")
endif()

target_link_extlibraries(${LIB_NAME}) 
export(TARGETS ${LIB_NAME} FILE ${LIB_NAME}-exports.cmake EXPORT_LINK_INTERFACE_LIBRARIES)

# install
include(install)

# todo: target to build docs

if(NOT SUPRESS_VERBOSE_OUTPUT)
    include(summary)
endif()

# uninstall
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${GDAL_ROOT_BINARY_DIR}/cmake_uninstall.cmake)
