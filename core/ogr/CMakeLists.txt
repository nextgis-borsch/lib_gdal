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

set(LIB_NAME "ogr")
project ("lib${LIB_NAME}")

option(GDAL_ENABLE_OGR "Build OGR module" ON)
if(GDAL_ENABLE_OGR)
    add_definitions(-DOGR_ENABLED)
endif()

add_definitions(-DGDAL_COMPILATION)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../port)
include_directories(${GDAL_ROOT_BINARY_DIR}/port)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gcore)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ogrsf_frmts)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ogrsf_frmts/mem)

set(LIB_INST_HEADERS
    ogr_api.h
    ogr_core.h
    ogr_feature.h
    ogr_featurestyle.h
    ogr_geometry.h
    ogr_p.h
    ogr_spatialref.h
    ogr_srs_api.h
    ogr_geocoding.h
)

set(LIB_HHEADERS
    ograpispy.h
    ogrgeomediageometry.h
    ogrpgeogeometry.h
    ogr_srs_esri_names.h
    osr_cs_wkt.h
    osr_cs_wkt_parser.h
    swq.h
)

set(LIB_CSOURCES
#    generate_encoding_table.c do we need special target for this
    gml2ogrgeometry.cpp
    ogr2gmlgeometry.cpp
    ogr_api.cpp
    ograpispy.cpp
    ograssemblepolygon.cpp
    ogrcircularstring.cpp
    ogrcompoundcurve.cpp
    ogrct.cpp
    ogrcurvecollection.cpp
    ogrcurve.cpp
    ogrcurvepolygon.cpp
    ogrfeature.cpp
    ogrfeaturedefn.cpp
    ogrfeaturequery.cpp
    ogrfeaturestyle.cpp
    ogrfielddefn.cpp
    ogr_fromepsg.cpp
    ogr_geocoding.cpp
    ogrgeomediageometry.cpp
    ogrgeometrycollection.cpp
    ogrgeometry.cpp
    ogrgeometryfactory.cpp
    ogrgeomfielddefn.cpp
    ogrlinearring.cpp
    ogrlinestring.cpp
    ogrmulticurve.cpp
    ogrmultilinestring.cpp
    ogrmultipoint.cpp
    ogrmultipolygon.cpp
    ogrmultisurface.cpp
    ogr_opt.cpp
    ogrpgeogeometry.cpp
    ogrpoint.cpp
    ogrpolygon.cpp
    ogrspatialreference.cpp
    ogr_srs_dict.cpp
    ogr_srs_erm.cpp
    ogr_srs_esri.cpp
    ogr_srsnode.cpp
    ogr_srs_ozi.cpp
    ogr_srs_panorama.cpp
    ogr_srs_pci.cpp
    ogr_srs_proj4.cpp
    ogr_srs_usgs.cpp
    ogr_srs_validate.cpp
    ogr_srs_xml.cpp
    ogrsurface.cpp
    ogrutils.cpp
    osr_cs_wkt.c
    osr_cs_wkt_parser.c
    swq.cpp
    swq_expr_node.cpp
    swq_op_general.cpp
    swq_op_registrar.cpp
    swq_parser.cpp
    swq_select.cpp
)

option(GDAL_USE_GEOS "Set ON to use libgeos" ON)
if(GDAL_USE_GEOS)
    option(GDAL_USE_GEOS_INTERNAL "Set ON to use internal libgeos" OFF)
    add_definitions(-DHAVE_GEOS=1)
    if(GDAL_USE_GEOS_INTERNAL)
        # todo: get geos from external project
        set(GEOS_FOUND ON)
    else()
        find_package(GEOS REQUIRED)
        if(GEOS_FOUND)
            get_filename_component(GEOS_ROOT ${GEOS_INCLUDE_DIR} PATH)
            include_directories(${GEOS_INCLUDE_DIR})
            include_directories(${GEOS_ROOT}/capi)
        endif()
    endif()
else()
    message(WARNING "No geos support")
endif()

