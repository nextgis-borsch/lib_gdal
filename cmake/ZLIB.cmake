option(WITH_ZLIB "Set ON to use zlib" ON)

if(WITH_ZLIB)
find_package(ZLIB)

if(ZLIB_FOUND)
  set(GDAL_USES_INTERNAL_ZLIB TRUE)
else()
  set (ZLIB_SRC_DIR ${ep_base}/Source/zlib CACHE INTERNAL "zlib internal sources path")
  set (ZLIB_BLD_DIR ${ep_base}/Build/zlib CACHE INTERNAL "zlib internal build path")
  # external projext
  ExternalProject_Add(zlib
    GIT_REPOSITORY ${EP_URL}/lib_z
    INSTALL_COMMAND "" # no install
    )        
  set (ZLIB_INCLUDE_DIRS ${ZLIB_SRC_DIR} ${ZLIB_BLD_DIR})
  
  if (MSVC)
    set(ZLIB_LIBRARIES
      DEBUG           "${ZLIB_BLD_DIR}/Debug/${CMAKE_STATIC_LIBRARY_PREFIX}zlibd${CMAKE_STATIC_LIBRARY_SUFFIX}"
      RELEASE         "${ZLIB_BLD_DIR}/Release/${CMAKE_STATIC_LIBRARY_PREFIX}zlib${CMAKE_STATIC_LIBRARY_SUFFIX}"
      )
  else()
    set(ZLIB_LIBRARIES
      "${ZLIB_BLD_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX}"
      )
  endif()      
  
  #set(ZLIB_FOUND ON)
endif()

include_directories(${ZLIB_INCLUDE_DIRS})
add_definitions(-DHAVE_ZLIB_H -DHAVE_ZLIB)
if(MSVC)
  add_definitions(-DZLIB_DLL)
endif()

else()
    message(WARNING "No zlib support")
endif()
