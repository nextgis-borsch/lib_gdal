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

include(configure)

option(WITH_ODBC "Set ON to use odbc" OFF)
option(WITH_XMLREFORMAT "Set ON to use xmlreformat" OFF)
option(WITH_ONLY_CRYPTODLL_ALG "Include cryptopp support" OFF)

#Find zlib or configure ExternalProject if not found .
include(ZLIB)
include(CURL)

include_directories(${GDAL_ROOT_BINARY_DIR}/port)

set(CPL_HEADERS
    cpl_atomic_ops.h
    cpl_config_extras.h
    cpl_conv.h
    cpl_csv.h
    cpl_error.h
    cpl_hash_set.h
    cpl_http.h
    cplkeywordparser.h
    cpl_list.h
    cpl_minixml.h
    cpl_multiproc.h
    cpl_port.h
    cpl_progress.h
    cpl_quad_tree.h
    cpl_spawn.h
    cpl_string.h
    cpl_time.h
    cpl_virtualmem.h
    cpl_vsi.h
    cpl_vsi_virtual.h
    cpl_worker_thread_pool.h
    gdal_csv.h
)

set(CPL_SOURCES
    cpl_atomic_ops.cpp
    cpl_base64.cpp
    cpl_conv.cpp
    cpl_csv.cpp
    cpl_error.cpp
    cpl_findfile.cpp
    cpl_getexecpath.cpp
    cplgetsymbol.cpp
    cpl_google_oauth2.cpp
    cpl_hash_set.cpp
    cpl_http.cpp
    cplkeywordparser.cpp
    cpl_list.cpp
    cpl_minixml.cpp
    cpl_multiproc.cpp
    cpl_path.cpp
    cpl_progress.cpp
    cpl_quad_tree.cpp
    cpl_recode.cpp
    cpl_recode_stub.cpp
    cpl_spawn.cpp
    cpl_string.cpp
    cplstring.cpp
    cplstringlist.cpp
    cpl_strtod.cpp
    cpl_time.cpp
    cpl_virtualmem.cpp
    cpl_vsil_abstract_archive.cpp
    cpl_vsil_buffered_reader.cpp
    cpl_vsil_cache.cpp
    cpl_vsil.cpp
#    cpl_vsil_simple.cpp
    cpl_vsil_sparsefile.cpp
    cpl_vsil_stdin.cpp
    cpl_vsil_stdout.cpp
    cpl_vsil_subfile.cpp
    cpl_vsil_tar.cpp
    cpl_vsi_mem.cpp
    cpl_vsisimple.cpp
    cpl_worker_thread_pool.cpp
    cpl_xml_validate.cpp
    vsipreload.cpp
)

if(WIN32)
    list(APPEND CPL_SOURECS cpl_vsil_win32.cpp)
else()
    list(APPEND CPL_SOURECS cpl_vsil_unix_stdio_64.cpp)
endif()

#check odbc
if(WITH_ODBC)
    list(APPEND CPL_HEADERS cpl_odbc.h)
    list(APPEND CPL_SOURECS cpl_odbc.cpp)
endif()

if(WITH_XMLREFORMAT)
    list(APPEND CPL_SOURECS xmlreformat.cpp)
endif()

if(WITH_ONLY_CRYPTODLL_ALG)
    #todo: check cryptopp func
    add_definitions(-DUSE_ONLY_CRYPTODLL_ALG)
endif()

if(HAVE_ICONV)
    list(APPEND CPL_SOURECS cpl_recode_iconv.cpp )
    include_directories(${ICONV_INCLUDE_DIR})
    add_definitions(-DHAVE_ICONV -DLIBICONV_PLUG)
endif()

#check zip and internal zip
if(WITH_ZLIB)
    add_definitions(-DHAVE_LIBZ -DZIP_SUPPORT)
    list(APPEND CPL_HEADERS
        cpl_minizip_ioapi.h
        cpl_minizip_unzip.h
        cpl_minizip_zip.h
    )
    list(APPEND CPL_SOURECS 
        cpl_vsil_gzip.cpp
        cpl_minizip_ioapi.cpp
        cpl_minizip_unzip.cpp
        cpl_minizip_zip.cpp
    )
endif()

#check curl
if(WITH_CURL)
  add_definitions(-DHAVE_CURL)
  list(APPEND CPL_HEADERS cpl_vsil_curl_priv.h)

  list(APPEND CPL_SOURECS
      cpl_vsil_curl.cpp
      cpl_vsil_curl_streaming.cpp )
   
endif()

add_library(cpl OBJECT ${CPL_HEADERS} ${CPL_SOURECS})

if(HAVE_ICONV)
    list(APPEND GDAL_TARGET_LINK_LIB ${ICONV_LIBRARIES})
endif()

if(WITH_ZLIB)
  if(NOT GDAL_USES_EXTERNAL_ZLIB)
  add_dependencies(cpl zlib)
   endif()
  list(APPEND GDAL_TARGET_LINK_LIB ${ZLIB_LIBRARIES})
endif()

if(WITH_CURL)
  if(NOT GDAL_USES_EXTERNAL_CURL)
    add_dependencies(cpl curl)
  endif()
  list(APPEND GDAL_TARGET_LINK_LIB ${CURL_LIBRARIES})
endif()

# build target
set(GDAL_TARGET_OBJECTS ${GDAL_TARGET_OBJECTS} $<TARGET_OBJECTS:cpl> PARENT_SCOPE)
set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} PARENT_SCOPE)

# install headers
set(GDAL_INSTALL_HEADERS ${GDAL_INSTALL_HEADERS} ${CPL_HEADERS} PARENT_SCOPE)
