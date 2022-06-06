################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015-2020, NextGIS <info@nextgis.com>
# Copyright (C) 2012-2020 Dmitry Baryshnikov
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

# Need because INSTALL_LIB_DIR points to /usr/lib/x86_64-linux-gnu/python2.7 and should to /usr/lib/python2.7
if(OSX_FRAMEWORK)
    set(INSTALL_PYTHON_DIR "Library" CACHE INTERNAL "Installation directory for python bindings" FORCE)
else()
    set(INSTALL_PYTHON_DIR "lib" CACHE INTERNAL "Installation directory for python bindings" FORCE)
endif()

if(WITH_PYTHON2)
    find_package(PythonInterp 2)
    if(PYTHONINTERP_FOUND)
        set(PYTHON2_EXECUTABLE ${PYTHON_EXECUTABLE})
        set(PYTHON2_VERSION_MAJOR ${PYTHON_VERSION_MAJOR})
        set(PYTHON2_VERSION_MINOR ${PYTHON_VERSION_MINOR})
    else()
        message(WARNING "Cannot find python 2")
    endif()
endif()

unset(PYTHONINTERP_FOUND)
unset(PYTHON_EXECUTABLE)
unset(PYTHON_VERSION_MAJOR)
unset(PYTHON_VERSION_MINOR)

if(WITH_PYTHON3)
    # Hack, not find python3 withouut this
    find_program(PYTHON3_EXECUTABLE python3)
    set(PYTHON_EXECUTABLE ${PYTHON3_EXECUTABLE})

    find_package(PythonInterp 3)
    if(PYTHONINTERP_FOUND)
        set(PYTHON3_EXECUTABLE ${PYTHON_EXECUTABLE})
        set(PYTHON3_VERSION_MAJOR ${PYTHON_VERSION_MAJOR})
        set(PYTHON3_VERSION_MINOR ${PYTHON_VERSION_MINOR})
    else()
        message(WARNING "Cannot find python 3")
    endif()
endif()

if(NOT PYTHON2_EXECUTABLE AND NOT PYTHON3_EXECUTABLE)
    message(FATAL_ERROR "Cannot find python")
endif()

if(OSX_FRAMEWORK)
    set(FLAGS "ARCHFLAGS=-arch x86_64")
elseif(WIN32)
    if(MSVC_VERSION EQUAL 1400)
        set(FLAGS ${FLAGS} "VS90COMNTOOLS=$ENV{VS80COMNTOOLS}")
    elseif(MSVC_VERSION EQUAL 1500)
        set(FLAGS ${FLAGS} "VS90COMNTOOLS=$ENV{VS90COMNTOOLS}")
    elseif(MSVC_VERSION EQUAL 1600)
        set(FLAGS ${FLAGS} "VS90COMNTOOLS=$ENV{VS100COMNTOOLS}")
    elseif(MSVC_VERSION EQUAL 1700)
        set(FLAGS ${FLAGS} "VS90COMNTOOLS=$ENV{VS110COMNTOOLS}")
    elseif(MSVC_VERSION EQUAL 1800)
        set(FLAGS ${FLAGS} "VS90COMNTOOLS=$ENV{VS120COMNTOOLS}")
    elseif(MSVC_VERSION EQUAL 1900)
        set(FLAGS ${FLAGS} "VS90COMNTOOLS=$ENV{VS140COMNTOOLS}")
    else() # 1912 - Need to copy vcvarsall.bat from VC\Auxiliary\Build to VC
        get_filename_component(_COMNTOOLS_PREFIX "${CMAKE_C_COMPILER}" PATH)
        get_filename_component(_COMNTOOLS_PREFIX "${_COMNTOOLS_PREFIX}" PATH)
        get_filename_component(_COMNTOOLS_PREFIX "${_COMNTOOLS_PREFIX}" PATH)
        get_filename_component(_COMNTOOLS_PREFIX "${_COMNTOOLS_PREFIX}" PATH)
        get_filename_component(_COMNTOOLS_PREFIX "${_COMNTOOLS_PREFIX}" PATH)
        get_filename_component(_COMNTOOLS_PREFIX "${_COMNTOOLS_PREFIX}" PATH)
        # get_filename_component(_COMNTOOLS_PREFIX "${_COMNTOOLS_PREFIX}" PATH)
        set(FLAGS ${FLAGS} "VS90COMNTOOLS=${_COMNTOOLS_PREFIX}")
    endif()
endif()

find_anyproject(NUMPY DEFAULT OFF)

