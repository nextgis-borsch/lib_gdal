# GDAL - Geospatial Data Abstraction Library

This is restructured GDAL sources tree fork with CMake build system.

GDAL is an open source X/MIT licensed translator library for raster and vector geospatial data formats. This is a mirror of the GDAL Subversion repository.

* Main site: http://www.gdal.org - Developer and user docs, links to other resources
* SVN repository: http://svn.osgeo.org/gdal
* Download: ftp://ftp.remotesensing.org/gdal, http://download.osgeo.org/gdal
* Wiki: http://trac.osgeo.org/gdal - Bug tracking, various user and developer contributed documentation and hints
* Mailing list: http://lists.osgeo.org/mailman/listinfo/gdal-dev

# Sync sources with orgin

Clone sources

 $ git clone https://github.com/OSGeo/gdal.git gdal-git

or update sources if already clonned

 $ git pull
 
Go to script folder of this repo /etc/cmake-build-helpers and execute

 $ python ./etc/cmake-build-helpers/gdal_restructure.py /path_to_gdal/gdal-git/ ./ ./etc/cmake-build-helpers/gdal_folders.csv
 
# Notes 
