################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Mikhail Gusev, gusevmihs@gmail.com
################################################################################
# Copyright (C) 2015-2016, NextGIS <info@nextgis.com>
# Copyright (C) 2016 Mikhail Gusev
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

option(ENABLE_ODS "Build Open Document Spreadsheet format" OFF)
if(ENABLE_ODS)
    set(TARGET_NAME vec_ods)

    add_definitions(-DODS_ENABLED)
	set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DODS_ENABLED" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_SOURCE_DIR}/drivers/vector/mem)
    
    if(EXPAT_FOUND)
        include_directories(${EXPAT_INCLUDE_DIRS})
    endif()

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        ogr_ods.h 
        ods_formula.h 
        ods_formula_parser.hpp 
        ${CMAKE_SOURCE_DIR}/drivers/vector/mem/ogr_mem.h
    )

    set(LIB_CSOURCES
        ogrodsdriver.cpp 
        ogrodsdatasource.cpp 
        ods_formula.cpp 
        ods_formula_node.cpp
    )
    
    # Comment from ogrsf_frmts/ods/GNUMakefile:
    # The sed substitution below workarounds a bug with gcc 4.1 -O2 (checked on 64bit platforms)
    # that produces buggy compiled code.
    # Seen on gcc 4.1.2-27ubuntu1 (Ubuntu 10.04) (not the default compiler) and gcc-4.1.2-48.el5 (CentOS 5.5)
    # (default compiler...)
    # The memset isn't necessary at all with a non-buggy compiler, but I've found
    # that it helps gcc 4.1 generating correct code here...
    # The sed with yyerrorlab shut up MSVC warnings about unreachable code
    # TODO: implement here targets: parser, testparser if needed.

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
endif()
