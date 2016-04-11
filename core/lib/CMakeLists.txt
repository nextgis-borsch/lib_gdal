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

file(GLOB LIBS ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(LIB ${LIBS})
    if(EXISTS ${LIB}/CMakeLists.txt)    
        if(NOT SUPRESS_VERBOSE_OUTPUT)
            string(ASCII 27 Esc)
            set(Blue        "${Esc}[34m")
            set(ColourReset "${Esc}[m")
            get_filename_component(LIB_NAME ${LIB} NAME)
            message(STATUS "add library: ${Blue}${LIB_NAME}${ColourReset}")
        endif()
        add_subdirectory(${LIB})
    endif()    
endforeach()
    
# send variables upper
set(TARGET_OBJECTS ${TARGET_OBJECTS} PARENT_SCOPE)
#set(TARGET_LINK_LIB ${TARGET_LINK_LIB} PARENT_SCOPE)
#set(DEPENDENCY_LIB ${DEPENDENCY_LIB} PARENT_SCOPE)
#set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} PARENT_SCOPE)
