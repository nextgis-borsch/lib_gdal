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
    set(DOX_INPUT "${CMAKE_SOURCE_DIR}/core/port \\ 
    ${CMAKE_SOURCE_DIR}/core/gcore \\
    ${CMAKE_SOURCE_DIR}/core/alg \\ 
    ${CMAKE_SOURCE_DIR}/docs/doc \\ 
    ${CMAKE_SOURCE_DIR}/apps \\
    ${CMAKE_SOURCE_DIR}/core/ogr \\
    ${CMAKE_SOURCE_DIR}/bindings/python/scripts \\
    ${CMAKE_SOURCE_DIR}/core/gnm \\
    ${CMAKE_SOURCE_DIR}/docs/apps \\
    ${CMAKE_SOURCE_DIR}/docs/gnm \\
    ${CMAKE_SOURCE_DIR}/docs/ogr"
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
    set(HTML_FOOTER "${CMAKE_CURRENT_BINARY_DIR}/html/gdal_footer.html")      
              
    find_package(Perl)
    set(PERL_PATH ${PERL_EXECUTABLE})
    
    #prepare translation
    set(TRANSLATIONS ru br)
    macro(prepare_translation trans)
        file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/doc/${trans}/Doxyfile Contents)
        foreach(Line ${Contents})
            if("${Line}" MATCHES "^# " OR 
               "${Line}" MATCHES "^USE_WINDOWS_ENCODING " OR 
               "${Line}" MATCHES "^DETAILS_AT_TOP " OR 
               "${Line}" MATCHES "^HTML_ALIGN_MEMBERS " OR
               "${Line}" MATCHES "^XML_SCHEMA " OR 
               "${Line}" MATCHES "^XML_DTD " OR 
               "${Line}" MATCHES "^MAX_DOT_GRAPH_WIDTH " OR 
               "${Line}" MATCHES "^MAX_DOT_GRAPH_HEIGHT")
                set(Line "NOP")   
            elseif("${Line}" MATCHES "^INPUT ")
                set(Line "INPUT = ${CMAKE_CURRENT_SOURCE_DIR}/doc/${trans}")
            elseif("${Line}" MATCHES "^FILE_PATTERNS ")
                set(Line "FILE_PATTERNS = ${DOX_FILE_PATTERNS}")
            elseif("${Line}" MATCHES "^HTML_FOOTER ") 
                set(Line "HTML_FOOTER = ${CMAKE_CURRENT_BINARY_DIR}/html/gdal_footer.html")   
            elseif("${Line}" MATCHES "^EXAMPLE_PATH ") 
                set(Line "EXAMPLE_PATH = ${DOX_EXAMPLE_PATH}") 
            elseif("${Line}" MATCHES "^PERL_PATH ") 
                set(Line "PERL_PATH = ${PERL_PATH}") 
            endif()
            if(NOT "${Line}" STREQUAL "NOP")
                set(ModifiedContents "${ModifiedContents}${Line}\n")
            endif()
        endforeach()
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile_${trans}.html ${ModifiedContents})        
    endmacro()
    
    foreach(TRANSLATION ${TRANSLATIONS})
        prepare_translation(${TRANSLATION})
    endforeach()
    
    #get images list
    file(GLOB DOX_HTML_IMAGES ${CMAKE_CURRENT_SOURCE_DIR}/doc/grid/*.png
                              ${CMAKE_CURRENT_SOURCE_DIR}/doc/images/* )
    set(DOX_HTML_IMAGES ${DOX_HTML_IMAGES} ${CMAKE_CURRENT_SOURCE_DIR}/data/gdalicon.png)                           

    configure_file(${CMAKE_MODULE_PATH}/Doxyfile.cmakein  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.html IMMEDIATE @ONLY)    

    add_custom_target(prepare_html
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/doc/gdal_footer.html ${CMAKE_CURRENT_BINARY_DIR}/html/gdal_footer.html
        COMMAND ${DOXYGEN_EXECUTABLE} -w html html/header.html html/footer.html html/stylesheet.css ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.html
        COMMENT "Create html templates"
    )

    add_custom_target (docs  
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.html"
        DEPENDS prepare_html
        COMMENT "Create html"
    )  
    
    foreach(TRANSLATION ${TRANSLATIONS})        
        add_custom_command(TARGET prepare_html POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/html/header.html ${CMAKE_CURRENT_BINARY_DIR}/html/header_${TRANSLATION}.html
              COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile_${TRANSLATION}.html"
              COMMENT "Create translation for ${TRANSLATION}"
        )
    endforeach()

    foreach(DOX_HTML_IMAGE ${DOX_HTML_IMAGES})
        add_custom_command(TARGET docs POST_BUILD
             COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DOX_HTML_IMAGE} ${CMAKE_CURRENT_BINARY_DIR}/html
        )
    endforeach()     
    
# TODO:
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

