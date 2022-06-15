################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Authors:   Dmitry Baryshnikov (polimax@mail.ru), Mikhail Gusev (gusevmihs@gmail.com)
################################################################################
# Copyright (C) 2015-2019, NextGIS <info@nextgis.com>
# Copyright (C) 2012-2019 Dmitry Baryshnikov
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

option(BUILD_TESTING "Build GDAL Unit Testing" OFF)
if(BUILD_TESTING)

    file(COPY gcore/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/gcore)
    # file(COPY alg/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/alg)
    file(COPY gdrivers/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/gdrivers)
    ##file(COPY ogr/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ogr)
    # file(COPY osr/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/osr)
    # file(COPY utilities/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/utilities)
    # file(COPY utilities/ref_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/utilities)

    list (FIND GDAL_BINDINGS "python" _index)
    if (${_index} GREATER -1 AND NOT SKIP_PYTHON_TESTS)
        # Find Python.
        find_package(PythonInterp REQUIRED)
        if(NOT PYTHONINTERP_FOUND)
            message(FATAL_ERROR "Cannot find python. Python required for testing")
        endif()

        # Create separate directory into which we will put generated python libraries (because each OS will have unique
        # directory name).
        # set(TEST_LIB_GDAL_DIRECTORY ${CMAKE_BINARY_DIR}/Testing-lib_gdal)
        # add_custom_command(TARGET GDAL_EP
        #     POST_BUILD
        #     COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_LIB_GDAL_DIRECTORY}
        #     COMMAND ${CMAKE_COMMAND} -E copy_directory
        #         ${CMAKE_BINARY_DIR}/third-party/Install/GDAL_EP/lib/Python27/site-packages/.
        #         ${TEST_LIB_GDAL_DIRECTORY}/.
        #     COMMENT "Copying GDAL python libraries to ${TEST_LIB_GDAL_DIRECTORY}")
        # if(WIN32)
        #     # For windows copy GDAL dlls to the python modules' directory.
        #     add_custom_command(TARGET GDAL_EP
        #         POST_BUILD
        #         COMMAND ${CMAKE_COMMAND} -E copy_directory
        #             ${CMAKE_BINARY_DIR}/third-party/Install/GDAL_EP/bin/.
        #             ${TEST_LIB_GDAL_DIRECTORY}/osgeo/.
        #         COMMENT "Copying GDAL dlls to ${TEST_LIB_GDAL_DIRECTORY}")
        # endif()

        # Finally we can do tests for each python test dir, adding this path for finding python modules
        # in order not to touch the system GDAL if it exists.
        macro(do_gdal_test name)
            add_test(NAME ${name} COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_all.py ${name})
            # set_property(TEST ${name} PROPERTY ENVIRONMENT "PYTHONPATH=${TEST_LIB_GDAL_DIRECTORY}") # reset PYTHONPATH
        endmacro()

        # TODO: customize CTest so it can correctly define tests' results.
        # https://cmake.org/Wiki/CMake/Testing_With_CTest

        # Run all python tests.
        do_gdal_test(gcore)
        do_gdal_test(gdrivers)
        do_gdal_test(alg)
        do_gdal_test(osr)
        do_gdal_test(ogr)
        do_gdal_test(utilities)
        do_gdal_test(pymod)
        do_gdal_test(pyscripts)
        do_gdal_test(gnm)

    endif()

    add_subdirectory(cpp)

endif()
