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
 
# Vector drivers 

Available vector drivers for now (by all driver dirs in sources):

| Driver | Cmaked  | OS tested | Dependences |
|---|:-:|---|---|
| geomedia | no |  |  |
| mem | no |  | no |
| pgdump | no |  |  |
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
| generic | no |  |  |
| geoconcept | no |  |  |
| geojson | no |  |  |
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
| mitab | no |  |  |
| mongodb | no |  |  |
| mssqlspatial | no |  |  |
| mysql | no |  |  |
| nas | no |  |  |
| ntf | no |  |  |
| oci | no |  |  |
| odbc | no |  |  |
| ods | no |  |  |
| ogdi | no |  |  |
| openair | no |  |  |
| openfilegdb | no |  |  |
| osm | no |  |  |
| pds | no |  |  |
| pg | no |  |  |
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
