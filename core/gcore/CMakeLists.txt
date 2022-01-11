################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015,2020 NextGIS <info@nextgis.com>
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

set(TARGET_NAME gcore)

include_directories(${CMAKE_SOURCE_DIR}/core/ogr/ogrsf_frmts)
include_directories(${CMAKE_SOURCE_DIR}/core/ogr/ogrsf_frmts/generic)
include_directories(${CMAKE_SOURCE_DIR}/core/ogr/ogrsf_frmts/geojson)
include_directories(${CMAKE_SOURCE_DIR}/core/frmts/gtiff)
include_directories(${CMAKE_SOURCE_DIR}/core/frmts/vrt)
include_directories(${CMAKE_SOURCE_DIR}/core/frmts/mem)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# TODO: remove this when change according include path in gdaldataset.cpp
include_directories(${CMAKE_SOURCE_DIR}/drivers/vector/sqlite)

configure_file(gdal_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/gdal_version.h COPYONLY)

set(LIB_HHEADERS
    gdal_frmts.h
    gdalgeorefpamdataset.h
    gdal.h
    gdaljp2abstractdataset.h
    gdaljp2metadata.h
    gdal_mdreader.h
    gdal_pam.h
    gdal_priv.h
    gdal_proxy.h
    gdal_rat.h
    rawdataset.h
    tilematrixset.hpp
    nasakeywordhandler.h
    ${CMAKE_CURRENT_BINARY_DIR}/gdal_version.h
)

set(LIB_HHEADERS_PRIV
# TODO: more elegant format's register  gdal_frmts.h
    gdaljp2metadatagenerator.h
    gdalsse_priv.h
    gdal_avx2_emulation.hpp
    gdal_thread_pool.h
)

set(LIB_CSOURCES
    gdalmajorobject.cpp
    gdalabstractbandblockcache.cpp
    gdalallvalidmaskband.cpp
    gdalarraybandblockcache.cpp
    gdalclientserver.cpp
    gdalcolortable.cpp
    gdaldataset.cpp
    gdaldefaultasync.cpp
    gdaldefaultoverviews.cpp
    gdaldllmain.cpp
    gdaldriver.cpp
    gdaldrivermanager.cpp
    gdalexif.cpp
    gdalgeorefpamdataset.cpp
    gdalhashsetbandblockcache.cpp
    gdaljp2abstractdataset.cpp
    gdaljp2box.cpp
    gdaljp2metadata.cpp
    gdaljp2metadatagenerator.cpp
    gdaljp2structure.cpp
    gdal_mdreader.cpp
    gdal_misc.cpp
    gdalmultidomainmetadata.cpp
    gdalnodatamaskband.cpp
    gdalnodatavaluesmaskband.cpp
    gdalopeninfo.cpp
    gdaloverviewdataset.cpp
    gdalproxydataset.cpp
    gdalproxypool.cpp
    gdalrasterband.cpp
    gdalrasterblock.cpp
    gdal_rat.cpp
    gdalrescaledalphaband.cpp
    gdalvirtualmem.cpp
    overview.cpp
    rasterio.cpp
    rasterio_ssse3.cpp
    rawdataset.cpp
    gdalmultidim.cpp
    tilematrixset.cpp
    gdal_thread_pool.cpp
    nasakeywordhandler.cpp
)

if(GDAL_PAM_ENABLED)
    set(LIB_HHEADERS ${LIB_HHEADERS}
        gdal_pam.h
    )
    set(LIB_CSOURCES ${LIB_CSOURCES}
        gdalpamdataset.cpp
        gdalpamrasterband.cpp
        gdalpamproxydb.cpp
    )
endif()

set(LIB_HHEADERS ${LIB_HHEADERS}
    gdalpython.h
)
set(LIB_CSOURCES ${LIB_CSOURCES}
    gdalpython.cpp
    gdalpythondriverloader.cpp
)

if(WITH_PYTHON)
    # TODO: 
else()
    add_definitions(-DGDAL_VRT_DISABLE_PYTHON -DGDAL_NO_AUTOLOAD)
endif()

if(WIN32)
    set(LIB_CSOURCES ${LIB_CSOURCES} Version.rc)
endif()

add_subdirectory(mdreader)

add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

# Build target
set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
set(TARGET_LINK_LIB ${TARGET_LINK_LIB} PARENT_SCOPE)
set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${TARGET_NAME} PARENT_SCOPE)

# Install headers
if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES ${LIB_HHEADERS} DESTINATION ${INSTALL_INC_DIR} COMPONENT headers)
endif()
