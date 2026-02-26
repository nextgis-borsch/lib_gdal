# Distributed under the GDAL/OGR MIT style License.  See accompanying file LICENSE.TXT.

#[=======================================================================[.rst:
CheckDependentLibraries.cmake
-----------------------------

Detect GDAL dependencies and set variable HAVE_*

#]=======================================================================]

include(CheckFunctionExists)
include(CMakeDependentOption)
include(FeatureSummary)
include(DefineFindPackage2)
include(CheckSymbolExists)

option(
  GDAL_USE_EXTERNAL_LIBS
  "Whether detected external libraries should be used by default. This should be set before CMakeCache.txt is created."
  ON)

set(GDAL_USE_INTERNAL_LIBS_ALLOWED_VALUES ON OFF WHEN_NO_EXTERNAL)
set(
  GDAL_USE_INTERNAL_LIBS WHEN_NO_EXTERNAL
  CACHE STRING "Control how internal libraries should be used by default. This should be set before CMakeCache.txt is created.")
set_property(CACHE GDAL_USE_INTERNAL_LIBS PROPERTY STRINGS ${GDAL_USE_INTERNAL_LIBS_ALLOWED_VALUES})
if(NOT GDAL_USE_INTERNAL_LIBS IN_LIST GDAL_USE_INTERNAL_LIBS_ALLOWED_VALUES)
    message(FATAL_ERROR "GDAL_USE_INTERNAL_LIBS must be one of ${GDAL_USE_INTERNAL_LIBS_ALLOWED_VALUES}")
endif()

set(GDAL_IMPORT_DEPENDENCIES [[
include(CMakeFindDependencyMacro)
include("${CMAKE_CURRENT_LIST_DIR}/DefineFindPackage2.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/GdalFindModulePath.cmake")
]])
if(TARGET Threads::Threads)
  string(APPEND GDAL_IMPORT_DEPENDENCIES "find_dependency(Threads)\n")
endif()

# Check that the configuration has a valid value for INTERFACE_INCLUDE_DIRECTORIES. This aimed at avoiding issues like
# https://github.com/OSGeo/gdal/issues/5324
function (gdal_check_target_is_valid target res_var)
  get_target_property(_interface_include_directories ${target} "INTERFACE_INCLUDE_DIRECTORIES")
  if(_interface_include_directories)
    foreach(_dir IN LISTS _interface_include_directories)
      if(NOT EXISTS "${_dir}")
        message(WARNING "Target ${target} references ${_dir} as a INTERFACE_INCLUDE_DIRECTORIES, but it does not exist. Ignoring that target.")
        set(${res_var} FALSE PARENT_SCOPE)
        return()
      endif()
    endforeach()
  elseif("${target}" STREQUAL "geotiff_library" AND DEFINED GeoTIFF_INCLUDE_DIRS)
    # geotiff-config.cmake of GeoTIFF 1.7.0 doesn't define a INTERFACE_INCLUDE_DIRECTORIES
    # property, but a GeoTIFF_INCLUDE_DIRS variable.
    set_target_properties(${target} PROPERTIES
                          INTERFACE_INCLUDE_DIRECTORIES "${GeoTIFF_INCLUDE_DIRS}")
  else()
     message(WARNING "Target ${target} has no INTERFACE_INCLUDE_DIRECTORIES property. Ignoring that target.")
     set(${res_var} FALSE PARENT_SCOPE)
     return()
  endif()
  set(${res_var} TRUE PARENT_SCOPE)
endfunction()

# Package acceptance based on a candidate target list.
# If a matching target is found, sets ${name}_FOUND to TRUE,
# ${name}_INCLUDE_DIRS to "" and ${name}_LIBRARIES to the target name.
# If `REQUIRED` is used, ${name}_FOUND is set to FALSE if no target matches.
function(gdal_check_package_target name)
  if("REQUIRED" IN_LIST ARGN)
    list(REMOVE_ITEM ARGN "REQUIRED")
    set(${name}_FOUND FALSE PARENT_SCOPE)
  endif()
  foreach(target IN LISTS ARGN)
    if(TARGET ${target})
      gdal_check_target_is_valid(${target} _is_valid)
      if (_is_valid)
        set(${name}_TARGET "${target}" PARENT_SCOPE)
        set(${name}_FOUND TRUE PARENT_SCOPE)
        return()
      endif()
    endif()
  endforeach()
endfunction()

