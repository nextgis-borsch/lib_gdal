# Find the MRSID library - Multi-resolution Seamless Image Database.
#
# Sets
#   MRSID_FOUND.  If false, don't try to use ecw
#   MRSID_INCLUDE_DIR
#   MRSID_LIBRARIES

FIND_PATH( MRSID_INCLUDE_DIR lt_base.h
  /usr/include
  /usr/local/include
)

IF( MRSID_INCLUDE_DIR )
  SET(SEARCH_DIRS
    /usr/lib
    /usr/local/lib
    /usr/lib64
    /usr/local/lib64
  )
  
  FIND_LIBRARY( MRSID_LIBRARY_LTI NAMES lti_dsdk
    ${SEARCH_DIRS}
  )
  
  FIND_LIBRARY( MRSID_LIBRARY_LTI_LIDAR NAMES lti_lidar_dsdk
    ${SEARCH_DIRS}
  )
    
  SET( MRSID_FOUND "NO" )

  IF( MRSID_LIBRARY_LTI )
  IF( MRSID_LIBRARY_LTI_LIDAR )
    SET( MRSID_LIBRARIES ${MRSID_LIBRARY_LTI} ${MRSID_LIBRARY_LTI_LIDAR})
    SET( MRSID_FOUND "YES" )

  ENDIF( MRSID_LIBRARY_LTI_LIDAR )
  ENDIF( MRSID_LIBRARY_LTI )

ENDIF( MRSID_INCLUDE_DIR )

message("MRSID_FOUND: ${MRSID_FOUND}")
message("MRSID_INCLUDE_DIR: ${MRSID_INCLUDE_DIR}")
message("MRSID_LIBRARIES: ${MRSID_LIBRARIES}")