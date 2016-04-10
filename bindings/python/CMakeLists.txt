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
    setup.cfg
#    setup.py
)

set(PY_COMMANDS
    scripts/epsg_tr.py 
    scripts/gdalchksum.py 
    scripts/gdal2xyz.py 
    scripts/gcps2wld.py
    scripts/gdalimport.py 
    scripts/gdal_merge.py 
    scripts/pct2rgb.py 
    scripts/rgb2pct.py
    scripts/gcps2vec.py
)

add_custom_target(copy_files ALL
    COMMENT "Copy files"
)

foreach(FILE ${COPY_IF_DIFFERENT})
    #message(STATUS "copy ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} to ${CMAKE_CURRENT_BINARY_DIR}/${FILE}")
    if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${FILE})
    add_custom_command(TARGET copy_files
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}" "${CMAKE_CURRENT_BINARY_DIR}/${FILE}"
    )
    endif()
endforeach()


foreach(FILE ${COPY})
    #message(STATUS "copy ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} to ${CMAKE_CURRENT_BINARY_DIR}/${FILE}")
    add_custom_command(TARGET copy_files
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}" "${CMAKE_CURRENT_BINARY_DIR}/${FILE}"
    )
endforeach()

if(MSVC)
    if(CMAKE_CL_64)
        set(PY_SETUP_PLATFORM_NAME --plat-name=win-amd64)
    else()
        set(PY_SETUP_PLATFORM_NAME --plat-name=win32)
    endif()
endif()

add_custom_target(build ALL
    # copy some files to binary_dir
    COMMAND set VS90COMNTOOLS=${VC_DIR}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/setup.py build ${PY_SETUP_PLATFORM_NAME}

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS copy_files
)

add_dependencies(build ${LIB_NAME})

add_custom_target(egg
    # copy some files to binary_dir
    COMMAND set VS90COMNTOOLS=${VC_DIR}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/setup.py bdist_egg
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS copy_files
)

# todo: add custom target to generate swig
# '../../port', '../../gcore', '../../alg', '../../ogr/', '../../ogr/ogrsf_frmts', '../../gnm'
if(MSVC)
    set(SWIG_PYTHON_LIBRARIES "'${LIB_NAME}_i'")
else()
    set(SWIG_PYTHON_LIBRARIES "'${LIB_NAME}'")
endif()
    
set(SWIG_PYTHON_LIBRARY_DIRS "'../../Release', '../../Debug', '../../MinSizeRel', '../../RelWithDebInfo', '../../.libs', '../../'")

set(SWIG_PYTHON_INCLUDE_DIRS "'${CMAKE_SOURCE_DIR}/core/alg', '${CMAKE_SOURCE_DIR}/core/port', '${CMAKE_SOURCE_DIR}/core/gcore', '${CMAKE_SOURCE_DIR}/core/gnm', '${CMAKE_SOURCE_DIR}/core/ogr', '${CMAKE_SOURCE_DIR}/core/ogr/ogrsf_frmts', '${CMAKE_SOURCE_DIR}/apps', '${CMAKE_BINARY_DIR}'")
if(GDAL_ENABLE_GNM)
    set(GDAL_HAVE_GNM "True")
else()
    set(GDAL_HAVE_GNM "False")
endif()

if(GDAL_ENABLE_OGR)
    set(GDAL_HAVE_OGR "True")
else()
    set(GDAL_HAVE_OGR "False")
endif()

configure_file(${CMAKE_MODULE_PATH}/setup.py.cmake ${CMAKE_CURRENT_BINARY_DIR}/setup.py @ONLY)

# https://docs.python.org/2/install/
# https://docs.python.org/3/install/
# https://pythonhosted.org/an_example_pypi_project/setuptools.html
if(UNIX)
    execute_process ( COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print get_python_lib()" OUTPUT_VARIABLE PY_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(TEST_CMD "try: 
    import setuptools
    print(1)
except ImportError: 
    pass")

    execute_process ( COMMAND ${PYTHON_EXECUTABLE} -c ${TEST_CMD} OUTPUT_VARIABLE PY_HAVE_SETUPTOOLS OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(SETUP_OPTS)

    if(PY_HAVE_SETUPTOOLS)
        set(SETUP_OPTS "${SETUP_OPTS} --single-version-externally-managed --record=rekord.txt")
    endif()
    
    string( LENGTH "$ENV{DESTDIR}" DESTDIRLEN )
    if ( ${DESTDIRLEN} GREATER 0 )
        set(SETUP_OPTS "${SETUP_OPTS} --root=$ENV{DESTDIR}")
    elseif(PYTHON_DESTDIR)    
        set(SETUP_OPTS "${SETUP_OPTS} --root=${PYTHON_DESTDIR}")
    endif()

    # check debian
    string(FIND ${PY_SITE_PACKAGES} "dist-packages" PY_DIST_POS)
    if(${PY_DIST_POS} GREATER -1)
        set(SETUP_OPTS "${SETUP_OPTS} --install-layout=deb")
    endif()

    # do we need --install-lib=$(PYTHON_INSTALL_LIB) ?

    set(SETUP_OPTS "${SETUP_OPTS} --prefix=${CMAKE_INSTALL_PREFIX}")
    
    if(NOT SUPRESS_VERBOSE_OUTPUT)
        message(STATUS "setup tools options: ${SETUP_OPTS}")
    endif()

    install(CODE "execute_process(
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/setup.py install ${SETUP_OPTS}  
            WORKING_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}
            )")
else()
    file(GLOB TESTS ${CMAKE_CURRENT_BINARY_DIR}/build/lib.*)
    foreach(TEST ${TESTS})
        install(DIRECTORY ${TEST}/
                DESTINATION ${INSTALL_LIB_DIR}/Python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}/site-packages
                COMPONENT libraries)
    endforeach()
endif()

#install scripts
install(PROGRAMS ${PY_COMMANDS} DESTINATION ${INSTALL_BIN_DIR} COMPONENT applications)        

#install examples        
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/ 
        DESTINATION ${INSTALL_DOC_DIR}/python-samples 
        COMPONENT documents)
