################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2017, NextGIS <info@nextgis.com>
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

set(WITH_EXPAT ON)
set(WITH_GeoTIFF ON)
set(WITH_ICONV ON)
set(WITH_JSONC ON)
set(WITH_LibXml2 ON)
set(WITH_TIFF ON)
set(WITH_ZLIB ON)
set(WITH_JBIG ON)
set(WITH_JPEG ON)
set(WITH_JPEG12 ON)
set(WITH_LibLZMA ON)
set(WITH_PYTHON ON)
set(WITH_PYTHON3 OFF)
set(WITH_PNG ON)
set(WITH_OpenSSL ON)
set(WITH_SQLite3 ON)
set(WITH_PostgreSQL ON)

set(GDAL_ENABLE_GNM ON)
set(GDAL_BUILD_APPS ON)
set(CMAKE_BUILD_TYPE Release)
if(APPLE)
    set(OSX_FRAMEWORK ON)
endif()

if(WIN32)
    set(WITH_EXPAT_EXTERNAL ON)
    set(WITH_GeoTIFF_EXTERNAL ON)
    set(WITH_ICONV_EXTERNAL ON)
    set(WITH_JSONC_EXTERNAL ON)
    set(WITH_LibXml2_EXTERNAL ON)
    set(WITH_TIFF_EXTERNAL ON)
    set(WITH_ZLIB_EXTERNAL ON)
    set(WITH_JBIG_EXTERNAL ON)
    set(WITH_JPEG_EXTERNAL ON)
    set(WITH_JPEG12_EXTERNAL ON)
    set(WITH_LibLZMA_EXTERNAL ON)
    set(WITH_PNG_EXTERNAL ON)
    set(WITH_OpenSSL_EXTERNAL ON)
    set(WITH_SQLite3_EXTERNAL ON)
    set(WITH_PostgreSQL_EXTERNAL ON)
endif()
