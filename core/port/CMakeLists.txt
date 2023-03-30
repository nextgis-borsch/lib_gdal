################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015-2021, NextGIS <info@nextgis.com>
# Copyright (C) 2012-2021 Dmitry Baryshnikov
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

option(WITH_ODBC "Set ON to use odbc" OFF)
option(WITH_XMLREFORMAT "Set ON to use xmlreformat" OFF)
option(WITH_ONLY_CRYPTODLL_ALG "Include cryptopp support" OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TARGET_NAME cpl)

set(LIB_HHEADERS
    ${CMAKE_BINARY_DIR}/cpl_config.h
    cpl_atomic_ops.h
    cpl_aws.h
    cpl_conv.h
    cpl_csv.h
    cpl_error.h
    cpl_hash_set.h
    cpl_http.h
    cplkeywordparser.h
    cpl_list.h
    cpl_minixml.h
    cpl_minizip_ioapi.h
    cpl_minizip_unzip.h
    cpl_minizip_zip.h
    cpl_multiproc.h
    cpl_odbc.h
    cpl_port.h
    cpl_progress.h
    cpl_quad_tree.h
    cpl_sha256.h
    cpl_spawn.h
    cpl_string.h
    cpl_time.h
    cpl_virtualmem.h
    cpl_vsi.h
    cpl_vsi_error.h
    cpl_vsi_virtual.h
    cpl_worker_thread_pool.h
    gdal_csv.h
    cpl_cpu_features.h
    cpl_google_cloud.h
    cpl_sha1.h
    cpl_json.h
    cpl_mem_cache.h
    cpl_vsil_curl_class.h
    cpl_userfaultfd.h
    cpl_auto_close.h
    cpl_json_streaming_writer.h
    cpl_vax.h
    cpl_compressor.h
    cpl_float.h
)

set(LIB_HHEADERS_PRIV
    cpl_config_extras.h
    cpl_alibaba_oss.h
    cpl_azure.h
    cpl_json_header.h
    cpl_json_streaming_parser.h
    cpl_md5.h
    cpl_swift.h
    cpl_safemaths.hpp
    cpl_error_internal.h
    cpl_curl_priv.h
    cpl_zlib_header.h
)

set(LIB_CSOURCES
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
    cpl_vsil_adls.cpp
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
    cpl_aws.cpp
    cpl_sha256.cpp
    cpl_vsil_crypt.cpp
    cpl_vsi_error.cpp
    cpl_vsil_curl.cpp
    cpl_cpu_features.cpp
    cpl_google_cloud.cpp
    cpl_sha1.cpp
    cpl_alibaba_oss.cpp
    cpl_azure.cpp
    cpl_json_streaming_parser.cpp
    cpl_json.cpp
    cpl_md5.cpp
    cpl_swift.cpp
    cpl_vsil_s3.cpp
    cpl_vsil_gs.cpp
    cpl_vsil_az.cpp
    cpl_vsil_oss.cpp
    cpl_vsil_swift.cpp
    cpl_vsil_webhdfs.cpp
    cpl_vsil_hdfs.cpp
    cpl_userfaultfd.cpp
    cpl_json_streaming_writer.cpp
    cpl_vax.cpp
    cpl_vsil_uploadonclose.cpp
    cpl_compressor.cpp
    cpl_float.cpp
)

if(WIN32)
    list(APPEND LIB_CSOURCES cpl_vsil_win32.cpp cpl_aws_win32.cpp)
else()
    list(APPEND LIB_CSOURCES cpl_vsil_unix_stdio_64.cpp)
endif()

# Check odbc
if(WITH_ODBC)
    list(APPEND LIB_HHEADERS_PRIV cpl_odbc.h)
    list(APPEND LIB_CSOURCES cpl_odbc.cpp)
endif()

if(WITH_ONLY_CRYPTODLL_ALG)
    # TODO: Check cryptopp func
    add_definitions(-DUSE_ONLY_CRYPTODLL_ALG)
endif()

if(ICONV_FOUND)
    list(APPEND LIB_CSOURCES cpl_recode_iconv.cpp )
endif()

# Check zip and internal zip
if(ZLIB_FOUND)
    list(APPEND LIB_HHEADERS_PRIV
        cpl_minizip_ioapi.h
        cpl_minizip_unzip.h
        cpl_minizip_zip.h
    )
    list(APPEND LIB_CSOURCES
        cpl_vsil_gzip.cpp
        cpl_minizip_ioapi.cpp
        cpl_minizip_unzip.cpp
        cpl_minizip_zip.cpp
    )
endif()

# Check curl is present
if(CURL_FOUND)
  list(APPEND LIB_HHEADERS_PRIV cpl_vsil_curl_priv.h)
  list(APPEND LIB_CSOURCES
      #cpl_vsil_curl.cpp
      cpl_vsil_curl_streaming.cpp )
endif()

add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

# Build target
set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${TARGET_NAME} PARENT_SCOPE)

# Install headers
if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${LIB_HHEADERS} DESTINATION ${INSTALL_INC_DIR} COMPONENT headers)
endif()
