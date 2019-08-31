file(GLOB_RECURSE IMPORTED_WIN_DLLS ${SRC_PATH}/third-party/*.dll)
message("Copy files to ${DST_PATH}\nFiles:\n${IMPORTED_WIN_DLLS}")
foreach(IMPORTED_WIN_DLL ${IMPORTED_WIN_DLLS})
    file(COPY ${IMPORTED_WIN_DLL} DESTINATION ${DST_PATH})
endforeach()