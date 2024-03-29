#!/usr/bin/env pytest
# -*- coding: utf-8 -*-
###############################################################################
# $Id$
#
# Project:  GDAL/OGR Test Suite
# Purpose:  Test "shared" open, and various refcount based stuff.
# Author:   Frank Warmerdam <warmerdam@pobox.com>
#
###############################################################################
# Copyright (c) 2003, Frank Warmerdam <warmerdam@pobox.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
###############################################################################


import gdaltest

from osgeo import ogr

###############################################################################
# Open two datasets in shared mode.


def test_ogr_refcount_1():
    # if ogr.GetOpenDSCount() != 0:
    #    gdaltest.post_reason( 'Initial Open DS count is not zero!' )
    #    return 'failed'

    gdaltest.ds_1 = ogr.OpenShared("data/idlink.dbf")
    gdaltest.ds_2 = ogr.OpenShared("data/poly.shp")

    # if ogr.GetOpenDSCount() != 2:
    #    gdaltest.post_reason( 'Open DS count not 2 after shared opens.' )
    #    return 'failed'

    if gdaltest.ds_1.GetRefCount() != 1 or gdaltest.ds_2.GetRefCount() != 1:
        gdaltest.post_reason("Reference count not 1 on one of datasources.")
        return "failed"


###############################################################################
# Verify that reopening one of the datasets returns the existing shared handle.


def test_ogr_refcount_2():

    ds_3 = ogr.OpenShared("data/idlink.dbf")

    # if ogr.GetOpenDSCount() != 2:
    #    gdaltest.post_reason( 'Open DS count not 2 after third open.' )
    #    return 'failed'

    # This test only works with the old bindings.
    try:
        if ds_3._o != gdaltest.ds_1._o:
            gdaltest.post_reason("We did not get the expected pointer.")
            return "failed"
    except Exception:
        pass

    if ds_3.GetRefCount() != 2:
        gdaltest.post_reason("Refcount not 2 after reopened.")
        return "failed"

    gdaltest.ds_3 = ds_3


###############################################################################
# Verify that releasing the datasources has the expected behaviour.


def test_ogr_refcount_3():

    gdaltest.ds_3.Release()

    if gdaltest.ds_1.GetRefCount() != 1:
        gdaltest.post_reason("Refcount not decremented as expected.")
        return "failed"

    gdaltest.ds_1.Release()


###############################################################################
# Verify that we can walk the open datasource list.


def test_ogr_refcount_4():

    with gdaltest.error_handler():
        ds = ogr.GetOpenDS(0)
    try:
        if ds._o != gdaltest.ds_2._o:
            gdaltest.post_reason("failed to fetch expected datasource")
            return "failed"
    except Exception:
        pass


###############################################################################


def test_ogr_refcount_cleanup():
    gdaltest.ds_2.Release()
