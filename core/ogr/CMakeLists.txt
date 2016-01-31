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

option(GDAL_ENABLE_OGR "Build OGR module" ON)
if(GDAL_ENABLE_OGR)
    add_definitions(-DOGR_ENABLED)
endif()
add_definitions(-DHAVE_MITAB)

set(TARGET_NAME ogr)

add_definitions(-DGDAL_COMPILATION)

include_directories(${CMAKE_SOURCE_DIR}/core/port)
include_directories(${CMAKE_SOURCE_DIR}/core/gcore)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ogrsf_frmts)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ogrsf_frmts/mem)

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

find_anyproject(EXPAT DEFAULT ON)
if(EXPAT_FOUND)
    add_definitions(-DHAVE_EXPAT)
    set(LIB_HHEADERS_PRIV ${LIB_HHEADERS_PRIV}
        ogr_expat.h
    )
    set(LIB_CSOURCES ${LIB_CSOURCES}
        ogr_expat.cpp
    )
endif()

add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

# TODO: add some custome targets
# The sed substitution below workarounds a bug with gcc 4.1 -O2 (checked on 64bit platforms)
# that produces buggy compiled code.
# Seen on gcc 4.1.2-27ubuntu1 (Ubuntu 10.04) (not the default compiler) and gcc-4.1.2-48.el5 (CentOS 5.5)
# (default compiler...)
# The memset isn't necessary at all with a non-buggy compiler, but I've found
# that it helps gcc 4.1 generating correct code here...
#parser:
#	bison -p swq -d -oswq_parser.cpp swq_parser.y
#	sed "s/yytype_int16 yyssa\[YYINITDEPTH\];/yytype_int16 yyssa[YYINITDEPTH]; \/\* workaround bug with gcc 4.1 -O2 \*\/ memset(yyssa, 0, sizeof(yyssa));/" < swq_parser.cpp | sed "s/\*yyssp = yystate/\*yyssp = (yytype_int16)yystate/" > swq_parser.cpp.tmp
#	mv swq_parser.cpp.tmp swq_parser.cpp
#
# The sed hack is to please MSVC
#osr_cs_wkt_parser:
#	bison --no-lines -p osr_cs_wkt_ -d -oosr_cs_wkt_parser.c osr_cs_wkt_grammar.y
#	sed "s/\*yyssp = yystate/\*yyssp = (yytype_int16)yystate/" < osr_cs_wkt_parser.c | sed "s/yyerrorlab:/#if 0\nyyerrorlab:/" | sed "s/yyerrlab1:/#endif\nyyerrlab1:/" | sed "s/for (yylen = 0; yystr\[yylen\]; yylen++)/for (yylen = 0; yystr != YY_NULL \&\& yystr\[yylen\]; yylen++)/"  > osr_cs_wkt_parser.c.tmp
#	mv osr_cs_wkt_parser.c.tmp osr_cs_wkt_parser.c
#
#osr_cs_wkt_parser.$(OBJ_EXT):   osr_cs_wkt_parser.c
#	$(CC) $(GDAL_INCLUDE) $(CFLAGS) $(CPPFLAGS) $(NO_LOGICAL_OP_FLAG) -c -o $@ $<
#
#swq_parser.$(OBJ_EXT):   swq_parser.cpp
#	$(CXX) $(GDAL_INCLUDE) $(CXXFLAGS) $(CPPFLAGS) $(NO_LOGICAL_OP_FLAG) -c -o $@ $<


# build target
set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
set(TARGET_LINK_LIB ${TARGET_LINK_LIB} PARENT_SCOPE)

# install headers
if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${LIB_HHEADERS} DESTINATION ${INSTALL_INC_DIR} COMPONENT headers)
endif() 
