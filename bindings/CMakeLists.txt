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

cmake_minimum_required (VERSION 2.8.10 FATAL_ERROR)

option(GDAL_SWIG_PYTHON "Set ON to build python bindings" OFF)
if(GDAL_SWIG_PYTHON)
    set(GDAL_BINDINGS ${GDAL_BINDINGS} python PARENT_SCOPE)
    set(BINDING python)
    add_subdirectory(python)
endif()

option(GDAL_SWIG_PHP "Set ON to build php bindings" OFF)
if(GDAL_SWIG_PHP)
    set(GDAL_BINDINGS ${GDAL_BINDINGS} php PARENT_SCOPE)
    set(BINDING php)
#    add_subdirectory(php)
endif()

option(GDAL_SWIG_CSHARP "Set ON to build csharp bindings" OFF)
if(GDAL_SWIG_CSHARP)
    set(GDAL_BINDINGS ${GDAL_BINDINGS} csharp PARENT_SCOPE)
    set(BINDING csharp)
#    add_subdirectory(csharp)
endif()

option(GDAL_SWIG_RUBY "Set ON to build ruby bindings" OFF)
if(GDAL_SWIG_RUBY)
    set(GDAL_BINDINGS ${GDAL_BINDINGS} ruby PARENT_SCOPE)
    set(BINDING ruby)
#    add_subdirectory(ruby)
endif()

option(GDAL_SWIG_PERL "Set ON to build perl bindings" OFF)
if(GDAL_SWIG_PERL)
    set(GDAL_BINDINGS ${GDAL_BINDINGS} perl PARENT_SCOPE)
    set(BINDING perl)
#    add_subdirectory(perl)
endif()

option(GDAL_SWIG_JAVA "Set ON to build java bindings" OFF)
if(GDAL_SWIG_JAVA)
    set(GDAL_BINDINGS ${GDAL_BINDINGS} java PARENT_SCOPE)
    set(BINDING java)
#    add_subdirectory(java)
endif()

