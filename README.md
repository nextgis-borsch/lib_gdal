# GDAL - Geospatial Data Abstraction Library

This is restructured GDAL sources tree fork with CMake build system [code name borsch].

GDAL is an open source X/MIT licensed translator library for raster and vector 
geospatial data formats. This is a mirror of the GDAL Subversion repository.

* Main site: http://www.gdal.org - Developer and user docs, links to other resources
* SVN repository: http://svn.osgeo.org/gdal
* Download: ftp://ftp.remotesensing.org/gdal, http://download.osgeo.org/gdal
* Wiki: http://trac.osgeo.org/gdal - Bug tracking, various user and developer 
  contributed documentation and hints
* Mailing list: http://lists.osgeo.org/mailman/listinfo/gdal-dev

# Sync sources with orgin

Clone sources

 $ git clone https://github.com/OSGeo/gdal.git gdal-git

or update sources if already clonned

 $ git pull
 
Go to script folder of this repo /etc/cmake-build-helpers and execute

 $ python ./etc/cmake-build-helpers/gdal_restructure.py /path_to_gdal/gdal-git/ ./ ./etc/cmake-build-helpers/gdal_folders.csv
 
# Build status

| OS | Status  |
|---|:-:|
| Windows | ![build status](http://176.9.38.120/buildbot/png?builder=gdal_win) |
| Ubuntu (packaging) | ![build status](http://176.9.38.120/buildbot/png?builder=gdal_deb) | 

# Ubuntu PPA

  $ sudo apt-get install software-properties-common python-software-properties
  
  $ sudo apt-add-repository ppa:nextgis/ppa
  
  $ sudo apt-get install gdal-bin python-gdal

# Test status

TODO:

# Raster drivers

Available raster drivers for now (by all driver dirs in sources). Drivers marked 
with '*' have high priority to be implemented.

| Driver | Cmaked | External dependencies | Notes |
|---|:-:|---|---|
| **aaigrid** | **yes** | no | 2 drivers in one (AAIGrid, GRASSASCIIGrid) |
| **adrg** | **yes** | no | 2 drivers in one (ADRG, SRP) |
| **aigrid** | **yes** | no | No additional build targets implemented |
| **airsar** | **yes** | no | - |
| **arg** | **yes** | no | - |
| **blx** | **yes** | no | - |
| **bmp** | **yes** | no | - |
| bpg | no |  |  |
| **bsb** | **yes** | no | No additional build targets implemented |
| **cals** | **yes** | no | - |
| **ceos** | **yes** | no | No additional build targets implemented |
| **ceos2** | **yes** | no | - |
| **coasp** | **yes** | no | - |
| **cosar** | **yes** | no | - |
| **ctg** | **yes** | no | - |
| dds | no |  |  |
| **dimap** | **yes** | no | - |
| dods | no |  |  |
| **dted** | **yes** | no | No additional build targets implemented |
| **e00grid** | **yes** | no | - |
| ecw * | no |  |  |
| **elas** | **yes** | no | - |
| **envisat** | **yes** | no | No additional build targets implemented |
| epsilon | no |  |  |
| **ers** | **yes** | no | No additional build targets implemented |
| **fit** | **yes** | no | - |
| fits | no |  |  |
| georaster | no |  |  |
| **gff** | **yes** | no | - |
| gif * | no |  | Russian maps in internet are in gif and ozi |
| grass * | no |  |  |
| **grib** | **yes** | Optionally: JASPER | No JASPER support implemented; No additional build targets implemented |
| **gsg** | **yes** | no | 3 drivers in one (GSAG, GSBG, GS7BG) |
| gta | no |  |  |
| **gtiff** | **yes** | no | Obligatory for building GDAL |
| **gxf** | **yes** | no | No additional build targets implemented |
| **hdf4** | **yes** | [lib_hdf4](https://github.com/nextgis-borsch/lib_hdf4) | 2 drivers in one (HDF4, HDF4Image); hdf-eos target implemented not as a library |
| hdf5 * | no |  |  |
| **hf2** | **yes** | no | - |
| **hfa** | **yes** | no | Obligatory for building GDAL |
| **idrisi** | **yes** | no | Has the same dir name in /vector |
| **ilwis** | **yes** | no | - |
| **ingr** | **yes** | TIFF? | - |
| **iris** | **yes** | no | - |
| **jaxapalsar** | **yes** | no | - |
| **jdem** | **yes** | no | - |
| jp2kak | no |  |  |
| **jpeg** | **yes** | JPEG, JPEG12 | No jpeg12 support implemented; | 
| jpeg2000 * | no |  |  |
| jpegls | no |  |  |
| jpipkak | no |  |  |
| kea | no |  |  |
| **kmlsuperoverlay** | **yes** | no | - |
| **l1b** | **yes** | no | - |
| **leveller** | **yes** | no | - |
| **map** | **yes** | no | - |
| **mbtiles** | **yes** | Optionally: ZLIB? | Requires built sqlite driver; Requires some other drivers? |
| **mem** | **yes** | no | Obligatory for building GDAL |
| mrsid * | no |  |  |
| mrsid_lidar * | no |  |  |
| msg | no |  |  |
| **msgn** | **yes** | no | - |
| netcdf * | no |  |  |
| **ngsgeoid** | **yes** | no | - |
| **nitf** | **yes** | Optionally: JPEG, JPEG12, TIFF? | 3 drivers in one (NITF, RPFTOC, ECRGTOC); Requires built jpeg driver; No JPEG12 support implemented; No additional build targets implemented;  |
| **northwood** | **yes** | no | 2 drivers in one (NWT_GRC, NWT_GRD) |
| ogdi | no |  |  |
| openjpeg | no |  |  |
| **ozi** | **yes** | ZLIB?, GIF? | - |
| pcidsk | no |  |  |
| pcraster | no |  |  |
| pdf * | no |  |  |
| **pds** | **yes** | no | 4 drivers in one (PDS, ISIS2, ISIS3, VICAR); Has the same dir name in /vector |
| pgchip | no |  |  |
| **plmosaic** | **yes** | CURL | - |
| **png** | **yes** | PNG |  |
| **postgisraster** | **yes** | PostgreSQL ([lib_pq](https://github.com/nextgis-borsch/lib_pq)) | - |
| **r** | **yes** | no | - |
| rasdaman | no |  |  |
| **rasterlite** | **yes** | Optionally: SPATIALITE? | Requires built sqlite driver |
| **raw** | **yes** | no | Obligatory for building GDAL |
| **rik** | **yes** | no | - |
| **rmf** | **yes** | no | - |
| **rs2** | **yes** | no | - |
| **saga** | **yes** | no | ... |
| sde * | no |  |  |
| **sdts** | **yes** | no | Requires inner sdts lib; Has the same dir name in /vector |
| **sgi** | **yes** | no | - |
| **srtmhgt** | **yes** | no | - |
| **terragen** | **yes** | no | - |
| **til** | **yes** | no | - |
| **tsx** | **yes** | no | - |
| **usgsdem** | **yes** | no | - |
| **vrt** | **yes** | no | Obligatory for building GDAL |
| **wcs** | **yes** | CURL | Adds some "HTTP Fetching Wrapper" (raster/vector driver?) |
| webp * | no |  | Need for mobile and server |
| **wms** | **yes** | CURL | - |
| **wmts** | **yes** | CURL | - |
| **xpm** | **yes** | no | - |
| **xyz** | **yes** | no | - |
| **zmap** | **yes** | no | - |

# Vector drivers 

Available vector drivers for now (by all driver dirs in sources). Drivers marked with '*' have high priority to be implemented. 

| Driver | Cmaked | External dependencies | Notes |
|---|:-:|---|---|
| **aeronavfaa** | **yes** | no | - |
| **arcgen** | **yes** | no | - |
| arcobjects | no |  |  |
| **avc** | **yes** | no | 2 drivers in one (AVCBin, AVCE00) |
| **bna** | **yes** | no | - |
| **cartodb** | **yes** | CURL | - |
| **cloudant** | **yes** | CURL | - |
| **couchdb** | **yes** | CURL | - |
| **csv** | **yes** | no | - |
| **csw** | **yes** | CURL | - |
| **dgn** | **yes** | no | No additional build targets implemented |
| dods | no |  |  |
| dwg | no | ... | Currently unsupported |
| **dxf** | **yes** | no | - |
| **edigeo** | **yes** | no | - |
| **elastic** | **yes** | CURL | - |
| filegdb * | no |  |  |
| fme | no |  |  |
| **geoconcept** | **yes** | no | - |
| **geojson** | **yes** | no | Obligatory for building GDAL |
| geomedia | no |  |  |
| **georss** | **yes** | Optionally: EXPAT | - |
| **gft** | **yes** | CURL | - |
| gme | no |  |  |
| **gml** | **yes** | Optionally: EXPAT, XERCES, SQLITE3 | No Xerces support implemented; No additional build targets implemented |
| **gmt** | **yes** | no | - |
| **gpsbabel** | **yes** | ? | Built without GPX driver - do we need it? |
| **gpx** | **yes** | Optionally: EXPAT | - |
| grass * | no |  |  |
| **gtm** | **yes** | no | - |
| **htf** | **yes** | no | - |
| idb | no |  |  |
| **idrisi** | **yes** | no | Requires built Idrisi raster driver; Has the same dir name in /raster |
| ili | no |  |  |
| ingres | no |  |  |
| **jml** | **yes** | Optionally: EXPAT | - |
| **kml** | **yes** | ... | ... |
| libkml * | no | Google LibKML |  |
| mdb * | no |  |  |
| **mem** | **yes** | no | Obligatory for building GDAL |
| **mitab** | **yes** | no | Obligatory for building GDAL; Depends on temporary /core/ogr directory |
| mongodb * | no |  |  |
| mssqlspatial * | no |  |  |
| mysql * | no |  |  |
| nas | no |  |  |
| **ntf** | **yes** | no | No additional build targets implemented |
| null | no |  |  |
| oci | no |  |  |
| odbc * | no |  |  |
| **ods** | **yes** | EXPAT | No additional build targets implemented - workarounding of gcc bug |
| ogdi | no |  |  |
| **openair** | **yes** | no | - |
| **openfilegdb** | **yes** | no | - |
| **osm** | **yes** | SQLITE3; Optionally: EXPAT | No additional build targets implemented |
| **pds** | **yes** | no | Requires built PDS raster driver; Has the same dir name in /raster |
| **pg** | **yes** | PostgreSQL ([lib_pq](https://github.com/nextgis-borsch/lib_pq)) | - |
| **pgdump** | **yes** | no | - |
| pgeo | no |  |  |
| **rec** | **yes** | no | - |
| **s57** | **yes** | no | Requires inner iso8211 lib; No additional build targets implemented |
| sde* | no |  |  |
| **sdts** | **yes** | no | Requires inner sdts and iso8211 libs; No additional build targets implemented; Has the same dir name in /raster |
| **segukooa** | **yes** | no | - |
| **segy** | **yes** | no | - |
| **selafin** | **yes** | no | - |
| **shape** | **yes** | no | - |
| sosi | no |  |  |
| **sqlite** | **yes** | SQLITE3; Optionally: SPATIALITE - [lib_spatialite](https://github.com/nextgis-borsch/lib_spatialite), PCRE | No Spatialite and PCRE support implemented; No additional build targets implemented |
| **sua** | **yes** | no | - |
| **svg** | **yes** | EXPAT | - |
| **sxf** | **yes** | no | ... |
| **tiger** | **yes** | no | No additional build targets implemented |
| **vdv** | **yes** | no | - |
| **vfk** | **yes** | SQLITE3 | - |
| **vrt** | **yes** | no | Obligatory for building GDAL |
| walk | no |  |  |
| **wasp** | **yes** | no | - |
| **wfs** | **yes** | CURL | Depends on temporary /core/frmts directory; Requires built wms driver |
| xls * | no | [lib_freexl](https://github.com/nextgis-borsch/lib_freexl) |  |
| **xlsx** | **yes** | EXPAT | - |
| **xplane** | **yes** | no | Obligatory for building GDAL |

# Common drivers 

Available raster+vector drivers for now (by all driver dirs in sources):

| Driver | Cmaked | External dependencies | Notes |
|---|:-:|---|---|
| **gpkg** | **yes** | SQLITE3; Optionally: SPATIALITE | Requires PNG?, JPEG?, WEBP? drivers; No Spatialite support implemented; Former OGR format |
| **plscenes** | **yes** | CURL | Former OGR format |
| cad | no | no | GSoC 2016 |

# Network drivers 

Available network (GNM) drivers for now (by all driver dirs in sources):

| Driver | Cmaked | External dependencies | Notes |
|---|:-:|---|---|
| db | no | no | Requires built PostGIS driver |
| **file** | **yes** | no | - |
