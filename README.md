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
 
# Build status

| OS | Status  |
|---|:-:|
| Windows | ![build status](http://176.9.38.120/buildbot/png?builder=makegdal_win) |
| Ubuntu | ![build status](http://176.9.38.120/buildbot/png?builder=makegdal_deb) | 

# Raster drivers

Available raster drivers for now (by all driver dirs in sources):

| Driver | Cmaked | External dependencies | Notes |
|---|:-:|---|---|
| aaigrid | no |  |  |
| adrg | no |  |  |
| aigrid | no |  |  |
| airsar | no |  |  |
| arg | no |  |  |
| blx | no |  |  |
| bmp | no |  |  |
| bpg | no |  |  |
| bsb | no |  |  |
| cals | no |  |  |
| ceos | no |  |  |
| ceos2 | no |  |  |
| coasp | no |  |  |
| cosar | no |  |  |
| ctg | no |  |  |
| dds | no |  |  |
| dimap | no |  |  |
| dods | no |  |  |
| dted | no |  |  |
| e00grid | no |  |  |
| ecw | no |  |  |
| elas | no |  |  |
| envisat | no |  |  |
| epsilon | no |  |  |
| ers | no |  |  |
| fit | no |  |  |
| fits | no |  |  |
| georaster | no |  |  |
| gff | no |  |  |
| gif | no |  |  |
| grass | no |  |  |
| grib | no |  |  |
| gsg | no |  |  |
| gta | no |  |  |
| gtiff | no |  |  |
| gxf | no |  |  |
| hdf4 | no |  |  |
| hdf5 | no |  |  |
| hf2 | no |  |  |
| hfa | no |  |  |
| **idrisi** | **yes** | no | - |
| ilwis | no |  |  |
| ingr | no |  |  |
| iris | no |  |  |
| iso8211 | no |  |  |
| jaxapalsar | no |  |  |
| jdem | no |  |  |
| jp2kak | no |  |  |
| jpeg | no |  |  | 
| jpeg2000 | no |  |  |
| jpegls | no |  |  |
| jpipkak | no |  |  |
| kea | no |  |  |
| kmlsuperoverlay | no |  |  |
| l1b | no |  |  |
| leveller | no |  |  |
| map | no |  |  |
| mbtiles | no |  |  |
| mem | no | no |  |
| mrsid | no |  |  |
| mrsid_lidar | no |  |  |
| msg | no |  |  |
| msgn | no |  |  |
| netcdf | no |  |  |
| ngsgeoid | no |  |  |
| nitf | no |  |  |
| northwood | no |  |  |
| ogdi | no |  |  |
| openjpeg | no |  |  |
| ozi | no |  |  |
| pcidsk | no |  |  |
| pcraster | no |  |  |
| pdf | no |  |  |
| **pds** | **yes** | no | 3 drivers in one (PDS, ISIS2, ISIS3) |
| pgchip | no |  |  |
| plmosaic | no |  |  |
| png | no |  |  |
| postgisraster | no |  |  |
| r | no |  |  |
| rasdaman | no |  |  |
| rasterlite | no |  |  |
| **raw** | **yes** | no | ... |
| rik | no |  |  |
| rmf | no |  |  |
| rs2 | no |  |  |
| saga | no |  |  |
| sde | no |  |  |
| **sdts** | **yes** | no | Required inner sdts lib |
| sgi | no |  |  |
| srtmhgt | no |  |  |
| terragen | no |  |  |
| til | no |  |  |
| tsx | no |  |  |
| usgsdem | no |  |  |
| vrt | no |  |  |
| wcs | no |  |  |
| webp | no |  |  |
| wms | no |  |  |
| wmts | no |  |  |
| xpm | no |  |  |
| xyz | no |  |  |
| zmap | no |  |  |

# Vector drivers 

Available vector drivers for now (by all driver dirs in sources):

| Driver | Cmaked | External dependencies | Notes |
|---|:-:|---|---|
| **aeronavfaa** | **yes** | no | - |
| **arcgen** | **yes** | no | - |
| arcobjects | no |  |  |
| **avc** | **yes** | no | 2 drivers in one (AVCBin, AVCE00) |
| **bna** | **yes** | no | - |
| cartodb | no |  |  |
| cloudant | no |  |  |
| couchdb | no |  |  |
| **csv** | **yes** | no | - |
| csw | no |  |  |
| **dgn** | **yes** | no | Built without "zip" target |
| dods | no |  |  |
| dwg | no |  |  |
| **dxf** | **yes** | no | - |
| **edigeo** | **yes** | no | - |
| elastic | no |  |  |
| filegdb | no |  |  |
| fme | no |  |  |
| **geoconcept** | **yes** | no | - |
| **geojson** | **yes** | no | Obligatory for GDAL |
| geomedia | no |  |  |
| georss | no |  |  |
| gft | no |  |  |
| gme | no |  |  |
| gml | no |  |  |
| **gmt** | **yes** | no | - |
| gpkg | no |  |  |
| gpsbabel | no |  |  |
| gpx | no |  |  |
| grass | no |  |  |
| **gtm** | **yes** | no | - |
| **htf** | **yes** | no | - |
| idb | no |  |  |
| **idrisi** | **yes** | no | Required Idrisi raster driver |
| ili | no |  |  |
| ingres | no |  |  |
| jml | no |  |  |
| **kml** | **yes** | Expat library | ... |
| libkml | no |  |  |
| mdb | no |  |  |
| **mem** | **yes** | no | Obligatory for GDAL |
| **mitab** | **yes** | no | Obligatory for GDAL |
| mongodb | no |  |  |
| mssqlspatial | no |  |  |
| mysql | no |  |  |
| nas | no |  |  |
| **ntf** | **yes** | no | Built without "ntfdump" target |
| null | no |  |  |
| oci | no |  |  |
| odbc | no |  |  |
| ods | no |  |  |
| ogdi | no |  |  |
| **openair** | **yes** | no | - |
| **openfilegdb** | **yes** | no | - |
| osm | no |  |  |
| **pds** | **yes** | no | Required PDS raster driver |
| pg | no |  |  |
| **pgdump** | **yes** | no | - |
| pgeo | no |  |  |
| plscenes | no |  |  |
| **rec** | **yes** | no | - |
| **s57** | **yes** | no | Required inner iso8211 lib; Built without some targets |
| sde | no |  |  |
| **sdts** | **yes** | no | Required inner sdts and iso8211 libs; Built without some targets |
| **segukooa** | **yes** | no | - |
| **segy** | **yes** | no | - |
| **selafin** | **yes** | no | - |
| **shape** | **yes** | no | - |
| sosi | no |  |  |
| sqlite | no |  |  |
| **sua** | **yes** | no | - |
| svg | no |  |  |
| **sxf** | **yes** | no | ... |
| **tiger** | **yes** | no | Built without "tigerinfo" target |
| vdv | no |  |  |
| vfk | no |  |  |
| **vrt** | **yes** | no | Obligatory for GDAL |
| walk | no |  |  |
| **wasp** | **yes** | no | - |
| wfs | no |  |  |
| xls | no |  |  |
| xlsx | no |  |  |
| **xplane** | **yes** | no | Obligatory for GDAL |
