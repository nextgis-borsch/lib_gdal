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

file(GLOB FORMATS ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(FORMAT ${FORMATS})
    if(EXISTS ${FORMAT}/CMakeLists.txt)
        if(NOT SUPPRESS_VERBOSE_OUTPUT)
            string(ASCII 27 Esc)
            set(Blue        "${Esc}[34m")
            set(ColourReset "${Esc}[m")
            get_filename_component(FORMAT_NAME ${FORMAT} NAME)
            message(STATUS "add vector format: ${Blue}${FORMAT_NAME}${ColourReset}")
        endif()
        add_subdirectory(${FORMAT})
    endif()
endforeach()


# send variables upper
set(TARGET_OBJECTS ${TARGET_OBJECTS} PARENT_SCOPE)
any_project_var_to_parent_scope()
set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} PARENT_SCOPE)
