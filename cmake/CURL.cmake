option(WITH_CURL "Set ON to use libcurl" ON)

if(WITH_CURL)
  find_package(CURL)
  if(CURL_FOUND)
    set(GDAL_USES_EXTERNAL_CURL TRUE CACHE INTERNAL "compiles curl with build"  )
    include_directories(${CURL_INCLUDE_DIRS})
  else()

    ExternalProject_Add(openssl
      GIT_REPOSITORY ${EP_URL}/lib_openssl
      INSTALL_COMMAND ""
      )
    set (OPENSSL_DIR ${ep_base}/Source/openssl CACHE INTERNAL "openssl internal path")
    set (OPENSSL_INCLUDE_DIR ${OPENSSL_DIR} CACHE PATH "openssl internal include path")
    #set (OPENSSL_LIBRARIES ${OPENSSL_DIR}/lib/openssl CACHE INTERNAL "openssl internal lib path")
    #set (OPENSSL_LIBRARY ${OPENSSL_LIBRARIES} CACHE FILEPATH "openssl internal lib path")
    
    link_directories(${OPENSSL_DIR}/Install/openssl)
    
    ExternalProject_Add(curl
      DEPENDS zlib
      DEPENDS openssl
      GIT_REPOSITORY ${EP_URL}/lib_curl
      CMAKE_ARGS
      -DBUILD_CURL_TESTS=OFF
      -DCURL_DISABLE_FTP=ON
      -DCURL_DISABLE_LDAP=ON
      -DCURL_DISABLE_TELNET=ON
      -DCURL_DISABLE_DICT=ON
      -DCURL_DISABLE_FILE=ON
      -DCURL_DISABLE_TFTP=ON
      -DCURL_DISABLE_LDAPS=ON
      -DCURL_DISABLE_RTSP=ON
      -DCURL_DISABLE_PROXY=ON
      -DCURL_DISABLE_POP3=ON
      -DCURL_DISABLE_IMAP=ON
      -DCURL_DISABLE_SMTP=ON
      -DCURL_DISABLE_GOPHER=ON
      -DCURL_DISABLE_CRYPTO_AUTH=OFF
      -DENABLE_IPV6=OFF
      -DENABLE_MANUAL=OFF
      -DCMAKE_USE_OPENSSL=ON
      -DCMAKE_USE_LIBSSH2=OFF
      INSTALL_COMMAND ""
      )
        
    set (LIBCURL_DIR ${ep_base}/Source/curl CACHE INTERNAL "libcurl internal path")
    set (LIBCURL_INCLUDE_DIR ${LIBCURL_DIR} CACHE PATH "libcurl internal include path")
    #set (LIBCURL_LIBRARIES ${LIBCURL_DIR}/lib/curl CACHE INTERNAL "libcurl internal lib path")
    #set (LIBCURL_LIBRARY ${LIBCURL_LIBRARIES} CACHE FILEPATH "libcurl internal lib path")
    link_directories(${LIBCURL_DIR}/Install/curl)
  
  endif()  
else()
    message(WARNING "No curl support")
endif()
