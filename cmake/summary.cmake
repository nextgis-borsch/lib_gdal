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

macro(summary_message text value)
    string(ASCII 27 Esc)
    set(BoldYellow  "${Esc}[1;33m")
    set(Magenta     "${Esc}[35m")
    set(Cyan        "${Esc}[36m")
    set(BoldCyan    "${Esc}[1;36m")
    set(White       "${Esc}[37m")
    set(ColourReset "${Esc}[m")
  
    if("${${value}}")
        message(STATUS "${BoldCyan}  ${text} yes${ColourReset}")
    else()
        message(STATUS "${Cyan}  ${text} no${ColourReset}")
    endif()
endmacro()

message(STATUS "GDAL is now configured for ${CMAKE_SYSTEM_NAME}")
message(STATUS "")
if(GDAL_SUMMARY)
    foreach(TO ${GDAL_SUMMARY})
        message(STATUS "${TO}")
    endforeach()
    message(STATUS "")
endif()
message(STATUS "  Installation directory:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  C compiler:                ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS}")
message(STATUS "  C++ compiler:              ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS}")
# TODO: do we need it?
#message(STATUS "")
#summary_message("  LIBTOOL support:         "  WITH_LIBTOOL)
message(STATUS "")
summary_message("LIBZ support:              " WITH_ZLIB)
summary_message("LIBLZMA support:           " WITH_LIBLZMA)
summary_message("cryptopp support:          " WITH_CRYPTOPP)
summary_message("GRASS support:             " WITH_GRASS) 
summary_message("CFITSIO support:           " WITH_CFITSIO)
summary_message("PCRaster support:          " WITH_PCRASTER)
summary_message("LIBPNG support:            " WITH_PNG)
summary_message("DDS support:               " WITH_DDS)
summary_message("GTA support:               " WITH_GTA)
summary_message("LIBTIFF support (BigTIFF=${HAVE_BIGTIFF}): " WITH_TIFF)
summary_message("LIBGEOTIFF support:        " WITH_GEOTIFF)
summary_message("LIBJPEG support:           " WITH_JPEG)
summary_message("12 bit JPEG:               " WITH_JPEG12)
summary_message("12 bit JPEG-in-TIFF:       " TIFF_JPEG12_ENABLED)
summary_message("LIBGIF support:            " WITH_GIF)
summary_message("OGDI support:              " WITH_OGDI)
summary_message("HDF4 support:              " WITH_HDF4)
summary_message("HDF5 support:              " WITH_HDF5)
summary_message("Kea support:               " WITH_KEA)
summary_message("NetCDF support:            " WITH_NETCDF)
summary_message("Kakadu support:            " WITH_KAKADU)

if(HAVE_JASPER_UUID)
summary_message("JasPer support (GeoJP2=${HAVE_JASPER_UUID}): " WITH_JASPER)
else()
summary_message("JasPer support:            " WITH_JASPER)
endif()

summary_message("OpenJPEG support:          " WITH_OPENJPEG)
summary_message("ECW support:               " WITH_ECW)
summary_message("MrSID support:             " WITH_MRSID)
summary_message("MrSID/MG4 Lidar support:   " WITH_MRSID_LIDAR)
summary_message("MSG support:               " WITH_MSG)
summary_message("GRIB support:              " WITH_GRIB)
summary_message("EPSILON support:           " WITH_EPSILON)
summary_message("WebP support:              " WITH_WEBP)
summary_message("cURL support (wms/wcs/...):" WITH_CURL)
summary_message("PostgreSQL support:        " WITH_PG)
summary_message("MRF support:               " WITH_MRF)
summary_message("MySQL support:             " WITH_MYSQL)
summary_message("Ingres support:            " WITH_INGRES)
summary_message("Xerces-C support:          " WITH_XERCES)
summary_message("NAS support:               " WITH_NAS)
summary_message("Expat support:             " WITH_EXPAT)
summary_message("libxml2 support:           " WITH_LIBXML2)
summary_message("Google libkml support:     " WITH_LIBKML)
summary_message("ODBC support:              " WITH_ODBC)
summary_message("PGeo support:              " WITH_PGEO)
summary_message("FGDB support:              " WITH_FGDB)
summary_message("MDB support:               " WITH_MDB)
summary_message("PCIDSK support:            " WITH_PCIDSK)
summary_message("OCI support:               " WITH_OCI)
summary_message("GEORASTER support:         " WITH_GEORASTER)
summary_message("SDE support:               " WITH_SDE)
summary_message("Rasdaman support:          " WITH_RASDAMAN)
summary_message("DODS support:              " WITH_DODS)
summary_message("SQLite support:            " WITH_SQLITE)
summary_message("PCRE support:              " WITH_PCRE)
summary_message("SpatiaLite support:        " WITH_SPATIALITE)
summary_message("DWGdirect support          " WITH_DWGDIRECT)
summary_message("INFORMIX DataBlade support:" WITH_IDB)
summary_message("GEOS support:              " WITH_GEOS)
summary_message("QHull support:             " WITH_QHULL)
summary_message("Poppler support:           " WITH_POPPLER)
summary_message("Podofo support:            " WITH_PODOFO)
summary_message("PDFium support:            " WITH_PDFIUM)
summary_message("OpenCL support:            " WITH_OPENCL)
summary_message("Armadillo support:         " WITH_ARMADILLO)
summary_message("FreeXL support:            " WITH_FREEXL)
summary_message("SOSI support:              " WITH_SOSI)
summary_message("MongoDB support:           " WITH_MONGODB)
message(STATUS "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
summary_message("Mac OS X Framework :       " MACOSX_FRAMEWORK)
message(STATUS "")
endif()
if(GDAL_BINDINGS)
    message(STATUS "  SWIG Bindings:              ${GDAL_BINDINGS}")
else()
    message(STATUS "  SWIG Bindings:              no")
endif()
message(STATUS "")
summary_message("PROJ.4 support:            " WITH_PROJ4)
summary_message("Json-c support:            " WITH_JSONC)
summary_message("enable OGR building:       " GDAL_ENABLE_OGR)
summary_message("enable GNM building:       " GDAL_ENABLE_GNM)
message(STATUS "")
summary_message("enable pthread support:    " GDAL_USE_CPL_MULTIPROC_PTHREAD)
summary_message("enable POSIX iconv support:" WITH_ICONV)
summary_message("hide internal symbols:     " GDAL_HIDE_INTERNAL_SYMBOLS)

if(WITH_PODOFO AND WITH_POPPLER AND WITH_PDFIUM)
    message(WARNING "--with-podofo, --with-poppler and --with-pdfium available. 
                     This is unusual setup, but will work. Pdfium will be used 
                     by default...")
elseif(WITH_PODOFO AND WITH_POPPLER)
    message(WARNING "--with-podofo and --with-poppler are both available. 
                     This is unusual setup, but will work. Poppler will be used 
                     by default...")
elseif(WITH_POPPLER AND WITH_PDFIUM)
    message(WARNING "--with-poppler and --with-pdfium are both available. This 
                     is unusual setup, but will work. Pdfium will be used by 
                     default...")
elseif(WITH_PODOFO AND WITH_PDFIUM)
    message(WARNING "--with-podofo and --with-pdfium are both available. This is 
                     unusual setup, but will work. Pdfium will be used by 
                     default...")
endif()

if(WITH_LIBXML2 AND WITH_FGDB)
    message(WARNING "-DWITH_LIBXML2 and -DWITH_FGDB are both available. 
                     There might be some incompatibility between system libxml2 
                     and the embedded copy within libFileGDBAPI")
endif()   

