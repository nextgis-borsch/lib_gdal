################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Mikhail Gusev, gusevmihs@gmail.com
################################################################################
# Copyright (C) 2015-2020, NextGIS <info@nextgis.com>
# Copyright (C) 2016 Mikhail Gusev
# Copyright (C) 2020 Dmitry Baryshnikov
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

option(ENABLE_GRIB "Build WMO GRIB1/GRIB2 (.grb) format" ON)
if(ENABLE_GRIB)
    set(TARGET_NAME ras_grib)

    add_definitions(-DFRMT_grib)
    set(FORMAT_DEFINITIONS ${FORMAT_DEFINITIONS} "-DFRMT_grib" PARENT_SCOPE)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/degrib/degrib)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/degrib/g2clib)
    include_directories(${CMAKE_SOURCE_DIR}/drivers/raster/mem)

    # TODO: implement JASPER support when it will be available as an external library.
    #if(JASPER_FOUND)
    #    add_definitions(-DHAVE_JASPER)
    #endif()

    set(LIB_HHEADERS
    )

    set(LIB_HHEADERS_PRIV
        gribdataset.h
    )

    set(LIB_CSOURCES
        gribdataset.cpp
        gribcreatecopy.cpp
    )

    set(LIB_CSOURCES ${LIB_CSOURCES}
        degrib/degrib/clock.c
        degrib/degrib/degrib1.cpp
        degrib/degrib/degrib2.cpp
        degrib/degrib/inventory.cpp
        degrib/degrib/metaname.cpp
        degrib/degrib/grib1tab.cpp
        degrib/degrib/myutil.c
        degrib/degrib/metaparse.cpp
        degrib/degrib/weather.c
        degrib/degrib/metaprint.cpp
        degrib/degrib/grib2api.c
        degrib/degrib/myassert.c
        degrib/degrib/scan.c
        degrib/degrib/tendian.cpp
        degrib/degrib/hazard.c
        degrib/degrib/myerror.cpp
    )

    set(LIB_CSOURCES ${LIB_CSOURCES}
        degrib/g2clib/gridtemplates.c
        degrib/g2clib/drstemplates.c
        degrib/g2clib/pdstemplates.c
        degrib/g2clib/gbits.c
        degrib/g2clib/g2_free.c
        degrib/g2clib/g2_unpack1.c
        degrib/g2clib/g2_unpack2.c
        degrib/g2clib/g2_unpack3.c
        degrib/g2clib/g2_unpack4.c
        degrib/g2clib/g2_unpack5.c
        degrib/g2clib/g2_unpack6.c
        degrib/g2clib/g2_unpack7.c
        degrib/g2clib/g2_info.c
        degrib/g2clib/g2_getfld.c
        degrib/g2clib/simunpack.c
        degrib/g2clib/comunpack.c
        degrib/g2clib/pack_gp.c
        degrib/g2clib/reduce.c
        degrib/g2clib/specunpack.c
        degrib/g2clib/rdieee.c
        degrib/g2clib/mkieee.c
        degrib/g2clib/int_power.c
        degrib/g2clib/simpack.c
        degrib/g2clib/compack.c
        degrib/g2clib/cmplxpack.c
        degrib/g2clib/misspack.c
        degrib/g2clib/dec_jpeg2000.cpp
        degrib/g2clib/jpcunpack.c
    )

    add_library(${TARGET_NAME} OBJECT ${LIB_HHEADERS} ${LIB_HHEADERS_PRIV} ${LIB_CSOURCES})

    # build target
    set(TARGET_OBJECTS ${TARGET_OBJECTS} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
else()
    warning_msg("Skip grib")
endif()
