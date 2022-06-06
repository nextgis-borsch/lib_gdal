################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015-2020 NextGIS <info@nextgis.com>
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

#option(GDAL_ENABLE_OGR "Build OGR module" ON)
if(GDAL_ENABLE_OGR)
    add_definitions(-DOGR_ENABLED)
endif()
add_definitions(-DHAVE_MITAB)

set(TARGET_NAME ogr)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ogrsf_frmts)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ogrsf_frmts/mem)
include_directories(${CMAKE_SOURCE_DIR}/drivers/vector/geojson)

set(LIB_HHEADERS
    ogr_api.h
    ogr_core.h
    ogr_feature.h
    ogr_featurestyle.h
    ogr_geometry.h
    ogr_p.h
    ogr_spatialref.h
    ogr_srs_api.h
    ogr_geocoding.h
    ogrsf_frmts/ogrsf_frmts.h
)

set(LIB_HHEADERS_PRIV
    ograpispy.h
    ogrpgeogeometry.h
    ogr_srs_esri_names.h
    ogr_swq.h
    ogr_libs.h
    ogr_geo_utils.h
    ogr_proj_p.h
)

set(LIB_CSOURCES
#    generate_encoding_table.c do we need special target for this?
    gml2ogrgeometry.cpp
    ogr2gmlgeometry.cpp
    ogr_api.cpp
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
    ogr_srs_usgs.cpp
    ogr_srs_xml.cpp
    ogrsurface.cpp
    ogrutils.cpp
    swq.cpp
    swq_expr_node.cpp
    swq_op_general.cpp
    swq_op_registrar.cpp
    swq_select.cpp
    ograpispy.cpp
    ogrpgeogeometry.cpp
    ogrpolyhedralsurface.cpp
    ogrtriangle.cpp
    ogrtriangulatedsurface.cpp
    ogr_geo_utils.cpp
    ogr_proj_p.cpp
)


if(EXPAT_FOUND)
    set(LIB_HHEADERS_PRIV ${LIB_HHEADERS_PRIV}
        ogr_expat.h
    )
    set(LIB_CSOURCES ${LIB_CSOURCES}
        ogr_expat.cpp
    )
endif()

# if(XERCES_FOUND)
    set(LIB_HHEADERS_PRIV ${LIB_HHEADERS_PRIV}
        ogr_xerces_headers.h
        ogr_xerces.h
    )
    set(LIB_CSOURCES ${LIB_CSOURCES}
        ogr_xerces.cpp
    )
# endif()

if(SFCGAL_FOUND)
    set(LIB_HHEADERS_PRIV ${LIB_HHEADERS_PRIV}
        ogr_sfcgal.h
    )
endif()

option(GDAL_UPDATE_SWQ "Set ON to update swq files" OFF)
if(GDAL_UPDATE_SWQ)
    find_package(BISON)
    if(BISON_FOUND)
        BISON_TARGET(SWQParser swq_parser.y ${CMAKE_CURRENT_BINARY_DIR}/swq_parser.cpp)
        set(LIB_CSOURCES ${LIB_CSOURCES}
            ${BISON_SWQParser_OUTPUTS}
        )
    endif()
endif()

if(NOT GDAL_UPDATE_SWQ OR NOT BISON_FOUND)
    set(LIB_CSOURCES ${LIB_CSOURCES}
        swq_parser.cpp
    )
endif()

add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

# Build target
set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
any_project_var_to_parent_scope()

# Install headers
if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${LIB_HHEADERS} DESTINATION ${INSTALL_INC_DIR} COMPONENT headers)
endif()
