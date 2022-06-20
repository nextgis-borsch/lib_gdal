#!/usr/bin/env pytest
# -*- coding: utf-8 -*-
###############################################################################
# $Id: ogr_sxf.py 26513 2013-10-02 11:59:50Z bishop $
#
# Project:  GDAL/OGR Test Suite
# Purpose:  Test OGR SXF driver functionality.
# Author:   Dmitry Baryshnikov <polimax@mail.ru>
#
###############################################################################
# Copyright (c) 2013, NextGIS
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

import shutil
import sys

import gdaltest
from osgeo import gdal
from osgeo import ogr
from osgeo import osr
import pytest
import random

###############################################################################
# Open SXF datasource.


def test_ogr_sxf_1():

    gdaltest.sxf_ds = None
    with gdaltest.error_handler():
        # Expect Warning 0 and Warning 6.
        gdaltest.sxf_ds = ogr.Open('data/sxf/100_test.sxf')

    if gdaltest.sxf_ds is not None:
        return
    pytest.fail()


###############################################################################
# Run test_ogrsf

def test_ogr_sxf_2():

    import test_cli_utilities
    if test_cli_utilities.get_test_ogrsf_path() is None:
        pytest.skip()

    ret = gdaltest.runexternal(test_cli_utilities.get_test_ogrsf_path() + ' data/sxf/100_test.sxf')

    assert ret.find('INFO') != -1 and ret.find('ERROR') == -1


###############################################################################
# Open SXF datasource with custom RSC file.

def test_ogr_sxf_3():

    lyr_names = ['SYSTEM',
                 'Not_Classified']
    sxf_name = 'tmp/test_ogr_sxf_3.sxf'
    rsc_name = 'tmp/test_ogr_sxf_3.rsc'
    fake_rsc = open(rsc_name, 'w')
    fake_rsc.close()
    shutil.copy('data/sxf/100_test.sxf', sxf_name)
    sxf_ds = gdal.OpenEx(sxf_name, gdal.OF_VECTOR, open_options=['SXF_RSC_FILENAME=' + rsc_name])

    assert sxf_ds is not None

    for layer_n in range(sxf_ds.GetLayerCount()):
        lyr = sxf_ds.GetLayer(layer_n)
        assert lyr_names[layer_n] == lyr.GetName()

###############################################################################
# Open SXF datasource with layers fullname.

def test_ogr_sxf_4(capsys):

    lyr_names = ['СИСТЕМНЫЙ',
                 'ВОДНЫЕ ОБЪЕКТЫ',
                 'НАСЕЛЕННЫЕ ПУНКТЫ',
                 'ИНФРАСТРУКТУРА',
                 'ЗЕМЛЕПОЛЬЗОВАНИЕ',
                 'РЕЛЬЕФ СУШИ',
                 'ГИДРОГРАФИЯ (РЕЛЬЕФ)',
                 'МАТЕМАТИЧЕСКАЯ ОСНОВА',
                 'Not_Classified']
    sxf_name = 'data/sxf/100_test.sxf'
    sxf_ds = gdal.OpenEx(sxf_name, gdal.OF_VECTOR, open_options=['SXF_LAYER_FULLNAME=YES'])

    assert sxf_ds is not None
    assert sxf_ds.GetLayerCount() == len(lyr_names)

    if sys.platform != 'win32':
        with capsys.disabled():
            print('Expected:')
            for n in lyr_names:
                print(n)
            print('In fact:')
            for layer_n in range(sxf_ds.GetLayerCount()):
                lyr = sxf_ds.GetLayer(layer_n)
                print(lyr.GetName())

    for layer_n in range(sxf_ds.GetLayerCount()):
        lyr = sxf_ds.GetLayer(layer_n)
        if lyr.TestCapability(ogr.OLCStringsAsUTF8) != 1:
            pytest.skip('skipping test: recode is not possible')
        assert lyr_names[layer_n] == lyr.GetName()

###############################################################################
# Create/Read SXF datasource

def create_datasource(sxf_name, test_data, layer_names):

    create_ds  = gdal.GetDriverByName('SXF').Create(sxf_name, 0, 0, 0, gdal.GDT_Unknown)
    assert create_ds is not None

    for name in layer_names:
        layer = create_ds.CreateLayer(name, srs = osr.SpatialReference(), geom_type = ogr.wkbPoint, options = ['SXF_NEW_BEHAVIOR=YES'])
        assert layer is not None
        
        layer.CreateField(ogr.FieldDefn('STRFIELD', ogr.OFTString))
        layer.CreateField(ogr.FieldDefn('DECFIELD', ogr.OFTInteger))
        layer.CreateField(ogr.FieldDefn('REALFIELD', ogr.OFTReal))
        
        feature = ogr.Feature(layer.GetLayerDefn())
        
        # With SXF_NEW_BEHAVIOR=YES, features are not created if the some system fields are not filled in:
        feature.SetField('OT', 'P')
        feature.SetField('CLCODE', random.randint(0, 999))
        
        feature.SetField('STRFIELD', test_data['STRFIELD_TEST_VALUE'])
        feature.SetField('DECFIELD', test_data['DECFIELD_TEST_VALUE'])
        feature.SetField('REALFIELD', test_data['REALFIELD_TEST_VALUE'])
        feature.SetGeometry(ogr.CreateGeometryFromWkt('POINT(0 0)'))
        
        ret = layer.CreateFeature(feature)
        assert ret == 0 and feature.GetFID() >= 0

def read_datasource(sxf_name, test_data, layer_names):

    open_ds = gdal.OpenEx(sxf_name, gdal.OF_READONLY, open_options = ['SXF_NEW_BEHAVIOR=YES'])
    assert open_ds is not None
    assert open_ds.GetLayerCount() == len(layer_names) + 2 # SYSTEM and Not_Classified

    for i in range(open_ds.GetLayerCount()):
        layer = open_ds.GetLayer(i)
        if layer.GetName() not in ['SYSTEM', 'Not_Classified']:
            assert layer.GetName() in layer_names
            
            feature = layer.GetNextFeature()
            assert feature is not None
            
            for i in range(feature.GetFieldCount()):
                field = feature.GetFieldDefnRef(i)
                assert field is not None
                
                if field.GetAlternativeName() == 'STRFIELD':
                    assert feature.GetFieldAsString(i) == test_data['STRFIELD_TEST_VALUE']
                elif field.GetAlternativeName() == 'DECFIELD':
                    assert feature.GetFieldAsInteger(i) == test_data['DECFIELD_TEST_VALUE']
                elif field.GetAlternativeName() == 'REALFIELD':                    
                    assert feature.GetFieldAsDouble(i) == test_data['REALFIELD_TEST_VALUE']

def test_ogr_sxf_5():
    sxf_name = 'tmp/test_ogr_sxf_5.sxf'
    layer_names = ['ВОДНЫЕ ОБЪЕКТЫ', 'ИНФРАСТРУКТУРА']
    test_data = {
        'STRFIELD_TEST_VALUE': '123',
        'DECFIELD_TEST_VALUE': 123,
        'REALFIELD_TEST_VALUE': 1.23,
    }
    
    create_datasource(sxf_name, test_data, layer_names)
    read_datasource(sxf_name, test_data, layer_names)

###############################################################################
#


def test_ogr_sxf_cleanup():

    if gdaltest.sxf_ds is None:
        pytest.skip()

    gdaltest.sxf_ds = None



