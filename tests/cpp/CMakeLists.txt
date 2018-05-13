################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Authors:   Dmitry Baryshnikov (polimax@mail.ru), Mikhail Gusev (gusevmihs@gmail.com)
################################################################################
# Copyright (C) 2015, NextGIS <info@nextgis.com>
# Copyright (C) 2012,2013,2014 Dmitry Baryshnikov
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tut)
include_directories(${PROJECT_SOURCE_DIR}/core/ogr/ogrsf_frmts)
include_directories(${PROJECT_SOURCE_DIR}/drivers/vector)

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

find_anyproject(OpenSSL REQUIRED)
find_anyproject(JBIG REQUIRED)
find_anyproject(SZIP REQUIRED)
set(TARGET_LINK_LIB ${TARGET_LINK_LIB} ${LIB_NAME})

macro(add_cpp_test_app name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} ${TARGET_LINK_LIB})
endmacro()

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-function")
    if(HARDEN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2 -fstack-protector-all -Wformat -Werror=format-security -ftrapv -fwrapv")
    endif()
endif()

set(PROGS testperfcopywords testcopywords testclosedondestroydm
    testthreadcond testvirtualmem testblockcache testblockcachewrite
    testblockcachelimits testdestroy testmultithreadedwriting
    test_c_include_from_cpp_file
)

if(CMAKE_CL_64 OR UNIX)
    set(PROGS ${PROGS}
        testsse
    )
endif()

foreach(PROG ${PROGS})
    add_cpp_test_app(${PROG})
endforeach()

add_executable(test_include_from_c_file test_include_from_c_file.c)
target_link_libraries(test_include_from_c_file ${TARGET_LINK_LIB})

add_executable(gdal_unit_test gdal_unit_test.cpp
    test_alg.cpp test_cpl.cpp test_gdal.cpp test_gdal_aaigrid.cpp
    test_gdal_dted.cpp test_gdal_gtiff.cpp test_triangulation.cpp
    test_ogr.cpp test_ogr_geos.cpp test_ogr_shape.cpp test_osr.cpp
test_osr_ct.cpp test_osr_pci.cpp test_osr_proj4.cpp tut/tut_gdal.cpp)
target_link_libraries(gdal_unit_test ${TARGET_LINK_LIB})

if(WIN32)
    file(GLOB_RECURSE IMPORTED_WIN_DLLS ${CMAKE_BINARY_DIR}/third-party/*.dll)
    foreach(IMPORTED_WIN_DLL ${IMPORTED_WIN_DLLS})
        add_custom_command(TARGET gdal_unit_test
                            POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E copy ${IMPORTED_WIN_DLL} $<TARGET_FILE_DIR:gdal_unit_test>
                            COMMENT "Copy ${IMPORTED_WIN_DLL} to gdal_unit_test destination"
                        )
    endforeach()
endif()

add_test(NAME quick_test COMMAND testperfcopywords)
if(APPLE)
    set_tests_properties(quick_test
        PROPERTIES ENVIRONMENT "DO_NOT_FAIL_ON_RECODE_ERRORS=TRUE")
    add_test(NAME gdal_unit_test COMMAND gdal_unit_test)
    set_tests_properties(gdal_unit_test
        PROPERTIES ENVIRONMENT "DO_NOT_FAIL_ON_RECODE_ERRORS=TRUE")
endif()
add_test(NAME testcopywords COMMAND testcopywords)
add_test(NAME testclosedondestroydm COMMAND testclosedondestroydm)
add_test(NAME testthreadcond COMMAND testthreadcond)
add_test(NAME testvirtualmem COMMAND testvirtualmem)
add_test(NAME testblockcache1 COMMAND testblockcache -check -co TILED=YES --debug TEST,LOCK -loops 3 --config GDAL_RB_LOCK_DEBUG_CONTENTION YES)
add_test(NAME testblockcache2 COMMAND testblockcache -check -co TILED=YES --debug TEST,LOCK -loops 3 --config GDAL_RB_LOCK_DEBUG_CONTENTION YES)
add_test(NAME testblockcache3 COMMAND testblockcache -check -co TILED=YES --debug TEST,LOCK -loops 3 --config GDAL_RB_LOCK_DEBUG_CONTENTION YES --config GDAL_RB_LOCK_TYPE SPIN)
add_test(NAME testblockcache4 COMMAND testblockcache -check -co TILED=YES -migrate)
add_test(NAME testblockcache5 COMMAND testblockcache -check -memdriver)
add_test(NAME testblockcache6 COMMAND testblockcachewrite --debug ON)
add_test(NAME testblockcache7 COMMAND testblockcache --config GDAL_BAND_BLOCK_CACHE HASHSET -check -co TILED=YES --debug TEST,LOCK -loops 3 --config GDAL_RB_LOCK_DEBUG_CONTENTION YES)
add_test(NAME testblockcache8 COMMAND testblockcache --config GDAL_BAND_BLOCK_CACHE HASHSET -check -co TILED=YES --debug TEST,LOCK -loops 3 --config GDAL_RB_LOCK_DEBUG_CONTENTION YES --config GDAL_RB_LOCK_TYPE SPIN)
add_test(NAME testblockcachelimits COMMAND testblockcachelimits --debug ON)
add_test(NAME testmultithreadedwriting COMMAND testmultithreadedwriting)
add_test(NAME testdestroy COMMAND testdestroy)
if(CMAKE_CL_64 OR UNIX)
    add_test(NAME testsse COMMAND testsse)
endif()
