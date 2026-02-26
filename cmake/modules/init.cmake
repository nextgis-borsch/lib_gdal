list(INSERT CMAKE_MODULE_PATH 0
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/../helpers")
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../borsch/cmake/FindAnyProject.cmake")
    list(INSERT CMAKE_MODULE_PATH 0
        "${CMAKE_CURRENT_LIST_DIR}/../borsch/cmake"
        "${CMAKE_CURRENT_LIST_DIR}/../borsch/cmake/modules")
endif()
include(GdalFindModulePath)
