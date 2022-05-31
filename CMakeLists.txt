################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015-2020, NextGIS <info@nextgis.com>
# Copyright (C) 2012-2020 Dmitry Baryshnikov
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

# Some init settings
set(CMAKE_COLOR_MAKEFILE ON)
# Set path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

if(NOT SKIP_DEFAULTS)
    include(defaults)
endif()

if(OSX_FRAMEWORK AND (BUILD_SHARED_LIBS OR BUILD_STATIC_LIBS))
  message(FATAL_ERROR "Only OSX_FRAMEWORK key or any or both BUILD_SHARED_LIBS
                       and BUILD_STATIC_LIBS keys are permitted")
endif()

if(OSX_FRAMEWORK)
    set(INSTALL_BIN_DIR "bin" CACHE INTERNAL "Installation directory for executables" FORCE)
    set(INSTALL_LIB_DIR "Library/Frameworks" CACHE INTERNAL "Installation directory for libraries" FORCE)
    set(INSTALL_SHORT_INC_DIR "${INSTALL_LIB_DIR}/${PROJECT_NAME}.framework/Headers" CACHE INTERNAL "Installation directory for headers" FORCE)  # For package paths
    set(INSTALL_CMAKECONF_DIR ${INSTALL_LIB_DIR}/${PROJECT_NAME}.framework/Resources/CMake CACHE INTERNAL "Installation directory for cmake config files" FORCE)
    set(SKIP_INSTALL_EXECUTABLES OFF)
    set(SKIP_INSTALL_EXPORT ON)
    set(SKIP_INSTALL_DOCS ON)
    set(CMAKE_MACOSX_RPATH ON)

  # Use, i.e. don't skip the full RPATH for the build tree
  #set(CMAKE_SKIP_BUILD_RPATH  TRUE)

  # When building, don't use the install RPATH already
  # (but later on when installing)
  #set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

  # The RPATH to be used when installing
  #set(CMAKE_INSTALL_RPATH "${INSTALL_LIB_DIR}")
  #set(CMAKE_INSTALL_RPATH "@loader_path/../../../../Library/Frameworks")

  # Don't add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  #set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

    set(GDAL_BUILD_DOCS OFF CACHE INTERNAL "Don't build docs on Mac OS X" FORCE)
else()
    include(GNUInstallDirs)

    set(INSTALL_BIN_DIR ${CMAKE_INSTALL_BINDIR} CACHE INTERNAL "Installation directory for executables" FORCE)
    set(INSTALL_LIB_DIR ${CMAKE_INSTALL_LIBDIR} CACHE INTERNAL "Installation directory for libraries" FORCE)
    set(INSTALL_INC_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}" CACHE INTERNAL "Installation directory for headers" FORCE)
    set(INSTALL_SHORT_INC_DIR ${INSTALL_INC_DIR}) # For package paths
    set(INSTALL_SHARE_DIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}" CACHE INTERNAL "Installation directory for share data" FORCE)
    set(INSTALL_PKGCONFIG_DIR "${INSTALL_LIB_DIR}/pkgconfig" CACHE INTERNAL "Installation directory for pkgconfig (.pc) files" FORCE)
    set(INSTALL_MAN_DIR ${CMAKE_INSTALL_MANDIR} CACHE INTERNAL "Installation directory for manual pages" FORCE)
    set(INSTALL_DOC_DIR ${CMAKE_INSTALL_DOCDIR} CACHE INTERNAL "Installation directory for doc pages" FORCE)
    set(INSTALL_CMAKECONF_DIR ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/CMake CACHE INTERNAL "Installation directory for cmake config files" FORCE)
endif()

# Debug on Windows
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Setup package meta-data
include(CMakeDependentOption)
include(util)

check_version(GDAL_MAJOR_VERSION GDAL_MINOR_VERSION GDAL_REV_VERSION)
set(VERSION ${GDAL_MAJOR_VERSION}.${GDAL_MINOR_VERSION}.${GDAL_REV_VERSION})
set(SOVERSION 26)

if(OSX_FRAMEWORK)
    set(FRAMEWORK_VERSION "${GDAL_MAJOR_VERSION}")
    set(INSTALL_INC_DIR "${INSTALL_LIB_DIR}/${PROJECT_NAME}.framework/Versions/${FRAMEWORK_VERSION}/Headers" CACHE INTERNAL "Installation directory for headers" FORCE)
    set(INSTALL_SHARE_DIR "${INSTALL_LIB_DIR}/${PROJECT_NAME}.framework/Versions/${FRAMEWORK_VERSION}/Resources/data")
    set(INSTALL_DOC_DIR "${INSTALL_LIB_DIR}/${PROJECT_NAME}.framework/Versions/${FRAMEWORK_VERSION}/Resources/docs")
    set(INSTALL_MAN_DIR "${INSTALL_LIB_DIR}/${PROJECT_NAME}.framework/Versions/${FRAMEWORK_VERSION}/Resources/man")
endif()

report_version(${PROJECT_NAME} ${VERSION})

if(UNIX)
    option(GDAL_INSTALL_DATA_IN_VERSION_DIR "Set ON to install GDAL in path with version (i.e. usr/local/share/gdal/1.11" OFF)
    if(GDAL_INSTALL_DATA_IN_VERSION_DIR)
        set(INSTALL_SHARE_DIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/${GDAL_MAJOR_VERSION}.${GDAL_MINOR_VERSION}" CACHE INTERNAL "Installation directory for share data" FORCE)
    endif()
    add_definitions(-DINST_DATA="${CMAKE_INSTALL_PREFIX}/${INSTALL_SHARE_DIR}")
endif()

set(TARGET_OBJECTS)

include(FindAnyProject)

if(WIN32)
    set(TARGET_LINK_LIB ${TARGET_LINK_LIB} Ws2_32.lib wbemuuid.lib)
endif()


find_anyproject(ICONV DEFAULT ON) # If not set anything - default OFF, set ON to check ON by DEFAULT
if(ICONV_FOUND)
    set(HAVE_ICONV TRUE)
    add_definitions(-DHAVE_ICONV)
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

find_anyproject(EXPAT DEFAULT ON)
if(EXPAT_FOUND)
    add_definitions(-DHAVE_EXPAT)
    if(NOT BUILD_SHARED_LIBS)
        add_definitions(-DXML_STATIC)
    endif()
endif()

find_anyproject(JSONC REQUIRED
    NAMES json-c jsonc json_c
)
if(JSONC_FOUND)
    include_directories(${JSONC_INCLUDE_DIRS}/json-c)
endif()

find_anyproject(CURL DEFAULT ON CMAKE_ARGS
    -DBUILD_CURL_EXE=OFF
    -DHTTP_ONLY=ON
    -DUSE_MANUAL=OFF)
if(CURL_FOUND)
    add_definitions(-DHAVE_CURL)
else()
    message(WARNING "No curl support")
endif()

find_anyproject(LibXml2 DEFAULT ON)
if(LIBXML2_FOUND)
    add_definitions(-DHAVE_LIBXML2)
endif()

# This is here as geos used in core/ogr and apps
find_anyproject(GEOS DEFAULT ON)
if(GEOS_FOUND)
    add_definitions(-DHAVE_GEOS=1)
else()
    message(WARNING "No geos support")
endif()

find_anyproject(LibLZMA)
if(LIBLZMA_FOUND)
    add_definitions(-DHAVE_LZMA_H=1)
endif()

find_anyproject(SQLite3 CMAKE_ARGS -DENABLE_COLUMN_METADATA=ON)
if(SQLITE3_FOUND)
    add_definitions(-DHAVE_SQLITE=1)
    option(ENABLE_SQLITE_GPKG "Build SQLite & GeoPackage (raster + vector) formats" ON)
    if(ENABLE_SQLITE_GPKG)
        add_definitions(-DSQLITE_ENABLED)
    endif()
endif()

find_anyproject(PROJ REQUIRED)
if(PROJ_FOUND)
    add_definitions(-DHAVE_PROJ_H=1)
    # if(NOT BUILD_STATIC_LIBS)
    #     add_definitions(-DPROJ_RENAME_SYMBOLS)
    # endif()    
endif()

## Options goes to config.h and gdal-config ####################################
option(GDAL_USE_CPL "Set ON to use cpl" ON)
if(GDAL_USE_CPL)
    add_definitions(-DUSE_CPL)
endif(GDAL_USE_CPL)

set(GDAL_ENABLE_OGR ON) # Always on
option(GDAL_ENABLE_GNM "Build GNM module" OFF)
################################################################################

include(configure)

add_definitions(-DGDAL_COMPILATION)

# Setup paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/port)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/gcore)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/core/gcore)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/ogr)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/alg)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/apps)

# Main directories
add_subdirectory(core)
add_subdirectory(drivers)

# Link library objects
list(REMOVE_DUPLICATES TARGET_OBJECTS)

# Put the libraries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf
# directories. This simplifies manual testing and the use of the build
# tree rather than installed GDAL libraries.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})


set(LIB_NAME ${PROJECT_NAME})
if(BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

add_subdirectory(apps)

if(OSX_FRAMEWORK)
    set(INST_HEADER
        ${CMAKE_CURRENT_SOURCE_DIR}/core/gcore/gdal.h
        ${CMAKE_CURRENT_BINARY_DIR}/core/gcore/gdal_version.h
    )
    add_library(${LIB_NAME} SHARED ${TARGET_OBJECTS} ${INST_HEADER})
    set_target_properties(${LIB_NAME} PROPERTIES
      FRAMEWORK TRUE
      FRAMEWORK_VERSION ${FRAMEWORK_VERSION}
      MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${VERSION}
      MACOSX_FRAMEWORK_BUNDLE_VERSION ${VERSION}
      MACOSX_FRAMEWORK_IDENTIFIER org.osgeo.libgdal
      XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
      PUBLIC_HEADER "${INST_HEADER}"
    )
else()
    add_library(${LIB_NAME} ${LIB_TYPE} ${TARGET_OBJECTS})
    if(BUILD_SHARED_LIBS AND BUILD_STATIC_LIBS)
        add_library(${LIB_NAME}static STATIC ${TARGET_OBJECTS})
        set(INSTALL_TARGETS ${INSTALL_TARGETS} ${LIB_NAME}static)
        if(WIN32)
            set_target_properties(${LIB_NAME}static PROPERTIES OUTPUT_NAME ${LIB_NAME}_i)
        else()
            set_target_properties(${LIB_NAME}static PROPERTIES OUTPUT_NAME ${LIB_NAME})
        endif()
        set_target_properties(${LIB_NAME}static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    endif()
endif()

set_target_properties(${LIB_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

set(INSTALL_TARGETS ${INSTALL_TARGETS} ${LIB_NAME})

# Test for ligio_gdal in saga
# set(_symbols "GDAL_1.8 { global: *; };")
# set(_version_script "${CMAKE_CURRENT_BINARY_DIR}/version.script")
# file(WRITE ${_version_script} "${_symbols}\n")
#
# if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
# 	# Create a version script for the GNU ld.
# 	set(LINK_FLAGS "${LINK_FLAGS} -Wl,-Bsymbolic -Wl,--version-script,\"${_version_script}\"")
# endif ()

if(BUILD_SHARED_LIBS OR OSX_FRAMEWORK)
    if(WIN32)
        set (LIB_VERSION ${GDAL_MAJOR_VERSION}0)
        if(MSVC)
            set_target_properties(${LIB_NAME} PROPERTIES IMPORT_SUFFIX "_i.lib")
        endif()
    else ()
      set (LIB_VERSION "${SOVERSION}.${GDAL_MINOR_VERSION}.${GDAL_REV_VERSION}")
    endif()
    set_target_properties (${LIB_NAME} PROPERTIES VERSION ${LIB_VERSION})
    set_target_properties (${LIB_NAME} PROPERTIES SOVERSION ${SOVERSION})
endif()


if(UNIX)
    set_target_properties(${LIB_NAME} PROPERTIES
        LINK_FLAGS "${LINK_FLAGS}"
        OUTPUT_NAME ${PROJECT_NAME})
    # Add Apple staff
    # if(NOT APPLE AND BUILD_SHARED_LIBS)
    #     set_target_properties(${LIB_NAME} PROPERTIES LINK_FLAGS "-Wl,--version-script,\"${_version_script}\"")
    # endif()
elseif(BUILD_SHARED_LIBS AND WIN32)
    set_target_properties(${LIB_NAME} PROPERTIES SUFFIX "${LIB_VERSION}.dll")
endif()

target_link_extlibraries(${LIB_NAME})


set(GDAL_BINDINGS)
add_subdirectory(bindings)
add_subdirectory(docs)

enable_testing()
add_subdirectory(tests)

# Install
include(install)

# Uninstall
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Info
if(NOT SUPPRESS_VERBOSE_OUTPUT)
    include(summary)
endif()
