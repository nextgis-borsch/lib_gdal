################################################################################
# Project:  CMake4GDAL
# Purpose:  CMake build scripts
# Author:   Dmitry Baryshnikov, polimax@mail.ru
################################################################################
# Copyright (C) 2015, NextGIS <info@nextgis.com>
# Copyright (C) 2012,2013,2014 Dmitry Baryshnikov
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
################################################################################

set_target_properties(${LIB_NAME}
    PROPERTIES PROJECT_LABEL ${PROJECT_NAME}
    VERSION ${VERSION}
    SOVERSION 1
)

#install lib and bin

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS ${LIB_NAME}
        EXPORT ${LIB_NAME}
        RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT libraries
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR} COMPONENT libraries
        LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT libraries
    )
endif()

if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL )
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ DESTINATION ${INSTALL_SHARE_DIR} COMPONENT libraries FILES_MATCHING PATTERN "*.*")
    install(FILES ${CMAKE_BINARY_DIR}/gdal.pc DESTINATION "${INSTALL_PKGCONFIG_DIR}" COMPONENT libraries)
endif()

set (CPACK_PACKAGE_NAME "${PACKAGE_NAME}")
set (CPACK_PACKAGE_VENDOR "GDAL")
set (CPACK_PACKAGE_VERSION "${VERSION}")
set (CPACK_PACKAGE_VERSION_MAJOR "${GDAL_MAJOR_VERSION}")
set (CPACK_PACKAGE_VERSION_MINOR "${GDAL_MINOR_VERSION}")
set (CPACK_PACKAGE_VERSION_PATCH "${GDAL_REV_VERSION}")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PACKAGE_NAME} Installation")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/docs/README")
set (CPACK_PACKAGE_INSTALL_DIRECTORY "${PACKAGE_NAME}")
set (CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${PACKAGE_NAME}-${VERSION}-${LIB_TYPE}")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/docs/LICENSE.TXT")
set (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/docs/NEWS")
set (CPACK_PACKAGE_RELOCATABLE TRUE)
set (CPACK_PACKAGE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/docs/data/gdalicon.png)
set (CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set (CPACK_PROJECT_CONFIG_FILE ${CMAKE_MODULE_PATH}/CPackConfig.cmake)
set (CPACK_COMPONENTS_ALL applications libraries headers documents)

if (WIN32)
  set (CPACK_GENERATOR "ZIP;NSIS")
  set (CPACK_MONOLITHIC_INSTALL ON)
  set (CPACK_NSIS_COMPONENT_INSTALL "ON")
  set (CPACK_NSIS_CONTACT "${PACKAGE_BUGREPORT}")
  set (CPACK_NSIS_MODIFY_PATH ON)
  set (CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_NAME} ${VERSION}")
else ()
  set (CPACK_GENERATOR "DEB;RPM;TGZ;ZIP") 
#  set (CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/pack")
  set (CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)

  set (CPACK_DEBIAN_COMPONENT_INSTALL ON)  
  set (CPACK_DEBIAN_PACKAGE_SECTION "Libraries")
  set (CPACK_DEBIAN_PACKAGE_MAINTAINER "${PACKAGE_BUGREPORT}")
  set (CPACK_DEBIAN_PRE_INSTALL_SCRIPT_FILE "/sbin/ldconfig")
  set (CPACK_DEBIAN_PRE_UNINSTALL_SCRIPT_FILE "/sbin/ldconfig")
  set (CPACK_DEBIAN_POST_INSTALL_SCRIPT_FILE "/sbin/ldconfig")
  set (CPACK_DEBIAN_POST_UNINSTALL_SCRIPT_FILE "/sbin/ldconfig")
      
  set (CPACK_RPM_COMPONENT_INSTALL ON)
  set (CPACK_RPM_PACKAGE_GROUP "Development/Tools")
  set (CPACK_RPM_PACKAGE_LICENSE "X/MIT")
  set (CPACK_RPM_PACKAGE_URL "${PACKAGE_URL}")
  set (CPACK_RPM_PRE_INSTALL_SCRIPT_FILE "/sbin/ldconfig")
  set (CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "/sbin/ldconfig")
  set (CPACK_RPM_POST_INSTALL_SCRIPT_FILE "/sbin/ldconfig")
  set (CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "/sbin/ldconfig")
endif ()

include(InstallRequiredSystemLibraries)

include (CPack)
   
#-----------------------------------------------------------------------------
# Now list the cpack commands
#-----------------------------------------------------------------------------
cpack_add_component (applications 
    DISPLAY_NAME "${PACKAGE_NAME} utility programs" 
    DEPENDS libraries
    GROUP Applications
)
cpack_add_component (libraries 
    DISPLAY_NAME "${PACKAGE_NAME} libraries"
    GROUP Runtime
)
cpack_add_component (headers 
    DISPLAY_NAME "${PACKAGE_NAME} headers" 
    DEPENDS libraries
    GROUP Development
)
cpack_add_component (documents 
    DISPLAY_NAME "${PACKAGE_NAME} documents"
    GROUP Documents
)

   
