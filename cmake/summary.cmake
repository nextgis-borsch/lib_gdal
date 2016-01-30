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

macro(summary_message text value)
    if("${${value}}")
        message(STATUS "  ${text} yes")
    else()
        message(STATUS "  ${text} no")
    endif()
endmacro()

message(STATUS "GDAL is now configured for ${CMAKE_SYSTEM_NAME}")
message(STATUS "")
foreach(TO ${GDAL_SUMMARY})
    message(STATUS "${TO}")
endforeach()
message(STATUS "")
message(STATUS "  Installation directory:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  C compiler:                ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS}")
message(STATUS "  C++ compiler:              ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS}")
if(GDAL_BINDINGS)
    message(STATUS "  SWIG Bindings:             ${GDAL_BINDINGS}")
else()
    message(STATUS "  SWIG Bindings:             no")
endif()
message(STATUS "")
summary_message("Zlib support:              " WITH_ZLIB)
summary_message("cURL support (wms/wcs/...):" WITH_CURL)
summary_message("GEOS support:              " GDAL_USE_GEOS)
summary_message("OpenCL support:            " GDAL_USE_OPENCL)
summary_message("Armadillo support:         " GDAL_USE_ARMADILLO)
summary_message("Expat support:             " GDAL_USE_EXPAT)
summary_message("PROJ.4 support:            " WITH_PROJ4)
summary_message("Json-c support:            " GDAL_USE_JSONC)
summary_message("enable OGR building:       " GDAL_ENABLE_OGR)
summary_message("enable GNM building:       " GDAL_ENABLE_GNM)
message(STATUS "")
summary_message("enable pthread support:    " GDAL_CPL_MULTIPROC_PTHREAD)
summary_message("enable POSIX iconv support:" HAVE_ICONV)
summary_message("hide internal symbols:     " GDAL_HIDE_INTERNAL_SYMBOLS)
