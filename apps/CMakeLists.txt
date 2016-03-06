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

set(TARGET_NAME apps_lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/drivers/vector/geojson)
include_directories(${CMAKE_SOURCE_DIR}/core/ogr)


set(LIB_HHEADERS
)

set(LIB_HHEADERS_PRIV
    commonutils.h 
    gdal_utils.h 
    gdal_utils_priv.h 
)

set(LIB_CSOURCES
    commonutils.cpp
    gdalinfo_lib.cpp   
    gdalwarp_lib.cpp
    gdaldem_lib.cpp
    gdal_translate_lib.cpp
    nearblack_lib.cpp
    gdal_rasterize_lib.cpp
    ogr2ogr_lib.cpp
    gdalbuildvrt_lib.cpp
    gdal_grid_lib.cpp
)

add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

# build target
set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)

option(GDAL_BUILD_APPS "Build console applications" ON)
if(GDAL_BUILD_APPS)
    set(INSTALL_TARGETS)
    
    macro(ADD_EXE target sources)
        add_executable(${target} ${sources})
        add_dependencies(${target} ${LIB_NAME})
        target_link_libraries(${target} ${LIB_NAME} ${TARGET_LINK_LIB})
        set(INSTALL_TARGETS ${INSTALL_TARGETS} ${target})
    endmacro()
       
    add_exe(gdalinfo gdalinfo_bin.cpp) 
    add_exe(gdalwarp gdalwarp_bin.cpp) 
    add_exe(gdaldem gdaldem_bin.cpp) 
    add_exe(gdal_translate gdal_translate_bin.cpp) 
       
    add_exe(gdalmanage gdalmanage.cpp)
    add_exe(gdalenhance gdalenhance.cpp)
    add_exe(gdaladdo gdaladdo.cpp)
    add_exe(gdalsrsinfo gdalsrsinfo.cpp)

    add_exe(nearblack nearblack_bin.cpp) 
    add_exe(gdallocationinfo gdallocationinfo.cpp)
    add_exe(gdaltransform gdaltransform.cpp)
    add_exe(gdalserver gdalserver.c)



    if(GDAL_ENABLE_GNM)
        add_exe(gnmanalyse gnmanalyse.cpp)
        add_exe(gnmmanage gnmmanage.cpp)
    endif()

    if(GDAL_ENABLE_OGR)
        add_exe(gdal_contour gdal_contour.cpp)
        add_exe(gdaltindex gdaltindex.c)        
        add_exe(gdal_rasterize gdal_rasterize_bin.cpp)
        add_exe(ogrinfo ogrinfo.cpp)
        add_exe(ogrlineref ogrlineref.cpp)        
        add_exe(ogr2ogr ogr2ogr_bin.cpp) 
        add_exe(gdal_grid gdal_grid_bin.cpp)
        add_exe(ogrtindex ogrtindex.cpp)
        add_exe(testepsg testepsg.cpp)
        add_exe(gdalbuildvrt gdalbuildvrt_bin.cpp)
#        add_exe(ogrdissolve ogrdissolve.cpp)
    endif()

    option(GDAL_BUILD_OPT_APPS "Build optional console applications" OFF)
    if(GDAL_BUILD_OPT_APPS)
        add_exe(multireadtest multireadtest.cpp)
        add_exe(dumpoverviews dumpoverviews.cpp)
        add_exe(gdalwarpsimple gdalwarpsimple.c)
        add_exe(gdalflattenmask gdalflattenmask.c)
        add_exe(gdaltorture gdaltorture.cpp)
        add_exe(gdal2ogr gdal2ogr.c)
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