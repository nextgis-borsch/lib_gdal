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

option(GDAL_BUILD_APPS "Build console applications" ON)
if(GDAL_BUILD_APPS)
    set(INSTALL_TARGETS)
    
    macro(ADD_EXE target sources)
        add_executable(${target} ${sources})
        target_link_libraries(${target} ${LIB_NAME})
        set(INSTALL_TARGETS ${INSTALL_TARGETS} ${target})
    endmacro()
    
    macro(ADD_EXE_BIN target sources lib_target lib_sources)
        add_library(${lib_target} OBJECT ${lib_sources})
        set(ESOURCES ${sources} $<TARGET_OBJECTS:${lib_target}>)
        add_exe(${target} ${ESOURCES})
    endmacro()
    
    add_exe_bin(gdalinfo gdalinfo_bin.cpp gdalinfo_lib gdalinfo_lib.cpp) 
    add_exe_bin(gdalwarp gdalwarp_bin.cpp gdalwarp_lib gdalwarp_lib.cpp) 
    add_exe_bin(gdaldem gdaldem_bin.cpp gdaldem_lib gdaldem_lib.cpp) 
    add_exe_bin(gdal_translate gdal_translate_bin.cpp gdal_translate_lib gdal_translate_lib.cpp) 
       
    add_exe(gdalmanage gdalmanage.cpp)
    add_exe(gdalenhance gdalenhance.cpp)
    add_exe(gdaladdo gdaladdo.cpp)
    add_exe(gdalsrsinfo gdalsrsinfo.cpp)

    add_exe_bin(nearblack nearblack_bin.cpp nearblack_lib nearblack_lib.cpp) 
    add_exe(gdallocationinfo gdallocationinfo.cpp)
    add_exe(gdaltransform gdaltransform.cpp)
    add_exe(gdalserver gdalserver.c)

#commonutils.cpp commonutils.h gdal_utils.h gdal_utils_priv.h

    if(GDAL_ENABLE_GNM)
        add_exe(gnmanalyse gnmanalyse.cpp)
        add_exe(gnmmanage gnmmanage.cpp)
    endif()

    if(GDAL_ENABLE_OGR)
        add_exe(gdal_contour gdal_contour.cpp)
        add_exe(gdaltindex gdaltindex.c)        
        add_exe_bin(gdal_rasterize gdal_rasterize_bin.cpp gdal_rasterize_lib gdal_rasterize_lib.cpp)
        add_exe(ogrinfo ogrinfo.cpp)
        add_exe(ogrlineref ogrlineref.cpp)        
        add_exe_bin(ogr2ogr ogr2ogr_bin.cpp ogr2ogr_lib ogr2ogr_lib.cpp)        
        add_exe_bin(gdal_grid gdal_grid_bin.cpp gdal_grid_lib gdal_grid_lib.cpp)
        add_exe(ogrtindex ogrtindex.cpp)
        add_exe(testepsg testepsg.cpp)
        add_exe_bin(gdalbuildvrt gdalbuildvrt_bin.cpp gdalbuildvrt_lib gdalbuildvrt_lib.cpp)
        add_exe(ogrdissolve ogrdissolve.cpp)
    endif()

    option(GDAL_BUILD_OPT_APPS "Build optional console applications" OFF)
    if(GDAL_BUILD_OPT_APPS)
        add_exe(multireadtest multireadtest.cpp)
        add_exe(dumpoverviews dumpoverviews.cpp)
        add_exe(gdalwarpsimple gdalwarpsimple.c)
        add_exe(gdalflattenmask gdalflattenmask.c)
        add_exe(gdaltorture gdaltorture.cpp)
        add_exe(gdal2ogr gdal2ogr.cpp)
        add_exe(test_ogrsf test_ogrsf.cpp)
        add_exe(gdalasyncread gdalasyncread.cpp)
        add_exe(testreprojmulti testreprojmulti.cpp)
    endif()
    
    configure_file(${CMAKE_MODULE_PATH}/gdal-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/gdal-config IMMEDIATE @ONLY)
    configure_file(${CMAKE_MODULE_PATH}/gdal-config-inst.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/gdal-config-inst IMMEDIATE @ONLY)

    if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
        install(TARGETS ${INSTALL_TARGETS}
        #    EXPORT ${INSTALL_TARGETS}
            RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT applications
            ARCHIVE DESTINATION ${INSTALL_LIB_DIR} COMPONENT applications
            LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT applications
        )
    endif()

endif()
