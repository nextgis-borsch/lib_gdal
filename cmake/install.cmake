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

set_target_properties(${GDAL_LIB_NAME}
    PROPERTIES PROJECT_LABEL ${PROJECT_NAME}
    VERSION ${GDAL_VERSION}
    SOVERSION 1
    ARCHIVE_OUTPUT_DIRECTORY ${GDAL_ROOT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${GDAL_ROOT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${GDAL_ROOT_BINARY_DIR}
    )

#install lib and bin

install(TARGETS ${GDAL_LIB_NAME}
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )

if(UNIX)
    option(GDAL_INSTALL_DATA_IN_VERSION_DIR "Set ON to install GDAL in path with version (i.e. usr/local/share/gdal/1.11" OFF)
    if(GDAL_INSTALL_DATA_IN_VERSION_DIR)
        install(DIRECTORY ${GDAL_ROOT_SOURCE_DIR}/data/ DESTINATION share/gdal/${GDAL_VERSION} FILES_MATCHING PATTERN "*.*")
    else()
        install(DIRECTORY ${GDAL_ROOT_SOURCE_DIR}/data/ DESTINATION share/gdal FILES_MATCHING PATTERN "*.*")
    endif()
elseif(WIN32)
    install(DIRECTORY ${GDAL_ROOT_SOURCE_DIR}/data/ DESTINATION share/gdal FILES_MATCHING PATTERN "*.*")
endif()

# install headers
install(FILES ${GDAL_INSTALL_HEADERS} DESTINATION include/gdal)
