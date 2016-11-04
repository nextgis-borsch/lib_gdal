# Find the ECW library - Enhanced Compression Wavelets for JPEG2000.
#
# Sets
#   ECW_FOUND.  If false, don't try to use ecw
#   ECW_INCLUDE_DIR
#   ECW_LIBRARY

FIND_PATH( ECW_INCLUDE_DIR NCSECWClient.h
  /usr/include
  /usr/local/include
)

IF( ECW_INCLUDE_DIR )
  FIND_LIBRARY( ECW_LIBRARY NCSEcwd
    /usr/lib
    /usr/local/lib
    /usr/lib64
    /usr/local/lib64
  )

  IF( ECW_LIBRARY )

    SET( ECW_FOUND "YES" )
    add_definitions(-DECWSDK_VERSION=53)

  ENDIF( ECW_LIBRARY )

ENDIF( ECW_INCLUDE_DIR )

message("ECW_FOUND: ${ECW_FOUND}")
message("ECW_INCLUDE_DIR: ${ECW_INCLUDE_DIR}")
message("ECW_LIBRARY: ${ECW_LIBRARY}")