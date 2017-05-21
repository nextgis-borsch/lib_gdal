# File found at: https://svn.osgeo.org/ossim/trunk/ossim_package_support/cmake/CMakeModules/FindOpenJPEG.cmake
# Modified for NextGIS Borsch project.

###
# File:  FindOpenJPEG.cmake
#
# Original script was copied from:
# http://code.google.com/p/emeraldviewer/source/browse/indra/cmake
###

# - Find OpenJPEG
# Find the OpenJPEG includes and library
# This module defines
#  OPENJPEG_INCLUDE_DIR, where to find openjpeg.h, etc.
#  OPENJPEG_LIBRARIES, the libraries needed to use OpenJPEG.
#  OPENJPEG_FOUND, If false, do not try to use OpenJPEG.
# also defined, but not for general use are
#  OPENJPEG_LIBRARY, where to find the OpenJPEG library.

FIND_PATH(OPENJPEG_INCLUDE_DIR openjpeg.h
  PATHS
    /usr/local/include/openjpeg
    /usr/local/include
    /usr/include/openjpeg
    /usr/include
  PATH_SUFFIXES
    openjpeg-2.0
    openjpeg-2.1
    openjpeg-2.2
  DOC "Location of OpenJPEG Headers"
)

SET(OPENJPEG_NAMES ${OPENJPEG_NAMES} openjp2 openjpeg)
FIND_LIBRARY(OPENJPEG_LIBRARY
  NAMES ${OPENJPEG_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF (OPENJPEG_LIBRARY AND OPENJPEG_INCLUDE_DIR)
    SET(OPENJPEG_LIBRARIES ${OPENJPEG_LIBRARY})
    SET(OPENJPEG_FOUND "YES")
ELSE (OPENJPEG_LIBRARY AND OPENJPEG_INCLUDE_DIR)
  SET(OPENJPEG_FOUND "NO")
ENDIF (OPENJPEG_LIBRARY AND OPENJPEG_INCLUDE_DIR)

if(OPENJPEG_INCLUDE_DIR)
    set(MAJOR_VERSION 0)
    set(MINOR_VERSION 0)
    set(REV_VERSION 0)

    if(EXISTS "${OPENJPEG_INCLUDE_DIR}/opj_config.h")
        file(READ "${OPENJPEG_INCLUDE_DIR}/proj_api.h" VERSION_H_CONTENTS)

        string(REGEX MATCH "OPJ_VERSION_MAJOR[ \t]+([0-9]+)"
          MAJOR_VERSION ${VERSION_H_CONTENTS})
        string (REGEX MATCH "([0-9]+)"
          MAJOR_VERSION ${MAJOR_VERSION})
        string(REGEX MATCH "OPJ_VERSION_MINOR[ \t]+([0-9]+)"
          MINOR_VERSION ${VERSION_H_CONTENTS})
        string (REGEX MATCH "([0-9]+)"
          MINOR_VERSION ${MINOR_VERSION})
        string(REGEX MATCH "OPJ_VERSION_BUILD[ \t]+([0-9]+)"
          REV_VERSION ${VERSION_H_CONTENTS})
        string (REGEX MATCH "([0-9]+)"
          REV_VERSION ${REV_VERSION})

        unset(VERSION_H_CONTENTS)
    endif()

    set(OPENJPEG_VERSION_STRING "${MAJOR_VERSION}.${MINOR_VERSION}.${REV_VERSION}")
    math(OPENJPEG_VERSION_NUM "${MAJOR_VERSION} * 10000 + ${MINOR_VERSION} * 100 + ${REV_VERSION}")
endif ()

IF (OPENJPEG_FOUND)
   IF (NOT OPENJPEG_FIND_QUIETLY)
      MESSAGE(STATUS "Found OpenJPEG: ${OPENJPEG_LIBRARIES}")
   ENDIF (NOT OPENJPEG_FIND_QUIETLY)
ELSE (OPENJPEG_FOUND)
   IF (OPENJPEG_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find OpenJPEG library")
   ENDIF (OPENJPEG_FIND_REQUIRED)
ENDIF (OPENJPEG_FOUND)

MARK_AS_ADVANCED(OPENJPEG_LIBRARY OPENJPEG_INCLUDE_DIR)
