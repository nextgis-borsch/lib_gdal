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

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOX_INPUT "${CMAKE_SOURCE_DIR}/core/port ${CMAKE_SOURCE_DIR}/core/gcore ${CMAKE_SOURCE_DIR}/core/alg ${CMAKE_SOURCE_DIR}/docs/doc  ${CMAKE_SOURCE_DIR}/apps ${CMAKE_SOURCE_DIR}/core/ogr ${CMAKE_SOURCE_DIR}/bindings/python/scripts ${CMAKE_SOURCE_DIR}/core/gnm"
    )
    
    #                         frmts/gdalallregister.cpp \ \n
    #                         frmts/vrt \ \n
    #                         ogr/ogrsf_frmts \ \n
    #                         ogr/ogrsf_frmts/generic \ \n
    #                         ogr/ogrsf_frmts/geojson/ogrgeojsonwriter.cpp \ \n
    #                         ogr/ogrsf_frmts/kml/ogr2kmlgeometry.cpp \ \n


    set(DOX_FILE_PATTERNS "*.h *.cpp *.c *.dox")
                             
    set(DOX_GENERATE_HTML "YES")                         
    set(DOX_GENERATE_MAN "NO")   

    set(DOX_EXAMPLE_PATH "${CMAKE_SOURCE_DIR}/apps # frmts \ frmts/jdem")      

    set(HTML_FOOTER "${CMAKE_SOURCE_DIR}/docs/doc/gdal_footer.html")                
    find_package(Perl)
    set(PERL_PATH ${PERL_EXECUTABLE})

    configure_file(${CMAKE_MODULE_PATH}/Doxyfile.cmakein  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.html IMMEDIATE @ONLY)    

    add_custom_target(prepare_html
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/doc/gdal_footer.html ${CMAKE_BINARY_DIR}/gdal_footer.html
        COMMAND ${DOXYGEN_EXECUTABLE} -w html ${CMAKE_CURRENT_BINARY_DIR}/html/header.html ${CMAKE_CURRENT_BINARY_DIR}/html/footer.html ${CMAKE_CURRENT_BINARY_DIR}/html/stylesheet.css ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.html
        COMMENT "Create html templates"
    )

    add_custom_target (docs          
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/html/header.html ${CMAKE_CURRENT_BINARY_DIR}/html/header_br.html
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/html/header.html ${CMAKE_CURRENT_BINARY_DIR}/html/header_ru.html
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/doc/gdal_footer.html ${CMAKE_CURRENT_BINARY_DIR}/html/gdal_footer.html
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/doc/br/Doxyfile"
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/doc/ru/Doxyfile"
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.html"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/data/gdalicon.png ${CMAKE_CURRENT_BINARY_DIR}/html
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/doc/images/ ${CMAKE_CURRENT_BINARY_DIR}/html
        #COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/doc/grid/*.png ${CMAKE_CURRENT_BINARY_DIR}/html
        DEPENDS prepare_html
        COMMENT "Create html"
    )       
    
# TODO:
#    	cp doc/grid/*.png html
#	cp frmts/*.html frmts/*/frmt_*.html html
#	cp frmts/openjpeg/*.xml html
#	cp frmts/wms/frmt_*.xml html
#	cp ogr/ogrsf_frmts/*/drv_*.html html
#	cp ogr/ogrsf_frmts/ogr_formats.html html
#	cp ogr/ogr_feature_style.html html
#	cp ogr/ogrsf_frmts/gpkg/geopackage_aspatial.html html
#	cp ogr/*.gif html


    
    set(DOX_GENERATE_HTML "NO")                         
    set(DOX_GENERATE_MAN "YES")   
    set(ENABLED_SECTIONS "man")
    set(DOX_FILE_PATTERNS "*.cpp *.dox")
    set(DOX_INPUT "${CMAKE_SOURCE_DIR}/apps ${CMAKE_SOURCE_DIR}/bindings/python/scripts")

    configure_file(${CMAKE_MODULE_PATH}/Doxyfile.cmakein  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.man IMMEDIATE @ONLY)     
    
    add_custom_target (man
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.man
        COMMENT "Create manuals"
    )
    
    if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL )
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man/man1 DESTINATION ${INSTALL_MAN_DIR})
    endif()
                   
endif()

