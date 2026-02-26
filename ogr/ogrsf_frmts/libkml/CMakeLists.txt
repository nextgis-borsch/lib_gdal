set(_LIBKML_BOOST_INCLUDE_HINTS
    "${CMAKE_BINARY_DIR}/third-party/install/include"
    "${CMAKE_BINARY_DIR}/../build_/third-party/install/include")
# Guard against stale cache entry from another build tree/configuration.
if(DEFINED LIBKML_BOOST_INCLUDE_DIR AND
   NOT EXISTS "${LIBKML_BOOST_INCLUDE_DIR}/boost/intrusive_ptr.hpp")
  unset(LIBKML_BOOST_INCLUDE_DIR CACHE)
endif()
find_path(LIBKML_BOOST_INCLUDE_DIR
          NAMES boost/intrusive_ptr.hpp
          HINTS ${_LIBKML_BOOST_INCLUDE_HINTS})
if(NOT LIBKML_BOOST_INCLUDE_DIR)
  message(WARNING "Boost headers not found (missing boost/intrusive_ptr.hpp). Disabling ogr_LIBKML driver.")
  return()
endif()

add_gdal_driver(
  TARGET ogr_LIBKML
  SOURCES libkml_headers.h
          ogrlibkmlfield.h
          ogr_libkml.h
          ogrlibkmlfeaturestyle.h
          ogrlibkmlfeature.h
          ogrlibkmlgeometry.h
          ogrlibkmlstyle.h
          ogrlibkmldriver.cpp
          ogrlibkmlfeaturestyle.cpp
          ogrlibkmllayer.cpp
          ogrlibkmlfeature.cpp
          ogrlibkmlgeometry.cpp
          ogrlibkmlstyle.cpp
          ogrlibkmldatasource.cpp
          ogrlibkmlfield.cpp
  CORE_SOURCES
          ogrlibkmldrivercore.cpp
  PLUGIN_CAPABLE
  NO_SHARED_SYMBOL_WITH_CORE
  NO_CXX_WFLAGS_EFFCXX
  NO_WFLAG_OLD_STYLE_CAST
)
if(NOT TARGET ogr_LIBKML)
    return()
endif()

gdal_standard_includes(ogr_LIBKML)
target_include_directories(ogr_LIBKML PRIVATE ${LIBKML_BOOST_INCLUDE_DIR})
gdal_target_link_libraries(ogr_LIBKML PRIVATE LIBKML::LibKML LIBKML::DOM LIBKML::ENGINE)
if (TARGET LIBKML::MINIZIP AND TARGET LIBKML::URIPARSER AND TARGET ZLIB::ZLIB AND GDAL_USE_EXPAT)
  # Fedora Rawhide doesn't have LIBKML::MINIZIP
  gdal_target_link_libraries(ogr_LIBKML PRIVATE LIBKML::MINIZIP LIBKML::URIPARSER ZLIB::ZLIB ${EXPAT_TARGET})
endif()
