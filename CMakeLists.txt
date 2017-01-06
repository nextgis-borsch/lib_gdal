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

# some init settings
set(CMAKE_COLOR_MAKEFILE ON)
# set path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

if(OSX_FRAMEWORK AND (BUILD_SHARED_LIBS OR BUILD_STATIC_LIBS))
  message(FATAL_ERROR "Only OSX_FRAMEWORK key or any or both BUILD_SHARED_LIBS
                       and BUILD_STATIC_LIBS keys are permitted")
endif()

if(OSX_FRAMEWORK)
  set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/Applications/${PROJECT_NAME}.app/Contents/MacOS/" CACHE INTERNAL "Installation directory for executables" FORCE)
  set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/Library/Frameworks" CACHE INTERNAL "Installation directory for libraries" FORCE)
  set(SKIP_INSTALL_EXECUTABLES ON)
  set(SKIP_INSTALL_EXPORT ON)
  set(SKIP_INSTALL_DOCS ON)
  set(CMAKE_MACOSX_RPATH ON)

  # use, i.e. don't skip the full RPATH for the build tree
  #set(CMAKE_SKIP_BUILD_RPATH  TRUE)

  # when building, don't use the install RPATH already
  # (but later on when installing)
  #set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

  # the RPATH to be used when installing
  #set(CMAKE_INSTALL_RPATH "${INSTALL_LIB_DIR}")
  #set(CMAKE_INSTALL_RPATH "@loader_path/../../../../Library/Frameworks")

  # don't add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  #set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

  set(GDAL_BUILD_DOCS OFF CACHE INTERNAL "Don't build docs on Mac OS X" FORCE)
else()
    include(GNUInstallDirs)

    set(INSTALL_BIN_DIR ${CMAKE_INSTALL_BINDIR} CACHE INTERNAL "Installation directory for executables" FORCE)
    set(INSTALL_LIB_DIR ${CMAKE_INSTALL_LIBDIR} CACHE INTERNAL "Installation directory for libraries" FORCE)
    set(INSTALL_INC_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}" CACHE INTERNAL "Installation directory for headers" FORCE)
    set(INSTALL_SHARE_DIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}" CACHE INTERNAL "Installation directory for share data" FORCE)
    set(INSTALL_PKGCONFIG_DIR "${INSTALL_LIB_DIR}/pkgconfig" CACHE INTERNAL "Installation directory for pkgconfig (.pc) files" FORCE)
    set(INSTALL_MAN_DIR ${CMAKE_INSTALL_MANDIR} CACHE INTERNAL "Installation directory for manual pages" FORCE)
    set(INSTALL_DOC_DIR ${CMAKE_INSTALL_DOCDIR} CACHE INTERNAL "Installation directory for doc pages" FORCE)
endif()

# setup package meta-data
include(CMakeDependentOption)
include(util)

check_version(GDAL_MAJOR_VERSION GDAL_MINOR_VERSION GDAL_REV_VERSION)
set(VERSION ${GDAL_MAJOR_VERSION}.${GDAL_MINOR_VERSION}.${GDAL_REV_VERSION})
set(SOVERSION 20)

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
    set(TARGET_LINK_LIB ${TARGET_LINK_LIB} Ws2_32.lib)
endif()


find_anyproject(ICONV DEFAULT ON) #default OFF, to set ON add DEFAULT ON
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

find_anyproject(EXPAT DEFAULT ON)
if(EXPAT_FOUND)
    add_definitions(-DHAVE_EXPAT)
    if(NOT BUILD_SHARED_LIBS)
        add_definitions(-DXML_STATIC)
    endif()
endif()

find_anyproject(JSONC REQUIRED)

# TODO: move this to /drivers or /drivers/raster while it's not needed for the core GDAL
find_anyproject(JPEG DEFAULT ON)
if(JPEG_FOUND)
    add_definitions(-DJPEG_SUPPORTED)
    add_definitions(-DHAVE_LIBJPEG) # added only after 2.1 release (especially for geotiff.cpp where incorrect #ifdef is used)
endif()

find_anyproject(JPEG12)
if(JPEG12_FOUND)
    add_definitions(-DJPEG12_SUPPORTED)
endif()

find_anyproject(LibLZMA)

find_anyproject(TIFF REQUIRED CMAKE_ARGS -DSKIP_BUILD_DOCS=ON)
find_anyproject(GeoTIFF VERSION 1.2.1 REQUIRED)

find_anyproject(SQLite3 CMAKE_ARGS -DENABLE_COLUMN_METADATA=ON)
if(SQLITE3_FOUND)
    option(ENABLE_SQLITE_GPKG "Build SQLite & GeoPackage (raster + vector) formats" ON)
    if(ENABLE_SQLITE_GPKG)
        add_definitions(-DSQLITE_ENABLED)
    endif()
endif()

# TODO: move this to the according drivers.
find_anyproject(PostgreSQL) # find libpq

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

# setup paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/port)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/gcore)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/ogr)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core/alg)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/apps)

# main directories
add_subdirectory(core)
add_subdirectory(drivers)

# link library objects
list(REMOVE_DUPLICATES TARGET_OBJECTS)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/core/gcore/gdal_version.h
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
    endif()
endif()
set(INSTALL_TARGETS ${INSTALL_TARGETS} ${LIB_NAME})

set(_symbols "GDAL_1.8 { global: *; };")
set(_version_script "${CMAKE_CURRENT_BINARY_DIR}/version.script")
file(WRITE ${_version_script} "${_symbols}\n")

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	# Create a version script for the GNU ld.
	set(LINK_FLAGS "${LINK_FLAGS} -Wl,-Bsymbolic -Wl,--version-script,\"${_version_script}\"")
endif ()

if(BUILD_SHARED_LIBS OR OSX_FRAMEWORK)
    if(WIN32)
        set (LIB_VERSION ${GDAL_MAJOR_VERSION}0)
        if(MSVC)
            set_target_properties(${LIB_NAME} PROPERTIES IMPORT_SUFFIX "_i.lib")
        endif()
    else ()
      set (LIB_VERSION "${GDAL_MAJOR_VERSION}0.${GDAL_MINOR_VERSION}.${GDAL_REV_VERSION}")
    endif()
    set_target_properties (${LIB_NAME} PROPERTIES VERSION ${LIB_VERSION})
    set_target_properties (${LIB_NAME} PROPERTIES SOVERSION ${SOVERSION})
endif()


if(UNIX)
    set_target_properties(${LIB_NAME} PROPERTIES
        LINK_FLAGS "${LINK_FLAGS}"
        OUTPUT_NAME ${PROJECT_NAME})
    # Add Apple staff
    if(NOT APPLE AND BUILD_SHARED_LIBS)
        set_target_properties(${LIB_NAME} PROPERTIES LINK_FLAGS "-Wl,--version-script,\"${_version_script}\"")
    endif()
elseif(BUILD_SHARED_LIBS AND WIN32)
    set_target_properties(${LIB_NAME} PROPERTIES SUFFIX "${GDAL_MAJOR_VERSION}${GDAL_MINOR_VERSION}.dll")
endif()

target_link_extlibraries(${LIB_NAME})

if (REGISTER_PACKAGE)
    export(TARGETS ${LIB_NAME} FILE ${LIB_NAME}-exports.cmake)
    set(EXPORT_PACKAGE_NAME GDAL)
    set(EXPORT_PACKAGE_NAME_UPPER GDAL)
    set(EXPORT_PACKAGE_LIB_NAME ${LIB_NAME})

    export(PACKAGE ${EXPORT_PACKAGE_NAME})

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        ${CMAKE_SOURCE_DIR}/cmake/PackageConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_PACKAGE_NAME}Config.cmake
        INSTALL_DESTINATION ${INSTALL_LIB_DIR}/cmake
        PATH_VARS INSTALL_INC_DIR )
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${EXPORT_PACKAGE_NAME}ConfigVersion.cmake
        VERSION ${VERSION}
        COMPATIBILITY AnyNewerVersion )
else()
    export(TARGETS ${LIB_NAME} FILE ${LIB_NAME}-exports.cmake EXPORT_LINK_INTERFACE_LIBRARIES)
endif()

set(GDAL_BINDINGS)
add_subdirectory(bindings)
add_subdirectory(docs)

# install
include(install)

# uninstall
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# info
if(NOT SUPPRESS_VERBOSE_OUTPUT)
    include(summary)
endif()
