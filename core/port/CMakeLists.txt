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


set(LIB_NAME "cpl")
project ("lib${LIB_NAME}")

include(configure)

#Find zlib or configure ExternalProject if not found .
include(ZLIB)

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
option(WITH_ODBC "Set ON to use odbc" OFF)

if(WITH_ODBC)
    list(APPEND CPL_HEADERS cpl_odbc.h)
    list(APPEND CPL_SOURECS cpl_odbc.cpp)
endif()

option(WITH_XMLREFORMAT "Set ON to use xmlreformat" OFF)
if(WITH_XMLREFORMAT)
    list(APPEND CPL_SOURECS xmlreformat.cpp)
endif()

option(WITH_ONLY_CRYPTODLL_ALG "Include cryptopp support" OFF)
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
#  add_dependencies( ZLIB)
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

#should we apply same method as in ZLIB ?
#check curl
option(WITH_CURL "Set ON to use libcurl" ON)
if(WITH_CURL)
    option(GDAL_USE_CURL_INTERNAL "Set ON to use internal libcurl" OFF)
    add_definitions(-DHAVE_CURL)
    set(CPL_HEADERS cpl_vsil_curl_priv.h)

    list(APPEND CPL_SOURECS
      cpl_vsil_curl.cpp
      cpl_vsil_curl_streaming.cpp
      )

    if(GDAL_USE_CURL_INTERNAL)
        ExternalProject_Add(openssl
            GIT_REPOSITORY ${EP_URL}/lib_openssl
            INSTALL_COMMAND ""
        )
        set (OPENSSL_DIR ${ep_base}/Source/openssl CACHE INTERNAL "openssl internal path")
        set (OPENSSL_INCLUDE_DIR ${OPENSSL_DIR} CACHE PATH "openssl internal include path")
        #set (OPENSSL_LIBRARIES ${OPENSSL_DIR}/lib/openssl CACHE INTERNAL "openssl internal lib path")
        #set (OPENSSL_LIBRARY ${OPENSSL_LIBRARIES} CACHE FILEPATH "openssl internal lib path")
        
        link_directories(${OPENSSL_DIR}/Install/openssl)
        
        ExternalProject_Add(curl
            DEPENDS zlib
            DEPENDS openssl
            GIT_REPOSITORY ${EP_URL}/lib_curl
            CMAKE_ARGS
            -DBUILD_CURL_TESTS=OFF
            -DCURL_DISABLE_FTP=ON
            -DCURL_DISABLE_LDAP=ON
            -DCURL_DISABLE_TELNET=ON
            -DCURL_DISABLE_DICT=ON
            -DCURL_DISABLE_FILE=ON
            -DCURL_DISABLE_TFTP=ON
            -DCURL_DISABLE_LDAPS=ON
            -DCURL_DISABLE_RTSP=ON
            -DCURL_DISABLE_PROXY=ON
            -DCURL_DISABLE_POP3=ON
            -DCURL_DISABLE_IMAP=ON
            -DCURL_DISABLE_SMTP=ON
            -DCURL_DISABLE_GOPHER=ON
            -DCURL_DISABLE_CRYPTO_AUTH=OFF
            -DENABLE_IPV6=OFF
            -DENABLE_MANUAL=OFF
            -DCMAKE_USE_OPENSSL=ON
            -DCMAKE_USE_LIBSSH2=OFF
            INSTALL_COMMAND ""
        )
        
        set (LIBCURL_DIR ${ep_base}/Source/curl CACHE INTERNAL "libcurl internal path")
        set (LIBCURL_INCLUDE_DIR ${LIBCURL_DIR} CACHE PATH "libcurl internal include path")
        #set (LIBCURL_LIBRARIES ${LIBCURL_DIR}/lib/curl CACHE INTERNAL "libcurl internal lib path")
        #set (LIBCURL_LIBRARY ${LIBCURL_LIBRARIES} CACHE FILEPATH "libcurl internal lib path")
        
        link_directories(${LIBCURL_DIR}/Install/curl)
    else()
        find_package(CURL REQUIRED)
        if(CURL_FOUND)
            include_directories(${CURL_INCLUDE_DIRS})
        endif()
    endif()
endif()

add_library(${LIB_NAME} OBJECT ${CPL_HEADERS} ${CPL_SOURECS})

if(HAVE_ICONV)
    list(APPEND GDAL_TARGET_LINK_LIB ${ICONV_LIBRARIES})
endif()

if(WITH_ZLIB)
    # if(GDAL_USE_LIBZ_INTERNAL)        
    #     add_dependencies(${LIB_NAME} zlib)
    # endif()    
    list(APPEND GDAL_TARGET_LINK_LIB ${ZLIB_LIBRARIES})
endif()

if(WITH_CURL)
    if(GDAL_USE_CURL_INTERNAL)        
        add_dependencies(${LIB_NAME} curl)
        #set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} curl)
    else()
        list(GDAL_TARGET_LINK_LIB ${CURL_LIBRARIES})
    endif()    
endif()

# build target
set(GDAL_TARGET_OBJECTS ${GDAL_TARGET_OBJECTS} $<TARGET_OBJECTS:${LIB_NAME}> PARENT_SCOPE)
set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} PARENT_SCOPE)

# install headers
set(GDAL_INSTALL_HEADERS ${GDAL_INSTALL_HEADERS} ${CPL_HEADERS} PARENT_SCOPE)
