###############################################################################
# CMake module to search for PROJ.4 library
#
# On success, the macro sets the following variables:
# PROJ4_FOUND       = if the library found
# PROJ4_LIBRARY     = full path to the library
# PROJ4_INCLUDE_DIR = where to find the library headers 
# also defined, but not for general use are
# PROJ4_LIBRARY, where to find the PROJ.4 library.
#
# Copyright (c) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
###############################################################################

# Try to use OSGeo4W installation
IF(WIN32)
    SET(PROJ4_OSGEO4W_HOME "C:/OSGeo4W") 

    IF($ENV{OSGEO4W_HOME})
        SET(PROJ4_OSGEO4W_HOME "$ENV{OSGEO4W_HOME}") 
    ENDIF()
ENDIF(WIN32)

FIND_PATH(PROJ4_INCLUDE_DIR proj_api.h
    PATHS ${PROJ4_OSGEO4W_HOME}/include
    DOC "Path to PROJ.4 library include directory")

SET(PROJ4_NAMES ${PROJ4_NAMES} proj proj_i)
FIND_LIBRARY(PROJ4_LIBRARY
    NAMES ${PROJ4_NAMES}
    PATHS ${PROJ4_OSGEO4W_HOME}/lib
    DOC "Path to PROJ.4 library file")

if(PROJ4_INCLUDE_DIR)
    set(PROJ4_VERSION_MAJOR 0)
    set(PROJ4_VERSION_MINOR 0)
    set(PROJ4_VERSION_PATCH 0)
    set(PROJ4_VERSION_NAME "EARLY RELEASE")

    if(EXISTS "${PROJ4_INCLUDE_DIR}/proj_api.h")
        file(READ "${PROJ4_INCLUDE_DIR}/proj_api.h" PROJ_API_H_CONTENTS)
        string(REGEX MATCH "PJ_VERSION[ \t]+([0-9]+)"
          PJ_VERSION ${PROJ_API_H_CONTENTS})
        string (REGEX MATCH "([0-9]+)"
          PJ_VERSION ${PJ_VERSION})

        string(SUBSTRING ${PJ_VERSION} 0 1 PROJ4_VERSION_MAJOR)
        string(SUBSTRING ${PJ_VERSION} 1 1 PROJ4_VERSION_MINOR)
        string(SUBSTRING ${PJ_VERSION} 2 1 PROJ4_VERSION_PATCH)
        unset(PROJ_API_H_CONTENTS)
    endif()
      
    set(PROJ4_VERSION_STRING "${PROJ4_VERSION_MAJOR}.${PROJ4_VERSION_MINOR}.${PROJ4_VERSION_PATCH}")   
endif ()    
         
# Handle the QUIETLY and REQUIRED arguments and set SPATIALINDEX_FOUND to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PROJ4 
                                  REQUIRED_VARS PROJ4_LIBRARY PROJ4_INCLUDE_DIR 
                                  VERSION_VAR PROJ4_VERSION_STRING)

IF(PROJ4_FOUND)
  set(PROJ4_LIBRARIES ${PROJ4_LIBRARY})
  set(PROJ4_INCLUDE_DIRS ${PROJ4_INCLUDE_DIR})
ENDIF()

# Hide internal variables
mark_as_advanced(
  PROJ4_INCLUDE_DIR
  PROJ4_LIBRARY)

#======================