set(COPY_IF_DIFFERENT
    extensions/gdal_array_wrap.cpp
    extensions/gdalconst_wrap.c
    extensions/gdal_wrap.cpp
    extensions/gnm_wrap.cpp
    extensions/ogr_wrap.cpp
    extensions/osr_wrap.cpp
)

set(COPY
    README.rst
    osgeo/gdal_array.py
    osgeo/gdalconst.py
    osgeo/gdalnumeric.py
    osgeo/gdal.py
    osgeo/gnm.py
    osgeo/__init__.py
    osgeo/ogr.py
    osgeo/osr.py
    osgeo/utils/__init__.py
    osgeo/utils/epsg_tr.py
    osgeo/utils/esri2wkt.py
    osgeo/utils/gcps2vec.py
    osgeo/utils/gcps2wld.py
    osgeo/utils/gdal_auth.py
    osgeo/utils/gdal_calc.py
    osgeo/utils/gdal_edit.py
    osgeo/utils/gdal_fillnodata.py
    osgeo/utils/gdal_merge.py
    osgeo/utils/gdal_pansharpen.py
    osgeo/utils/gdal_polygonize.py
    osgeo/utils/gdal_proximity.py
    osgeo/utils/gdal_retile.py
    osgeo/utils/gdal_sieve.py
    osgeo/utils/gdal2xyz.py
    osgeo/utils/gdalchksum.py
    osgeo/utils/gdalcompare.py
    osgeo/utils/gdalident.py
    osgeo/utils/gdalimport.py
    osgeo/utils/gdalmove.py
    osgeo/utils/mkgraticule.py
    osgeo/utils/ogrmerge.py
    osgeo/utils/pct2rgb.py
    osgeo/utils/rgb2pct.py
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
    scripts/esri2wkt.py
    scripts/gcps2vec.py
    scripts/gcps2wld.py
    scripts/gdal2tiles.py
    scripts/gdal2xyz.py
    scripts/gdal_auth.py
    scripts/gdal_calc.py
    scripts/gdal_edit.py
    scripts/gdal_fillnodata.py
    scripts/gdal_merge.py
    scripts/gdal_pansharpen.py
    scripts/gdal_polygonize.py
    scripts/gdal_proximity.py
    scripts/gdal_retile.py
    scripts/gdal_sieve.py
    scripts/gdalchksum.py
    scripts/gdalcompare.py
    scripts/gdalident.py
    scripts/gdalimport.py
    scripts/gdalmove.py
    scripts/mkgraticule.py
    scripts/ogrmerge.py
    scripts/pct2rgb.py
    scripts/rgb2pct.py
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
        set(PY_PLATFORM_NAME win-amd64)
    else()
        set(PY_PLATFORM_NAME win32)
    endif()
    set(PY_SETUP_PLATFORM_NAME --plat-name=${PY_PLATFORM_NAME})
endif()

macro(build_py ver)

    if(UNIX AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        add_custom_target(build_py${ver} ALL
            COMMAND ${PYTHON${ver}_EXECUTABLE} setup.py build # ${BUILD_INCLUDES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS copy_files
            VERBATIM
        )
    else()
        add_custom_target(build_py${ver} ALL
            COMMAND ${CMAKE_COMMAND} -E env "${FLAGS}"
            ${PYTHON${ver}_EXECUTABLE} setup.py build # ${BUILD_INCLUDES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS copy_files
            VERBATIM
        )
    endif()

    add_dependencies(build_py${ver} ${LIB_NAME})

    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(SETUP_OPTS ${SETUP_OPTS} --no-compile --single-version-externally-managed --record=rekord.txt --prefix=${CMAKE_CURRENT_BINARY_DIR}/inst_tmp)
    elseif(WIN32)
        string(REPLACE "/" "\\\\" TMP_PATH "${CMAKE_CURRENT_BINARY_DIR}/inst_tmp")
        set(SETUP_OPTS ${SETUP_OPTS} --no-compile --single-version-externally-managed --record=rekord.txt --prefix=${TMP_PATH})
    else()
        execute_process ( COMMAND ${PYTHON${ver}_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())" OUTPUT_VARIABLE PY_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

        set(TEST_CMD "try:
    import setuptools
    print(1)
except ImportError:
    pass")

        execute_process ( COMMAND ${PYTHON${ver}_EXECUTABLE} -c ${TEST_CMD} OUTPUT_VARIABLE PY_HAVE_SETUPTOOLS OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(PY_HAVE_SETUPTOOLS)
            set(SETUP_OPTS ${SETUP_OPTS} --no-compile --single-version-externally-managed --record=rekord.txt)
        endif()

        # string( LENGTH "$ENV{DESTDIR}" DESTDIRLEN )
        # if ( ${DESTDIRLEN} GREATER 0 )
        #     set(SETUP_OPTS ${SETUP_OPTS} --root=$ENV{DESTDIR})
        # elseif(PYTHON_DESTDIR)
        #     set(SETUP_OPTS ${SETUP_OPTS} --root=${PYTHON_DESTDIR})
        # endif()

        # check debian
        string(FIND ${PY_SITE_PACKAGES} "dist-packages" PY_DIST_POS)
        if(${PY_DIST_POS} GREATER -1)
            set(SETUP_OPTS ${SETUP_OPTS} --install-layout=deb)
        endif()

        # TODO: do we need --install-lib=$(PYTHON_INSTALL_LIB) ?

        set(SETUP_OPTS ${SETUP_OPTS} --prefix=${CMAKE_CURRENT_BINARY_DIR}/inst_tmp)

    endif()
    if(UNIX AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        add_custom_target(install_py${ver} ALL
            COMMAND ${PYTHON${ver}_EXECUTABLE} setup.py install ${SETUP_OPTS}
            DEPENDS build_py${ver}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM
        )
    else()
        add_custom_target(install_py${ver} ALL
            COMMAND ${CMAKE_COMMAND} -E env "${FLAGS}"
            ${PYTHON${ver}_EXECUTABLE} setup.py install ${SETUP_OPTS}
            DEPENDS build_py${ver}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM
        )
    endif()
endmacro()

if(PYTHON2_EXECUTABLE)
    build_py(2)
endif()

if(PYTHON3_EXECUTABLE)
    build_py(3)
endif()

# todo: add custom target to generate swig
# '../../port', '../../gcore', '../../alg', '../../ogr/', '../../ogr/ogrsf_frmts', '../../gnm'
if(MSVC)
    set(SWIG_PYTHON_LIBRARIES "'${LIB_NAME}_i'")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SWIG_PYTHON_FRAMEWORK "${LIB_NAME}")
    set(SWIG_PYTHON_FRAMEWORK_DIRS "${CMAKE_BINARY_DIR}")
else()
    set(SWIG_PYTHON_LIBRARIES "'${LIB_NAME}'")
endif()

set(SWIG_PYTHON_LIBRARY_DIRS "'${CMAKE_BINARY_DIR}'")
set(FILL_SWIG_PATHS
    "Release"
    "Debug"
    "MinSizeRel"
    "RelWithDebInfo"
    ".libs"
)

macro(fill_swig_paths LIB_PATH)
    if(EXISTS "${CMAKE_BINARY_DIR}/${LIB_PATH}")
        set(SWIG_PYTHON_LIBRARY_DIRS "${SWIG_PYTHON_LIBRARY_DIRS}, '../../${LIB_PATH}'")
    endif()
endmacro()

foreach(swig_path ${FILL_SWIG_PATHS})
    fill_swig_paths(${swig_path})
endforeach()

set(SWIG_PYTHON_LIBRARY_DIRS "${SWIG_PYTHON_LIBRARY_DIRS}, '../../${CMAKE_BUILD_TYPE}', '../../lib/${CMAKE_BUILD_TYPE}'")

set(SWIG_PYTHON_INCLUDE_DIRS "'${CMAKE_SOURCE_DIR}/core/alg', '${CMAKE_SOURCE_DIR}/core/port', '${CMAKE_SOURCE_DIR}/core/gcore', '${CMAKE_BINARY_DIR}/core/gcore', '${CMAKE_SOURCE_DIR}/core/gnm', '${CMAKE_SOURCE_DIR}/core/ogr', '${CMAKE_SOURCE_DIR}/core/ogr/ogrsf_frmts', '${CMAKE_SOURCE_DIR}/apps', '${CMAKE_BINARY_DIR}'")
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

configure_file(${CMAKE_SOURCE_DIR}/cmake/setup.py.cmake ${CMAKE_CURRENT_BINARY_DIR}/setup.py @ONLY)

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/inst_tmp/lib/ DESTINATION ${INSTALL_PYTHON_DIR})
endif()

if(NOT SKIP_INSTALL_EXECUTABLES AND NOT SKIP_INSTALL_ALL )
    #install scripts
    install(PROGRAMS ${PY_COMMANDS} DESTINATION ${INSTALL_BIN_DIR} COMPONENT applications)

    #install examples
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/
            DESTINATION ${INSTALL_DOC_DIR}/python-samples
            COMPONENT documents)
endif()