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
    if(EXISTS "$ENV{VS140COMNTOOLS}../../VC")
       set(VC_DIR $ENV{VS140COMNTOOLS})
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
    scripts/esri2wkt.py
    scripts/gdalchksum.py
    scripts/gdal2xyz.py
    scripts/gcps2wld.py
    scripts/gdalimport.py
    scripts/gdal_merge.py
    scripts/pct2rgb.py
    scripts/rgb2pct.py
    scripts/gcps2vec.py
    scripts/gdal_retile.py
    scripts/gdal_sieve.py
    scripts/gdal2tiles.py
    scripts/gdal_fillnodata.py
    scripts/gdal_polygonize.py
    scripts/gdal_proximity.py
    scripts/gdalident.py
    scripts/mkgraticule.py
    scripts/gdal_calc.py
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
    add_custom_target(build_py${ver} ALL
        # copy some files to binary_dir
        COMMAND set VS90COMNTOOLS=${VC_DIR}
        COMMAND ${PYTHON${ver}_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/setup.py build ${PY_SETUP_PLATFORM_NAME}

        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS copy_files
    )
    add_dependencies(build_py${ver} ${LIB_NAME})
    add_custom_target(egg_py${ver}
        # copy some files to binary_dir
        COMMAND set VS90COMNTOOLS=${VC_DIR}
        COMMAND ${PYTHON${ver}_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/setup.py bdist_egg
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS copy_files
    )
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

configure_file(${CMAKE_SOURCE_DIR}/cmake/setup.py.cmake ${CMAKE_CURRENT_BINARY_DIR}/setup.py @ONLY)

# https://docs.python.org/2/install/
# https://docs.python.org/3/install/
# https://pythonhosted.org/an_example_pypi_project/setuptools.html
if(UNIX)
    macro(install_py ver)
        execute_process ( COMMAND ${PYTHON${ver}_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())" OUTPUT_VARIABLE PY_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

        set(TEST_CMD "try: 
    import setuptools
    print(1)
except ImportError: 
    pass")

        execute_process ( COMMAND ${PYTHON${ver}_EXECUTABLE} -c ${TEST_CMD} OUTPUT_VARIABLE PY_HAVE_SETUPTOOLS OUTPUT_STRIP_TRAILING_WHITESPACE)

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

        # TODO: do we need --install-lib=$(PYTHON_INSTALL_LIB) ?

        set(SETUP_OPTS "${SETUP_OPTS} --prefix=${CMAKE_INSTALL_PREFIX}")
        
        if(NOT SUPPRESS_VERBOSE_OUTPUT)
            message(STATUS "setup tools options: ${SETUP_OPTS}")
        endif()

        install(CODE "execute_process(
                COMMAND ${PYTHON${ver}_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/setup.py install ${SETUP_OPTS}  
                WORKING_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}
                )")
    endmacro()
    
    if(PYTHON2_EXECUTABLE)
        install_py(2)
    endif()

    if(PYTHON3_EXECUTABLE)
        install_py(3)
    endif()                     
else()
    macro(install_win_py ver)
        set(BINDING_PATH ${CMAKE_CURRENT_BINARY_DIR}/build/lib.${PY_PLATFORM_NAME}-${PYTHON${ver}_VERSION_MAJOR}.${PYTHON${ver}_VERSION_MINOR})                  
        install(DIRECTORY "${BINDING_PATH}/"
                DESTINATION ${INSTALL_LIB_DIR}/Python${PYTHON${ver}_VERSION_MAJOR}${PYTHON${ver}_VERSION_MINOR}/site-packages
                COMPONENT libraries)
    endmacro()
    
    if(PYTHON2_EXECUTABLE)
        install_win_py(2)
    endif()

    if(PYTHON3_EXECUTABLE)
        install_win_py(3)
    endif()
endif()

#install scripts
install(PROGRAMS ${PY_COMMANDS} DESTINATION ${INSTALL_BIN_DIR} COMPONENT applications)        

#install examples        
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/samples/ 
        DESTINATION ${INSTALL_DOC_DIR}/python-samples 
        COMPONENT documents)
