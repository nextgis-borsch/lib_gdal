# import osgeo.gdal as a convenience
from osgeo.gdal import deprecation_warn
deprecation_warn('gnm')

from osgeo.gnm import *