# Macro to declare a dependency on an external package.
# If not marked with the ALWAYS_ON_WHEN_FOUND option, dependencies can be
# marked for user control with either the CAN_DISABLE or DISABLED_BY_DEFAULT
# option. User control is done via a cache variable GDAL_USE_{name in upper case}
# with the default value ON for CAN_DISABLE or OFF for DISABLED_BY_DEFAULT.
# The RECOMMENDED option is used for the feature summary.
# The VERSION, CONFIG, MODULE, COMPONENTS and NAMES parameters are passed to find_package().
# Using NAMES with find_package() implies config mode. However, gdal_check_package()
# attempts another find_package() without NAMES if the config mode attempt was not
# successful, allowing a fallback to Find modules.
# The TARGETS parameter can define a list of candidate targets. If given, a
# package will only be accepted if it defines one of the given targets. The matching
# target name will be saved in ${name}_TARGET.
# The NAMES and TARGETS map to GDAL_CHECK_PACKAGE_${name}_NAMES and
# GDAL_CHECK_PACKAGE_${name}_TARGETS cache variables which can be used to
# overwrite the default config and targets names.
# The required find_dependency() commands for exported config are appended to
# the GDAL_IMPORT_DEPENDENCIES string (when BUILD_SHARED_LIBS=OFF).
macro (gdal_check_package name purpose)
  set(_options CONFIG MODULE CAN_DISABLE RECOMMENDED DISABLED_BY_DEFAULT ALWAYS_ON_WHEN_FOUND)
  set(_oneValueArgs VERSION NAMES)
  set(_multiValueArgs COMPONENTS TARGETS PATHS)
  cmake_parse_arguments(_GCP "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})
  string(TOUPPER ${name} key)
  set(_find_dependency "")
  set(_find_dependency_args "")
  if(FIND_PACKAGE2_${name}_ENABLED)
    find_package2(${name} QUIET OUT_DEPENDENCY _find_dependency)
  else()
    set(_find_anyproject_args QUIET)
    if (_GCP_VERSION AND NOT ("${name}" STREQUAL "TileDB") AND NOT ("${name}" STREQUAL "HDF5"))
      list(APPEND _find_anyproject_args VERSION ${_GCP_VERSION})
    endif ()
    if (_GCP_MODULE)
      list(APPEND _find_anyproject_args MODULE)
    endif ()
    if (_GCP_COMPONENTS)
      list(APPEND _find_anyproject_args COMPONENTS ${_GCP_COMPONENTS})
    endif ()
    if (_GCP_NAMES)
      list(APPEND _find_anyproject_args NAMES ${_GCP_NAMES})
    endif ()
    find_anyproject(${name} ${_find_anyproject_args})
    set(_gdal_pkg_found FALSE)
    if ((DEFINED ${name}_FOUND AND ${name}_FOUND) OR (DEFINED ${key}_FOUND AND ${key}_FOUND))
      set(_gdal_pkg_found TRUE)
    endif()
    if (_gdal_pkg_found)
      set(_candidate_targets ${_GCP_TARGETS})
      set(_candidate_target_vars
        "${name}_LIBRARIES"
        "${name}_LIBRARY"
        "${UPPER_NAME}_LIBRARIES"
        "${UPPER_NAME}_LIBRARY")
      foreach(_candidate_var IN LISTS _candidate_target_vars)
        if (DEFINED ${_candidate_var})
          list(APPEND _candidate_targets ${${_candidate_var}})
        endif ()
      endforeach ()

      if (_candidate_targets)
        gdal_check_package_target(${name} ${_candidate_targets})
        if ((NOT (DEFINED ${name}_FOUND AND ${name}_FOUND)) AND (DEFINED ${key}_FOUND AND ${key}_FOUND))
          gdal_check_package_target(${key} ${_candidate_targets})
          set(${name}_FOUND "${key}_FOUND")
        endif ()
        # If find_anyproject() reported the package as found but none of the
        # candidate CMake targets exists, retry with regular find_package().
        if ((NOT (DEFINED ${name}_FOUND AND ${name}_FOUND)) AND
            (NOT (DEFINED ${key}_FOUND AND ${key}_FOUND)))
          set(_gdal_pkg_found FALSE)
        endif ()
      endif ()

      if (_gdal_pkg_found)
        string(REPLACE ";" " " _find_dependency_args "${name}")
      endif ()
    endif ()
    if ((NOT (DEFINED ${name}_FOUND AND ${name}_FOUND)) AND (NOT (DEFINED ${key}_FOUND AND ${key}_FOUND)))
    set(_find_package_args)
    # For some reason passing the HDF5 version requirement cause a linking error of the libkea driver on Conda Windows builds...
    if (_GCP_VERSION AND NOT ("${name}" STREQUAL "TileDB") AND NOT ("${name}" STREQUAL "HDF5"))
      list(APPEND _find_package_args ${_GCP_VERSION})
    endif ()
    if (_GCP_CONFIG)
      list(APPEND _find_package_args CONFIG)
    endif ()
    if (_GCP_MODULE)
      list(APPEND _find_package_args MODULE)
    endif ()
    if (_GCP_COMPONENTS)
      list(APPEND _find_package_args COMPONENTS ${_GCP_COMPONENTS})
    endif ()
    if (_GCP_PATHS)
      list(APPEND _find_package_args PATHS ${_GCP_PATHS})
    endif ()
    if (_GCP_NAMES)
      set(GDAL_CHECK_PACKAGE_${name}_NAMES "${_GCP_NAMES}" CACHE STRING "Config file name for ${name}")
      mark_as_advanced(GDAL_CHECK_PACKAGE_${name}_NAMES)
    endif ()
    if (_GCP_TARGETS)
      set(GDAL_CHECK_PACKAGE_${name}_TARGETS "${_GCP_TARGETS}" CACHE STRING "Target name candidates for ${name}")
      mark_as_advanced(GDAL_CHECK_PACKAGE_${name}_TARGETS)
    endif ()
    if (GDAL_CHECK_PACKAGE_${name}_NAMES)
      find_package(${name} NAMES ${GDAL_CHECK_PACKAGE_${name}_NAMES} ${_find_package_args})
      gdal_check_package_target(${name} ${GDAL_CHECK_PACKAGE_${name}_TARGETS} REQUIRED)
      if (${name}_FOUND)
        get_filename_component(_find_dependency_args "${${name}_CONFIG}" NAME)
        string(REPLACE ";" " " _find_dependency_args "${name} ${_find_package_args} NAMES ${GDAL_CHECK_PACKAGE_${name}_NAMES} CONFIGS ${_find_dependency_args}")
      endif ()
    endif ()
    if (NOT ${name}_FOUND)
      find_package(${name} ${_find_package_args})
      if (${name}_FOUND)
        gdal_check_package_target(${name} ${GDAL_CHECK_PACKAGE_${name}_TARGETS})
      elseif (${key}_FOUND) # Some find modules do not set <Pkg>_FOUND
        gdal_check_package_target(${key} ${GDAL_CHECK_PACKAGE_${name}_TARGETS})
        set(${name}_FOUND "${key}_FOUND")
      endif ()
      if (${name}_FOUND)
        string(REPLACE ";" " " _find_dependency_args "${name} ${_find_package_args}")
      endif()
    endif ()
    endif()
  endif ()
  if (${key}_FOUND OR ${name}_FOUND)
    if(_GCP_VERSION)

      if( "${name}" STREQUAL "TileDB" AND NOT DEFINED TileDB_VERSION)
        get_property(_dirs TARGET TileDB::tiledb_shared PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
        foreach(_dir IN LISTS _dirs)
          set(TILEDB_VERSION_FILENAME "${_dir}/tiledb/tiledb_version.h")
          if(EXISTS ${TILEDB_VERSION_FILENAME})
            file(READ ${TILEDB_VERSION_FILENAME} _tiledb_version_contents)
            string(REGEX REPLACE "^.*TILEDB_VERSION_MAJOR +([0-9]+).*$" "\\1" TILEDB_VERSION_MAJOR "${_tiledb_version_contents}")
            string(REGEX REPLACE "^.*TILEDB_VERSION_MINOR +([0-9]+).*$" "\\1" TILEDB_VERSION_MINOR "${_tiledb_version_contents}")
            set(TileDB_VERSION "${TILEDB_VERSION_MAJOR}.${TILEDB_VERSION_MINOR}")
          endif()
        endforeach()
      endif()

      # Normalize version variables when find modules expose only uppercase
      # names (e.g. SQLITE3_VERSION) but the package key is mixed case
      # (e.g. SQLite3).
      if ((NOT DEFINED ${name}_VERSION OR "${${name}_VERSION}" STREQUAL "") AND
          DEFINED ${key}_VERSION AND NOT "${${key}_VERSION}" STREQUAL "")
        set(${name}_VERSION "${${key}_VERSION}")
      endif()
      if ((NOT DEFINED ${name}_VERSION_STRING OR
           "${${name}_VERSION_STRING}" STREQUAL "") AND
          DEFINED ${key}_VERSION_STRING AND NOT "${${key}_VERSION_STRING}" STREQUAL "")
        set(${name}_VERSION_STRING "${${key}_VERSION_STRING}")
      endif()
      if ((NOT DEFINED ${name}_VERSION_STR OR
           "${${name}_VERSION_STR}" STREQUAL "") AND
          DEFINED ${key}_VERSION_STR AND NOT "${${key}_VERSION_STR}" STREQUAL "")
        set(${name}_VERSION_STR "${${key}_VERSION_STR}")
      endif()
      if ((NOT DEFINED ${name}_VERSION OR "${${name}_VERSION}" STREQUAL "") AND
          DEFINED ${name}_VERSION_STRING AND NOT "${${name}_VERSION_STRING}" STREQUAL "")
        set(${name}_VERSION "${${name}_VERSION_STRING}")
      endif()
      if ((NOT DEFINED ${name}_INCLUDE_DIRS OR "${${name}_INCLUDE_DIRS}" STREQUAL "") AND
          DEFINED ${key}_INCLUDE_DIRS AND NOT "${${key}_INCLUDE_DIRS}" STREQUAL "")
        set(${name}_INCLUDE_DIRS "${${key}_INCLUDE_DIRS}")
      endif()
      if ((NOT DEFINED ${name}_INCLUDE_DIR OR "${${name}_INCLUDE_DIR}" STREQUAL "") AND
          DEFINED ${key}_INCLUDE_DIR AND NOT "${${key}_INCLUDE_DIR}" STREQUAL "")
        set(${name}_INCLUDE_DIR "${${key}_INCLUDE_DIR}")
      endif()
      if ((NOT DEFINED ${name}_LIBRARIES OR "${${name}_LIBRARIES}" STREQUAL "") AND
          DEFINED ${key}_LIBRARIES AND NOT "${${key}_LIBRARIES}" STREQUAL "")
        set(${name}_LIBRARIES "${${key}_LIBRARIES}")
      endif()
      if ((NOT DEFINED ${name}_LIBRARY OR "${${name}_LIBRARY}" STREQUAL "") AND
          DEFINED ${key}_LIBRARY AND NOT "${${key}_LIBRARY}" STREQUAL "")
        set(${name}_LIBRARY "${${key}_LIBRARY}")
      endif()

      if( "${${name}_VERSION}" STREQUAL "")
        message(WARNING "${name} has unknown version. Assuming it is at least matching the minimum version required of ${_GCP_VERSION}")
        set(HAVE_${key} ON)
      elseif( ${name}_VERSION VERSION_LESS ${_GCP_VERSION})
        message(WARNING "Ignoring ${name} because it is at version ${${name}_VERSION}, whereas the minimum version required is ${_GCP_VERSION}")
        set(HAVE_${key} OFF)
      else()
        set(HAVE_${key} ON)
      endif()
    else()
      set(HAVE_${key} ON)
    endif()
  else ()
    set(HAVE_${key} OFF)
  endif ()

  # find_anyproject() may report *_FOUND and library/include variables without
  # defining imported targets. Create compatibility imported targets expected
  # by the rest of GDAL CMake logic.
  if(HAVE_${key})
    set(_gcp_include_dirs)
    foreach(_inc_var IN ITEMS
            ${name}_INCLUDE_DIRS ${name}_INCLUDE_DIR
            ${key}_INCLUDE_DIRS ${key}_INCLUDE_DIR)
      if(DEFINED ${_inc_var} AND NOT "${${_inc_var}}" STREQUAL "")
        set(_gcp_include_dirs "${${_inc_var}}")
        break()
      endif()
    endforeach()

    set(_gcp_link_libraries)
    foreach(_lib_var IN ITEMS
            ${name}_LIBRARIES ${name}_LIBRARY
            ${key}_LIBRARIES ${key}_LIBRARY)
      if(DEFINED ${_lib_var} AND NOT "${${_lib_var}}" STREQUAL "")
        set(_gcp_link_libraries "${${_lib_var}}")
        break()
      endif()
    endforeach()
    # Some package configs provide logical library names (e.g. "webp")
    # instead of full paths. Resolve them to concrete files when possible.
    set(_gcp_pkg_dir)
    if(DEFINED ${name}_DIR AND NOT "${${name}_DIR}" STREQUAL "")
      set(_gcp_pkg_dir "${${name}_DIR}")
    elseif(DEFINED ${key}_DIR AND NOT "${${key}_DIR}" STREQUAL "")
      set(_gcp_pkg_dir "${${key}_DIR}")
    endif()
    set(_gcp_search_prefixes)
    if(_gcp_pkg_dir)
      get_filename_component(_gcp_prefix "${_gcp_pkg_dir}" DIRECTORY)
      get_filename_component(_gcp_prefix "${_gcp_prefix}" DIRECTORY)
      get_filename_component(_gcp_prefix "${_gcp_prefix}" DIRECTORY)
      list(APPEND _gcp_search_prefixes "${_gcp_prefix}")
    endif()
    if(_gcp_include_dirs)
      foreach(_gcp_include_dir IN LISTS _gcp_include_dirs)
        get_filename_component(_gcp_include_prefix "${_gcp_include_dir}" DIRECTORY)
        list(APPEND _gcp_search_prefixes "${_gcp_include_prefix}")
      endforeach()
    endif()
    list(REMOVE_DUPLICATES _gcp_search_prefixes)
    if(_gcp_search_prefixes AND _gcp_link_libraries)
      set(_gcp_link_libraries_resolved)
      foreach(_gcp_lib IN LISTS _gcp_link_libraries)
        set(_gcp_lib_resolved "${_gcp_lib}")
        if(NOT TARGET "${_gcp_lib}" AND
           NOT IS_ABSOLUTE "${_gcp_lib}" AND
           NOT "${_gcp_lib}" MATCHES "^-")
          foreach(_gcp_search_prefix IN LISTS _gcp_search_prefixes)
            set(_gcp_candidates
                "${_gcp_search_prefix}/lib/${_gcp_lib}${CMAKE_IMPORT_LIBRARY_SUFFIX}"
                "${_gcp_search_prefix}/lib/${_gcp_lib}${CMAKE_STATIC_LIBRARY_SUFFIX}"
                "${_gcp_search_prefix}/lib/lib${_gcp_lib}${CMAKE_IMPORT_LIBRARY_SUFFIX}"
                "${_gcp_search_prefix}/lib/lib${_gcp_lib}${CMAKE_STATIC_LIBRARY_SUFFIX}")
            foreach(_gcp_candidate IN LISTS _gcp_candidates)
              if(EXISTS "${_gcp_candidate}")
                set(_gcp_lib_resolved "${_gcp_candidate}")
                break()
              endif()
            endforeach()
            if(IS_ABSOLUTE "${_gcp_lib_resolved}")
              break()
            endif()
          endforeach()
        endif()
        list(APPEND _gcp_link_libraries_resolved "${_gcp_lib_resolved}")
      endforeach()
      set(_gcp_link_libraries "${_gcp_link_libraries_resolved}")
    endif()
    if("${name}" STREQUAL "LibKML")
      # borsch libkml package may omit Boost include dirs from exported target.
      # Try to locate boost headers in sibling/common install prefixes.
      set(_libkml_has_boost_headers FALSE)
      if(_gcp_include_dirs)
        foreach(_inc_dir IN LISTS _gcp_include_dirs)
          if(EXISTS "${_inc_dir}/boost/intrusive_ptr.hpp")
            set(_libkml_has_boost_headers TRUE)
            break()
          endif()
        endforeach()
      endif()
      if(NOT _libkml_has_boost_headers)
        set(_libkml_extra_include_dirs)
        foreach(_gcp_search_prefix IN LISTS _gcp_search_prefixes)
          foreach(_candidate_include IN ITEMS
                  "${_gcp_search_prefix}/include"
                  "${_gcp_search_prefix}/../include")
            if(EXISTS "${_candidate_include}/boost/intrusive_ptr.hpp")
              list(APPEND _libkml_extra_include_dirs "${_candidate_include}")
            endif()
          endforeach()
        endforeach()
        # Fallback for borsch layout where common headers are installed in
        # <build>/third-party/install/include rather than package prefix.
        if(EXISTS "${CMAKE_BINARY_DIR}/third-party/install/include/boost/intrusive_ptr.hpp")
          list(APPEND _libkml_extra_include_dirs "${CMAKE_BINARY_DIR}/third-party/install/include")
        endif()
        if(_libkml_extra_include_dirs)
          if(_gcp_include_dirs)
            list(APPEND _gcp_include_dirs ${_libkml_extra_include_dirs})
          else()
            set(_gcp_include_dirs ${_libkml_extra_include_dirs})
          endif()
          list(REMOVE_DUPLICATES _gcp_include_dirs)
        endif()
        unset(_libkml_extra_include_dirs)
      endif()
      unset(_libkml_has_boost_headers)
    endif()

    set(_gcp_candidate_targets ${name}::${name} ${key}::${name})
    if("${name}" STREQUAL "SQLite3")
      list(APPEND _gcp_candidate_targets SQLite::SQLite3)
    elseif("${name}" STREQUAL "CURL")
      list(APPEND _gcp_candidate_targets CURL::libcurl)
    endif()
    list(REMOVE_DUPLICATES _gcp_candidate_targets)

    foreach(_gcp_target IN LISTS _gcp_candidate_targets)
      if(NOT TARGET ${_gcp_target})
        add_library(${_gcp_target} INTERFACE IMPORTED)
      endif()
      if(_gcp_link_libraries)
        set(_gcp_target_link_libraries "${_gcp_link_libraries}")
        list(REMOVE_ITEM _gcp_target_link_libraries ${_gcp_target})
        if(_gcp_target_link_libraries)
          # *_LIBRARIES variables may contain debug/optimized/general keywords.
          # Those keywords are valid for target_link_libraries(), but not when
          # assigned directly to INTERFACE_LINK_LIBRARIES.
          target_link_libraries(${_gcp_target} INTERFACE ${_gcp_target_link_libraries})
        endif()
      endif()
      if(_gcp_include_dirs)
        set_target_properties(${_gcp_target} PROPERTIES
                              INTERFACE_INCLUDE_DIRECTORIES "${_gcp_include_dirs}")
      endif()
      if((NOT DEFINED ${name}_TARGET OR "${${name}_TARGET}" STREQUAL "") AND TARGET ${_gcp_target})
        set(${name}_TARGET "${_gcp_target}")
      endif()
    endforeach()
    if(DEFINED ${name}_TARGET AND NOT "${${name}_TARGET}" STREQUAL "")
      set(${name}_TARGET "${${name}_TARGET}")
    endif()
    if("${name}" STREQUAL "GEOS")
      # Prefer GEOS C API target for GDAL/test linkage. Some external GEOS
      # packages export plain geos/geos_c targets, while GDAL expects
      # namespaced GEOS::geos_c.
      if(TARGET geos_c AND NOT TARGET GEOS::geos_c)
        add_library(GEOS::geos_c INTERFACE IMPORTED)
        set_target_properties(GEOS::geos_c PROPERTIES
                              INTERFACE_LINK_LIBRARIES "geos_c")
        if(_gcp_include_dirs)
          set_target_properties(GEOS::geos_c PROPERTIES
                                INTERFACE_INCLUDE_DIRECTORIES "${_gcp_include_dirs}")
        endif()
      endif()
      if(TARGET geos AND NOT TARGET GEOS::GEOS)
        add_library(GEOS::GEOS INTERFACE IMPORTED)
        set_target_properties(GEOS::GEOS PROPERTIES
                              INTERFACE_LINK_LIBRARIES "geos")
        if(_gcp_include_dirs)
          set_target_properties(GEOS::GEOS PROPERTIES
                                INTERFACE_INCLUDE_DIRECTORIES "${_gcp_include_dirs}")
        endif()
      endif()
      if(TARGET GEOS::geos_c)
        set(${name}_TARGET "GEOS::geos_c")
      elseif(TARGET geos_c)
        set(${name}_TARGET "geos_c")
      endif()
    endif()
    if("${name}" STREQUAL "LibKML")
      # borsch KML package exports a single "kml" target, while GDAL's
      # libkml driver expects component targets like LIBKML::DOM/ENGINE.
      # Provide compatibility imported interface targets that forward to
      # the detected base LibKML target.
      set(_libkml_base_target)
      foreach(_libkml_candidate IN ITEMS LIBKML::LibKML LibKML::LibKML kml)
        if(TARGET ${_libkml_candidate})
          set(_libkml_base_target "${_libkml_candidate}")
          break()
        endif()
      endforeach()
      if(_libkml_base_target)
        set(_libkml_include_dirs)
        if(TARGET ${_libkml_base_target})
          get_target_property(_libkml_include_dirs ${_libkml_base_target} INTERFACE_INCLUDE_DIRECTORIES)
        endif()
        if((NOT _libkml_include_dirs) AND _gcp_include_dirs)
          set(_libkml_include_dirs "${_gcp_include_dirs}")
        endif()
        if(EXISTS "${CMAKE_BINARY_DIR}/third-party/install/include/boost/intrusive_ptr.hpp")
          list(APPEND _libkml_include_dirs "${CMAKE_BINARY_DIR}/third-party/install/include")
        endif()
        if(_libkml_include_dirs)
          list(REMOVE_DUPLICATES _libkml_include_dirs)
        endif()
        if(TARGET LIBKML::LibKML AND _libkml_include_dirs)
          set_target_properties(LIBKML::LibKML PROPERTIES
                                INTERFACE_INCLUDE_DIRECTORIES "${_libkml_include_dirs}")
        endif()
        foreach(_libkml_component IN ITEMS DOM ENGINE)
          if(NOT TARGET LIBKML::${_libkml_component})
            add_library(LIBKML::${_libkml_component} INTERFACE IMPORTED)
          endif()
          set_target_properties(LIBKML::${_libkml_component} PROPERTIES
                                INTERFACE_LINK_LIBRARIES "${_libkml_base_target}")
          if(_libkml_include_dirs)
            set_target_properties(LIBKML::${_libkml_component} PROPERTIES
                                  INTERFACE_INCLUDE_DIRECTORIES "${_libkml_include_dirs}")
          endif()
        endforeach()
        unset(_libkml_include_dirs)
      endif()
      unset(_libkml_base_target)
    endif()
    if("${name}" STREQUAL "Iconv" AND _gcp_link_libraries)
      set(Iconv_IS_BUILT_IN FALSE)
      set(Iconv_IS_BUILT_IN FALSE)
    endif()
    if("${name}" STREQUAL "SQLite3")
      # Some external SQLite3 packages (e.g. borsch SQLITE3Config.cmake)
      # do not export SQLite3_HAS_* feature variables expected by GDAL.
      if(NOT DEFINED SQLite3_HAS_MUTEX_ALLOC OR
         NOT DEFINED SQLite3_HAS_COLUMN_METADATA OR
         NOT DEFINED SQLite3_HAS_RTREE)
        set(_sqlite3_includes)
        foreach(_sqlite3_inc_var IN ITEMS SQLite3_INCLUDE_DIRS SQLite3_INCLUDE_DIR SQLITE3_INCLUDE_DIRS SQLITE3_INCLUDE_DIR)
          if(DEFINED ${_sqlite3_inc_var} AND NOT "${${_sqlite3_inc_var}}" STREQUAL "")
            set(_sqlite3_includes "${${_sqlite3_inc_var}}")
            break()
          endif()
        endforeach()
        if((NOT _sqlite3_includes) AND DEFINED SQLite3_TARGET AND TARGET ${SQLite3_TARGET})
          get_target_property(_sqlite3_target_includes ${SQLite3_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
          if(_sqlite3_target_includes)
            set(_sqlite3_includes "${_sqlite3_target_includes}")
          endif()
        endif()

        set(_sqlite3_libs)
        foreach(_sqlite3_lib_var IN ITEMS SQLite3_LIBRARIES SQLite3_LIBRARY SQLITE3_LIBRARIES SQLITE3_LIBRARY)
          if(DEFINED ${_sqlite3_lib_var} AND NOT "${${_sqlite3_lib_var}}" STREQUAL "")
            set(_sqlite3_libs "${${_sqlite3_lib_var}}")
            break()
          endif()
        endforeach()
        if((NOT _sqlite3_libs) AND DEFINED SQLite3_TARGET AND TARGET ${SQLite3_TARGET})
          set(_sqlite3_libs "${SQLite3_TARGET}")
        endif()

        if(_sqlite3_includes AND _sqlite3_libs)
          set(_sqlite3_required_includes_old "${CMAKE_REQUIRED_INCLUDES}")
          set(_sqlite3_required_libraries_old "${CMAKE_REQUIRED_LIBRARIES}")
          set(CMAKE_REQUIRED_INCLUDES "${_sqlite3_includes}")
          set(CMAKE_REQUIRED_LIBRARIES "${_sqlite3_libs}")
          if(NOT DEFINED SQLite3_HAS_MUTEX_ALLOC)
            check_symbol_exists(sqlite3_mutex_alloc sqlite3.h SQLite3_HAS_MUTEX_ALLOC)
          endif()
          if(NOT DEFINED SQLite3_HAS_COLUMN_METADATA)
            check_symbol_exists(sqlite3_column_table_name sqlite3.h SQLite3_HAS_COLUMN_METADATA)
          endif()
          if(NOT DEFINED SQLite3_HAS_RTREE)
            check_symbol_exists(sqlite3_rtree_query_callback sqlite3.h SQLite3_HAS_RTREE)
          endif()
          set(CMAKE_REQUIRED_INCLUDES "${_sqlite3_required_includes_old}")
          set(CMAKE_REQUIRED_LIBRARIES "${_sqlite3_required_libraries_old}")
          unset(_sqlite3_required_includes_old)
          unset(_sqlite3_required_libraries_old)
        endif()
      endif()
    endif()
  endif()

  if (purpose STREQUAL "")

  else ()
    if (_GCP_RECOMMENDED)
      set_package_properties(
        ${name} PROPERTIES
        PURPOSE ${purpose}
        TYPE RECOMMENDED)
    else ()
      set_package_properties(${name} PROPERTIES PURPOSE ${purpose})
    endif ()
  endif ()

  if (_GCP_CAN_DISABLE OR _GCP_DISABLED_BY_DEFAULT)
    set(_gcpp_status ON)
    if (GDAL_USE_${key})
      if (NOT HAVE_${key})
        message(FATAL_ERROR "Configured to use ${key}, but not found")
      endif ()
    elseif (NOT GDAL_USE_EXTERNAL_LIBS)
      set(_gcpp_status OFF)
      if (HAVE_${key} AND NOT GDAL_USE_${key})
        message(STATUS
          "${key} has been found, but is disabled due to GDAL_USE_EXTERNAL_LIBS=OFF. Enable it by setting GDAL_USE_${key}=ON"
          )
        set(_find_dependency_args "")
      endif ()
    endif ()
    if (_gcpp_status AND _GCP_DISABLED_BY_DEFAULT)
      set(_gcpp_status OFF)
      if (HAVE_${key} AND NOT GDAL_USE_${key})
        message(STATUS "${key} has been found, but is disabled by default. Enable it by setting GDAL_USE_${key}=ON")
        set(_find_dependency_args "")
      endif ()
    endif ()
    cmake_dependent_option(GDAL_USE_${key} "Set ON to use ${key}" ${_gcpp_status} "HAVE_${key}" OFF)
  elseif (NOT _GCP_ALWAYS_ON_WHEN_FOUND)
    message(FATAL_ERROR "Programming error: missing CAN_DISABLE or DISABLED_BY_DEFAULT option for component ${name}")
  endif ()

  if(_find_dependency_args)
    string(REPLACE "\"" "\\\"" _find_dependency_args "${_find_dependency_args}")
    set(_find_dependency "find_dependency(${_find_dependency_args})\n")
  endif()
  if(NOT BUILD_SHARED_LIBS AND GDAL_USE_${key} AND _find_dependency)
    string(APPEND GDAL_IMPORT_DEPENDENCIES "${_find_dependency}")
  endif()
  unset(_find_dependency_args)
  unset(_find_dependency)
endmacro ()

function (split_libpath _lib)
  if (_lib)
    # split lib_line into -L and -l linker options
    get_filename_component(_path ${${_lib}} PATH)
    get_filename_component(_name ${${_lib}} NAME_WE)
    string(REGEX REPLACE "^lib" "" _name ${_name})
    set(${_lib} -L${_path} -l${_name})
  endif ()
endfunction ()

function (gdal_internal_library libname)
  set(_options REQUIRED)
  set(_oneValueArgs)
  set(_multiValueArgs)
  cmake_parse_arguments(_GIL "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})
  if ("${GDAL_USE_INTERNAL_LIBS}" STREQUAL "ON")
      set(_default_value ON)
  elseif ("${GDAL_USE_INTERNAL_LIBS}" STREQUAL "OFF")
      set(_default_value OFF)
  elseif( GDAL_USE_${libname} )
      set(_default_value OFF)
  else()
      set(_default_value ON)
  endif()
  set(GDAL_USE_${libname}_INTERNAL
      ${_default_value}
      CACHE BOOL "Use internal ${libname} copy (if set to ON, has precedence over GDAL_USE_${libname})")
  if (_GIL_REQUIRED
      AND (NOT GDAL_USE_${libname})
      AND (NOT GDAL_USE_${libname}_INTERNAL))
    message(FATAL_ERROR "GDAL_USE_${libname} or GDAL_USE_${libname}_INTERNAL must be set to ON")
  endif ()
endfunction ()

# vim: ts=4 sw=4 sts=4 et
