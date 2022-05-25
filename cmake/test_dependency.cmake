LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

set(repo_bin_type repka)
set(repo_bin_url https://rm.nextgis.com)
set(repo_bin_id 2)

function(get_binary_package url repo repo_type repo_id compiler download_url name)

    if(NOT EXISTS ${DST_PATH}/${repo}_latest.json)
        file(DOWNLOAD
            ${url}/api/repo/${repo_id}/borsch?packet_name=${repo}&release_tag=latest
            ${DST_PATH}/${repo}_latest.json
            TLS_VERIFY OFF
        )
    endif()
    # Get assets files.
    file(READ ${DST_PATH}/${repo}_latest.json _JSON_CONTENTS)

    include(JSONParser)
    sbeParseJson(api_request _JSON_CONTENTS)
    foreach(asset_id ${api_request.files})
        string(FIND ${api_request.files_${asset_id}.name} "${compiler}.zip" IS_FOUND)
        # In this case we get static and shared. Add one more check.
        string(FIND ${api_request.files_${asset_id}.name} "static-${compiler}.zip" IS_FOUND_STATIC)
        if(IS_FOUND_STATIC GREATER 0)
            continue()
        endif()
        if(IS_FOUND GREATER 0)
            message("Found binary package ${api_request.files_${asset_id}.name}")
            set(${download_url} ${url}/api/asset/${api_request.files_${asset_id}.id}/download PARENT_SCOPE)
            string(REPLACE ".zip" "" FOLDER_NAME ${api_request.files_${asset_id}.name} )
            set(${name} ${FOLDER_NAME} PARENT_SCOPE)
            break()
        endif()
    endforeach()

endfunction()

set(repo_bin ${REPKA_PACKAGE})

get_binary_package(${repo_bin_url} ${repo_bin} ${repo_bin_type} ${repo_bin_id} ${COMPILER} BINARY_URL BINARY_NAME)

if(BINARY_URL)
    # Download binary build files.
    if(NOT EXISTS ${SRC_PATH}/${repo_bin}.zip)
        file(DOWNLOAD
            ${BINARY_URL}
            ${SRC_PATH}/${repo_bin}.zip
            TLS_VERIFY OFF
        )
    endif()

    # Extact files.
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DST_PATH}
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xfz ${SRC_PATH}/${repo_bin}.zip
        WORKING_DIRECTORY ${DST_PATH}
    )

    if(WIN32)
        file(GLOB_RECURSE IMPORTED_WIN_DLLS ${DST_PATH}/${BINARY_NAME}/bin/*.dll)
        message("Copy files to ${DST_PATH}\nFiles:\n${IMPORTED_WIN_DLLS}")
        foreach(IMPORTED_WIN_DLL ${IMPORTED_WIN_DLLS})
            file(COPY ${IMPORTED_WIN_DLL} DESTINATION ${DST_PATH})
        endforeach()
    elseif(OSX_FRAMEWORK)
        file(GLOB_RECURSE _FRAMEWORKS LIST_DIRECTORIES true ${DST_PATH}/${BINARY_NAME}/Library/Frameworks/*.framework)
        foreach(_FRAMEWORK ${_FRAMEWORKS})
            get_filename_component(FRAMEWORK_NAME ${_FRAMEWORK} NAME)
            if(FRAMEWORK_NAME MATCHES ".*framework")
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E rename ${_FRAMEWORK} ${DST_PATH}/${FRAMEWORK_NAME} 
                    WORKING_DIRECTORY ${DST_PATH}
                )
            endif()
        endforeach()
    endif()
endif()
