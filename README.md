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

| Driver | Cmaked  | OS tested | Dependences |
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
| idrisi | no |  |  |
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
| mem | no |  | no |
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
| pds | no |  |  |
| pgchip | no |  |  |
| plmosaic | no |  |  |
| png | no |  |  |
| postgisraster | no |  |  |
| r | no |  |  |
| rasdaman | no |  |  |
| rasterlite | no |  |  |
| raw | no |  |  |
| rik | no |  |  |
| rmf | no |  |  |
| rs2 | no |  |  |
| saga | no |  |  |
| sde | no |  |  |
| sdts | no |  |  |
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

| Driver | Cmaked  | OS tested | Dependences |
|---|:-:|---|---|
| aeronavfaa | no |  |  |
| arcgen | no |  |  |
| arcobjects | no |  |  |
| avc | no |  |  |
| bna | no |  |  |
| cartodb | no |  |  |
| cloudant | no |  |  |
| couchdb | no |  |  |
| csv | no |  |  |
| csw | no |  |  |
| dgn | no |  |  |
| dods | no |  |  |
| dwg | no |  |  |
| dxf | no |  |  |
| edigeo | no |  |  |
| elastic | no |  |  |
| filegdb | no |  |  |
| fme | no |  |  |
| geoconcept | no |  |  |
| geojson | no |  |  |
| geomedia | no |  |  |
| georss | no |  |  |
| gft | no |  |  |
| gme | no |  |  |
| gml | no |  |  |
| gmt | no |  |  |
| gpkg | no |  |  |
| gpsbabel | no |  |  |
| gpx | no |  |  |
| grass | no |  |  |
| gtm | no |  |  |
| htf | no |  |  |
| idb | no |  |  |
| idrisi | no |  |  |
| ili | no |  |  |
| ingres | no |  |  |
| jml | no |  |  |
| kml | no |  |  |
| libkml | no |  |  |
| mdb | no |  |  |
| mem | no |  | no |
| mitab | no |  |  |
| mongodb | no |  |  |
| mssqlspatial | no |  |  |
| mysql | no |  |  |
| nas | no |  |  |
| ntf | no |  |  |
| null | no |  |  |
| oci | no |  |  |
| odbc | no |  |  |
| ods | no |  |  |
| ogdi | no |  |  |
| openair | no |  |  |
| openfilegdb | no |  |  |
| osm | no |  |  |
| pds | no |  |  |
| pg | no |  |  |
| pgdump | no |  |  |
| pgeo | no |  |  |
| plscenes | no |  |  |
| rec | no |  |  |
| s57 | no |  |  |
| sde | no |  |  |
| sdts | no |  |  |
| segukooa | no |  |  |
| segy | no |  |  |
| selafin | no |  |  |
| shape | no |  | no |
| sosi | no |  |  |
| sqlite | no |  |  |
| sua | no |  |  |
| svg | no |  |  |
| sxf | no |  |  |
| tiger | no |  |  |
| vfk | no |  |  |
| vrt | no |  |  |
| walk | no |  |  |
| wasp | no |  |  |
| wfs | no |  |  |
| xls | no |  |  |
| xlsx | no |  |  |
| xplane | no |  |  |
