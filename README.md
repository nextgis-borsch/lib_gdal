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

# License

![License](https://img.shields.io/badge/License-X/MIT-blue.svg?maxAge=2592000)

# Borsch

Borsch repository link: https://github.com/nextgis-borsch/borsch

# Sync sources with origin

Clone sources

 $ git clone https://github.com/OSGeo/gdal.git gdal-git

or update sources

 $ git pull

Go to borsch repository (opt folder) and execute

 $ python tools.py organize --src /path_to_gdal_sources/ --dst_name lib_gdal

# Build status

| OS | Status  |
|---|:-:|
| Windows | ![build status](http://176.9.38.120/buildbot/png?builder=gdal_win) |
| Ubuntu (packaging) | ![build status](http://176.9.38.120/buildbot/png?builder=gdal_deb) |
| Ubuntu (packaging dev) | ![build status](http://176.9.38.120/buildbot/png?builder=gdal_debdev) |

# Ubuntu PPA

  $ sudo apt-get install software-properties-common python-software-properties

  $ sudo apt-add-repository ppa:nextgis/ppa

  $ sudo apt-get install gdal-bin python-gdal

# Test status

TODO: Show test states here ([AppVeyor](https://www.appveyor.com/), [Travis CI](https://travis-ci.org/), etc.)

# Raster drivers

Available raster drivers for now (by all driver directories in sources). Drivers marked
with '\*' have high priority to be implemented.

| Driver | CMaked | External dependencies | Notes |
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
| derived | no |  |  |
| **dimap** | **yes** | no | - |
| dods | no |  |  |
| **dted** | **yes** | no | No additional build targets implemented |
| **e00grid** | **yes** | no | - |
| **ecw** | **yes** |  | Only Windows yet (need library repository update) |
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
| **hdf4** | **yes** | [HDF4](https://github.com/nextgis-borsch/lib_hdf4) | 2 drivers in one (HDF4, HDF4Image); [hdf-eos](https://github.com/nextgis-borsch/lib_hdfeos2) target implemented not as a library |
| hdf5 * | no |  |  |
| **hf2** | **yes** | no | - |
| **hfa** | **yes** | no | Obligatory for building GDAL |
| **idrisi** | **yes** | no | Has the same dir name in /vector |
| **ilwis** | **yes** | no | - |
| **ingr** | **yes** | [TIFF](https://github.com/nextgis-borsch/lib_tiff)? | - |
| **iris** | **yes** | no | - |
| **jaxapalsar** | **yes** | no | - |
| **jdem** | **yes** | no | - |
| jp2kak | no |  | needs Kakadu library |
| jp2lura | no |  | Requires Lurawave library |
| **jpeg** | **yes** | [JPEG, JPEG12](https://github.com/nextgis-borsch/lib_jpeg) | No jpeg12 support implemented? |
| jpeg2000 | no |  | needs libjasper |
| jpegls | no |  | needs CharLS library |
| jpipkak | no |  | needs Kakadu library |
| kea | no |  |  |
| **kmlsuperoverlay** | **yes** | no | - |
| **l1b** | **yes** | no | - |
| **leveller** | **yes** | no | - |
| **map** | **yes** | no | - |
| **mbtiles** | **yes** | Optionally: [ZLIB](https://github.com/nextgis-borsch/lib_z)? | Requires built sqlite driver; Requires some other drivers? |
| **mem** | **yes** | no | Obligatory for building GDAL |
| **mrsid** | **yes** |  | Only Windows yet (need library repository update) |
| mrsid_lidar * | no |  |  |
| msg | no |  |  |
| **msgn** | **yes** | no | - |
| netcdf * | no |  |  |
| **ngsgeoid** | **yes** | no | - |
| **nitf** | **yes** | Optionally: [JPEG, JPEG12](https://github.com/nextgis-borsch/lib_jpeg), [TIFF](https://github.com/nextgis-borsch/lib_tiff)? | 3 drivers in one (NITF, RPFTOC, ECRGTOC); Requires built jpeg driver; No JPEG12 support implemented; No additional build targets implemented;  |
| **northwood** | **yes** | no | 2 drivers in one (NWT_GRC, NWT_GRD) |
| ogdi | no |  |  |
| **openjpeg** | **yes** | [OPENJPEG](https://github.com/nextgis-borsch/lib_openjpeg) | This is JPEG2000 implementation; No additional build targets implemented (plugin so/dll) |
| **ozi** | **yes** | [ZLIB](https://github.com/nextgis-borsch/lib_z)?, GIF? | - |
| pcidsk | no |  |  |
| pcraster | no |  |  |
| pdf * | no |  |  |
| **pds** | **yes** | no | 4 drivers in one (PDS, ISIS2, ISIS3, VICAR); Has the same dir name in /vector |
| pgchip | no |  |  |
| **plmosaic** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | - |
| **png** | **yes** | [PNG](https://github.com/nextgis-borsch/lib_png) |  |
| **postgisraster** | **yes** | [PostgreSQL](https://github.com/nextgis-borsch/lib_pq) | - |
| **r** | **yes** | no | - |
| rasdaman | no |  |  |
| **rasterlite** | **yes** | Optionally: [SPATIALITE](https://github.com/nextgis-borsch/lib_spatialite)? | Requires built sqlite driver |
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
| **wcs** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | Adds some "HTTP Fetching Wrapper" (raster/vector driver?) |
| webp * | no |  | Need for mobile and server |
| **wms** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | - |
| **wmts** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | - |
| **xpm** | **yes** | no | - |
| **xyz** | **yes** | no | - |
| **zmap** | **yes** | no | - |

# Vector drivers

Available vector drivers for now (by all driver directories in sources). Drivers marked with '\*' have high priority to be implemented.

| Driver | CMaked | External dependencies | Notes |
|---|:-:|---|---|
| **aeronavfaa** | **yes** | no | - |
| **arcgen** | **yes** | no | - |
| arcobjects | no |  |  |
| **avc** | **yes** | no | 2 drivers in one (AVCBin, AVCE00) |
| **bna** | **yes** | no | - |
| **cartodb** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | - |
| **cloudant** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | - |
| **couchdb** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | - |
| **csv** | **yes** | no | - |
| **csw** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | - |
| **dgn** | **yes** | no | No additional build targets implemented |
| dods | no |  |  |
| dwg | no | ... | Currently unsupported |
| **dxf** | **yes** | no | - |
| **edigeo** | **yes** | no | - |
| **elastic** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | - |
| filegdb * | no |  |  |
| fme | no |  |  |
| **geoconcept** | **yes** | no | - |
| **geojson** | **yes** | no | Obligatory for building GDAL |
| geomedia | no |  |  |
| **georss** | **yes** | Optionally: [EXPAT](https://github.com/nextgis-borsch/lib_expat) | - |
| **gft** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | - |
| gme | no |  |  |
| **gml** | **yes** | Optionally: [EXPAT](https://github.com/nextgis-borsch/lib_expat), XERCES, [SQLITE3](https://github.com/nextgis-borsch/lib_sqlite) | No Xerces support implemented; No additional build targets implemented |
| gmlas | no |  | requires Xerces |
| **gmt** | **yes** | no | - |
| **gpsbabel** | **yes** | ? | Built without GPX driver - do we need it? |
| **gpx** | **yes** | Optionally: [EXPAT](https://github.com/nextgis-borsch/lib_expat) | - |
| grass * | no |  |  |
| **gtm** | **yes** | no | - |
| **htf** | **yes** | no | - |
| idb | no |  |  |
| **idrisi** | **yes** | no | Requires built Idrisi raster driver; Has the same dir name in /raster |
| ili | no |  |  |
| ingres | no |  |  |
| **jml** | **yes** | Optionally: [EXPAT](https://github.com/nextgis-borsch/lib_expat) | - |
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
| **ods** | **yes** | [EXPAT](https://github.com/nextgis-borsch/lib_expat) | No additional build targets implemented - workaround of gcc bug |
| ogdi | no |  |  |
| **openair** | **yes** | no | - |
| **openfilegdb** | **yes** | no | - |
| **osm** | **yes** | [SQLITE3](https://github.com/nextgis-borsch/lib_sqlite); Optionally: [EXPAT](https://github.com/nextgis-borsch/lib_expat) | No additional build targets implemented |
| **pds** | **yes** | no | Requires built PDS raster driver; Has the same dir name in /raster |
| **pg** | **yes** | [PostgreSQL](https://github.com/nextgis-borsch/lib_pq) | - |
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
| **sqlite** | **yes** | [SQLITE3](https://github.com/nextgis-borsch/lib_sqlite); Optionally: [SPATIALITE](https://github.com/nextgis-borsch/lib_spatialite), PCRE | No Spatialite and PCRE support implemented; No additional build targets implemented |
| **sua** | **yes** | no | - |
| **svg** | **yes** | [EXPAT](https://github.com/nextgis-borsch/lib_expat) | - |
| **sxf** | **yes** | no | ... |
| **tiger** | **yes** | no | No additional build targets implemented |
| **vdv** | **yes** | no | - |
| **vfk** | **yes** | [SQLITE3](https://github.com/nextgis-borsch/lib_sqlite) | - |
| **vrt** | **yes** | no | Obligatory for building GDAL |
| walk | no |  |  |
| **wasp** | **yes** | no | - |
| **wfs** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | Depends on temporary /core/frmts directory; Requires built WMS driver |
| xls * | no | [FreeXL](https://github.com/nextgis-borsch/lib_freexl) |  |
| **xlsx** | **yes** | [EXPAT](https://github.com/nextgis-borsch/lib_expat) | - |
| **xplane** | **yes** | no | Obligatory for building GDAL |

# Common drivers

Available raster+vector drivers for now (by all driver dirs in sources):

| Driver | CMaked | External dependencies | Notes |
|---|:-:|---|---|
| **gpkg** | **yes** | [SQLITE3](https://github.com/nextgis-borsch/lib_sqlite); Optionally: SPATIALITE | Requires [PNG](https://github.com/nextgis-borsch/lib_png)?, [JPEG](https://github.com/nextgis-borsch/lib_jpeg)?, WEBP? drivers; No Spatialite support implemented; Former OGR format |
| **plscenes** | **yes** | [CURL](https://github.com/nextgis-borsch/lib_curl) | Former OGR format |
| cad | yes | [OpenCAD](https://github.com/nextgis-borsch/lib_opencad) | GSoC 2016 |

# Network drivers

Available network (GNM) drivers for now (by all driver directories in sources):

| Driver | CMaked | External dependencies | Notes |
|---|:-:|---|---|
| db | no | no | Requires built PostGIS driver |
| **file** | **yes** | no | - |
