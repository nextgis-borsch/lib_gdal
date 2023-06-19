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

    if (UNIX)
        set(SKIP_PYTHON_TESTS OFF)
    endif ()
    
      function (symlink_or_copy source destination)
        if (CMAKE_VERSION VERSION_GREATER 3.14)
          file(
            CREATE_LINK ${source} ${destination}
            RESULT res
            SYMBOLIC)
          if (NOT res EQUAL 0)
            message(STATUS "Copying content of ${source} to ${destination}")
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${source} ${destination})
          endif ()
        else ()
          if (NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
            execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination})
          else ()
            message(STATUS "Copying content of ${source} to ${destination}")
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${source} ${destination})
          endif ()
        endif ()
      endfunction ()
    
    set(PYTHON_ENV "LD_LIBRARY_PATH=$<SHELL_PATH:$<TARGET_FILE_DIR:${LIB_NAME}>>"
                   "PYTHONPATH=${PROJECT_BINARY_DIR}/bindings/python/inst_tmp/lib/python3/dist-packages")

	if (NOT SKIP_PYTHON_TESTS)
		if(NOT PYTHON3_EXECUTABLE)
			message(FATAL_ERROR "Cannot find python")
		endif()
        
        configure_file(${CMAKE_SOURCE_DIR}/cmake/pytest.ini.in ${CMAKE_CURRENT_BINARY_DIR}/pytest.ini @ONLY)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/conftest.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        
        if (NOT "${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
          foreach (subdir IN ITEMS pymod proj_grids cpp/data)
              if (SKIP_COPYING_AUTOTEST_SUBDIRS)
                message(STATUS "Skipping copying ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}")
              else ()
                symlink_or_copy(${CMAKE_CURRENT_SOURCE_DIR}/${subdir} ${CMAKE_CURRENT_BINARY_DIR}/${subdir})
              endif ()
          endforeach ()
        endif()
        
        foreach (tgt IN ITEMS
                  ogr
                  gcore
                  gdrivers
                  alg
                  osr
                  gnm
                  pyscripts
                  utilities)

            symlink_or_copy(${CMAKE_CURRENT_SOURCE_DIR}/${tgt} ${CMAKE_CURRENT_BINARY_DIR}/${tgt})

            add_test(NAME autotest_${tgt} COMMAND ${CMAKE_COMMAND} -E env ${PYTHON_ENV} ${PYTHON3_EXECUTABLE} -m pytest -c ${CMAKE_CURRENT_BINARY_DIR}/pytest.ini ${tgt})

            set_property(TEST autotest_${tgt} PROPERTY ENVIRONMENT "${PYTHON_RUN_ENV}")
        endforeach ()

    else()
        foreach (tgt IN ITEMS
              gcore
              gdrivers)
            symlink_or_copy(${CMAKE_CURRENT_SOURCE_DIR}/${tgt} ${CMAKE_CURRENT_BINARY_DIR}/${tgt})
        endforeach ()
    endif()

    add_subdirectory(cpp)

endif()