option(GDAL_USE_PROJ "Set ON to use PROJ.4" ON)
if(GDAL_USE_PROJ)
    option(GDAL_USE_PROJ_INTERNAL "Set ON to use internal PROJ.4" OFF)
    add_definitions(-DHAVE_PROJ) # todo: do we need it? -DPROJ_STATIC)
    if(GDAL_USE_PROJ_INTERNAL)
        ExternalProject_Add(proj
            GIT_REPOSITORY ${EP_URL}/lib_proj
            INSTALL_COMMAND ""
        )
        
        set (PROJ_DIR ${ep_base}/proj CACHE INTERNAL "proj.4 internal path")
        set (PROJ_INCLUDE_DIR ${PROJ_DIR} CACHE PATH "proj.4 internal include path")
        set (PROJ_LIBRARIES ${PROJ_DIR}/lib/proj CACHE INTERNAL "proj.4 internal lib path")
        set (PROJ_LIBRARY ${PROJ_LIBRARIES} CACHE FILEPATH "proj.4 internal lib path")
        
        link_directories(${LIBCURL_DIR}/lib)
    else()
        find_package(PROJ REQUIRED)
        if(PROJ_FOUND)
            include_directories(${PROJ_INCLUDE_DIR})
        endif()
    endif()
else()
    message(WARNING "No proj.4 support")
endif()

option(GDAL_USE_EXPAT "Set ON to use libexpat" ON)
if(GDAL_USE_EXPAT)
    option(GDAL_USE_EXPAT_INTERNAL "Set ON to use internal libgeos" OFF)
    add_definitions(-DHAVE_EXPAT)
    if(GDAL_USE_EXPAT_INTERNAL)
        ExternalProject_Add(expat
            GIT_REPOSITORY ${EP_URL}/lib_expat
            INSTALL_COMMAND ""
        )
       
        set (EXPAT_DIR ${ep_base}/expat CACHE INTERNAL "expat internal path")
        set (EXPAT_INCLUDE_DIRS ${EXPAT_DIR} CACHE PATH "expat internal include path")
        set (EXPAT_LIBRARIES ${EXPAT_DIR}/lib/expat CACHE INTERNAL "expat internal lib path")
        set (EXPAT_LIBRARY ${EXPAT_LIBRARIES} CACHE FILEPATH "expat internal lib path")
        
        link_directories(${ZLIB_DIR}/lib)
    else()
        find_package(EXPAT REQUIRED)
        if(EXPAT_FOUND)
            include_directories(${EXPAT_INCLUDE_DIR})
        endif()
    endif()
endif()

if(GDAL_USE_EXPAT)
    include_directories(${EXPAT_INCLUDE_DIRS})
    set(LIB_HHEADERS ${LIB_HHEADERS}
        ogr_expat.h
    )
    set(LIB_CSOURCES ${LIB_CSOURCES}
        ogr_expat.cpp
    )
else()
    message(WARNING "No expat support")
endif()

add_library(${LIB_NAME} OBJECT ${LIB_HHEADERS} ${LIB_CSOURCES})

if(GDAL_USE_EXPAT)    
    if(GDAL_USE_EXPAT_INTERNAL)        
        add_dependencies(${LIB_NAME} expat)
        #set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} expat})
    else()
        set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} ${EXPAT_LIBRARIES})
    endif()
endif()

if(GEOS_FOUND)
    set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} ${GEOS_LIBRARY})
endif()

if(GDAL_USE_PROJ)    
    if(GDAL_USE_PROJ_INTERNAL)        
        add_dependencies(${LIB_NAME} proj)
        #set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} proj})
    else()
        set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} ${PROJ_LIBRARY})
    endif()
endif()

# build target
set(GDAL_TARGET_OBJECTS ${GDAL_TARGET_OBJECTS} $<TARGET_OBJECTS:${LIB_NAME}> PARENT_SCOPE)
set(GDAL_TARGET_LINK_LIB ${GDAL_TARGET_LINK_LIB} PARENT_SCOPE)

# install headers
set(GDAL_INSTALL_HEADERS ${GDAL_INSTALL_HEADERS} ${LIB_INST_HEADERS} PARENT_SCOPE)
