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

option(BUILD_TESTS "Set ON to build tests" ON)
# testing
if(BUILD_TESTS)
  # call cmake enable_testing function
  enable_testing()
  #check for python
  find_package(PythonInterp REQUIRED)
  if(NOT PYTHONINTERP_FOUND)
    message(FATAL_ERROR "Cannot find python")
  endif()

  #add tests
  add_test(gcore ${PYTHON_EXECUTABLE} gcore)
  add_test(gdrivers ${PYTHON_EXECUTABLE} gdrivers)
  add_test(alg ${PYTHON_EXECUTABLE} alg)
  add_test(osr ${PYTHON_EXECUTABLE} osr)
  add_test(ogr ${PYTHON_EXECUTABLE} ogr)
  add_test(utilities ${PYTHON_EXECUTABLE} utilities)
  add_test(pyscripts ${PYTHON_EXECUTABLE} pyscripts)

endif()

