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

find_package(PythonInterp REQUIRED)
if(NOT PYTHONINTERP_FOUND)
    message(FATAL_ERROR "Cannot find python")
endif()

set(VC_DIR "")
if(MSVC)
    if(EXISTS "$ENV{VS100COMNTOOLS}../../VC")
       set(VC_DIR $ENV{VS100COMNTOOLS})
    endif()
    if(EXISTS "$ENV{VS110COMNTOOLS}../../VC")
       set(VC_DIR $ENV{VS110COMNTOOLS})
    endif()
    if(EXISTS "$ENV{VS120COMNTOOLS}../../VC")
       set(VC_DIR $ENV{VS120COMNTOOLS})
    endif()
    if(EXISTS "$ENV{VS130COMNTOOLS}../../VC")
       set(VC_DIR $ENV{VS130COMNTOOLS})
    endif()

    message(STATUS "VC_DIR='${VC_DIR}'")

    set(ENV{VS90COMNTOOLS} ${VC_DIR})
endif()

set(CURRENT_DIR bindings/${BINDING})

set(COPY_IF_DIFFERENT
    extensions/gdal_array_wrap.cpp
    extensions/gdalconst_wrap.c
    extensions/gdal_wrap.cpp
    extensions/gnm_wrap.cpp
    extensions/ogr_wrap.cpp
    extensions/osr_wrap.cpp
)

set(COPY
    README.txt
    osgeo/gdal_array.py
    osgeo/gdalconst.py
    osgeo/gdalnumeric.py
    osgeo/gdal.py
    osgeo/gnm.py
    osgeo/__init__.py
    osgeo/ogr.py
    osgeo/osr.py
    gdalconst.py
    gdalnumeric.py
    gdal.py
    gnm.py
    ogr.py
    osr.py
#    setup.py
)

add_custom_target(copy_files ALL
    COMMENT "Copy files"
)

foreach(FILE ${COPY_IF_DIFFERENT})
    #message(STATUS "copy ${GDAL_ROOT_SOURCE_DIR}/${CURRENT_DIR}/${FILE} to ${GDAL_ROOT_BINARY_DIR}/${CURRENT_DIR}/${FILE}")
    if(NOT EXISTS ${GDAL_ROOT_BINARY_DIR}/${CURRENT_DIR}/${FILE})
    add_custom_command(TARGET copy_files
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GDAL_ROOT_SOURCE_DIR}/${CURRENT_DIR}/${FILE}" "${GDAL_ROOT_BINARY_DIR}/${CURRENT_DIR}/${FILE}"
    )
    endif()
endforeach()


foreach(FILE ${COPY})
    #message(STATUS "copy ${GDAL_ROOT_SOURCE_DIR}/${CURRENT_DIR}/${FILE} to ${GDAL_ROOT_BINARY_DIR}/${CURRENT_DIR}/${FILE}")
    add_custom_command(TARGET copy_files
        COMMAND ${CMAKE_COMMAND} -E copy "${GDAL_ROOT_SOURCE_DIR}/${CURRENT_DIR}/${FILE}" "${GDAL_ROOT_BINARY_DIR}/${CURRENT_DIR}/${FILE}"
    )
endforeach()

add_custom_target(build ALL
    # copy some files to binary_dir
    COMMAND set VS90COMNTOOLS=${VC_DIR}
    COMMAND ${PYTHON_EXECUTABLE} ${GDAL_ROOT_BINARY_DIR}/${CURRENT_DIR}/setup.py build
    WORKING_DIRECTORY ${GDAL_ROOT_BINARY_DIR}/${CURRENT_DIR}
    DEPENDS copy_files
)


# todo: add custom target to generate swig
# todo: add install target $(PYTHON) setup.py install --prefix=$(DESTDIR)$(prefix)
# '../../port', '../../gcore', '../../alg', '../../ogr/', '../../ogr/ogrsf_frmts', '../../gnm'
set(SWIG_PYTHON_INCLUDE_DIRS "'${GDAL_ROOT_SOURCE_DIR}/core/alg', '${GDAL_ROOT_SOURCE_DIR}/core/port', '${GDAL_ROOT_SOURCE_DIR}/core/gcore', '${GDAL_ROOT_SOURCE_DIR}/core/gnm', '${GDAL_ROOT_SOURCE_DIR}/core/ogr', '${GDAL_ROOT_SOURCE_DIR}/core/ogr/ogrsf_frmts'")
if(GDAL_ENABLE_GNM)
    set(GDAL_HAVE_GNM "True")
else()
    set(GDAL_HAVE_GNM "False")
endif()

configure_file(${CMAKE_MODULE_PATH}/setup.py.cmake ${GDAL_ROOT_BINARY_DIR}/${CURRENT_DIR}/setup.py @ONLY)

install(CODE "execute_process(
            COMMAND \"${PYTHON_EXECUTABLE}\" \"${GDAL_ROOT_BINARY_DIR}/${CURRENT_DIR}/setup.py\" install
            WORKING_DIRECTORY  ${GDAL_ROOT_BINARY_DIR}/${CURRENT_DIR}
        )")
