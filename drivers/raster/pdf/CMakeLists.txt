################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
################################################################################
# Copyright (C) 2015-2023, NextGIS <info@nextgis.com>
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

option(ENABLE_PDF "Build PDF format" ON)
if (ENABLE_PDF)
	set(TARGET_NAME ras_pdf)

    add_definitions(-DFRMT_pdf)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_pdf" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/vrt)
    include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/mem)

    set(LIB_HHEADERS_PRIV
		  gdal_pdf.h
          pdfcreatecopy.h
          pdfio.h
          pdfobject.h
          pdfcreatecopy.h
          pdfsdk_headers.h
    )

    set(LIB_CSOURCES
          ogrpdflayer.cpp
          pdfcreatecopy.cpp
          pdfdataset.cpp
          pdfio.cpp
          pdfobject.cpp
          pdfreadvectors.cpp
          ogrpdflayer.cpp
          pdfwritabledataset.cpp
          pdfcreatefromcomposition.cpp
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip pdf")
endif()
