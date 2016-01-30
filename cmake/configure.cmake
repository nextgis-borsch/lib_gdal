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

# Include all the necessary files for macros
include (CheckFunctionExists)
include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckLibraryExists)
include (CheckSymbolExists)
include (CheckTypeSize)
include (TestBigEndian)
include (CheckCSourceCompiles)
include (CheckCXXSourceCompiles)
# include (CompilerFlags)

check_function_exists(vsnprintf HAVE_VSNPRINTF)
check_function_exists(snprintf HAVE_SNPRINTF)
check_function_exists(getcwd HAVE_GETCWD)
check_include_file("ctype.h" HAVE_CTYPE_H)
check_include_file("stdlib.h" HAVE_STDLIB_H)

if (HAVE_CTYPE_H AND HAVE_STDLIB_H)
    set(STDC_HEADERS 1)
endif ()

check_include_file("assert.h" HAVE_ASSERT_H)
check_include_file("fcntl.h" HAVE_FCNTL_H)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file("locale.h" HAVE_LOCALE_H)
check_include_file("errno.h" HAVE_ERRNO_H)
check_include_file("direct.h" HAVE_DIRECT_H)
check_include_file("dlfcn.h" HAVE_DLFCN_H)
check_include_file("dbmalloc.h" HAVE_DBMALLOC_H)

test_big_endian(WORDS_BIGENDIAN)
if (WORDS_BIGENDIAN)
    set (HOST_FILLORDER FILLORDER_MSB2LSB)
else ()
    set (HOST_FILLORDER FILLORDER_LSB2MSB)
endif ()

check_type_size ("int" INT)
check_type_size ("unsigned long" UNSIGNED_LONG)
check_type_size ("void*" VOIDP)

#check_include_file("ieeefp.h" HAVE_IEEEFP_H)
#if(HAVE_IEEEFP_H)
    set(HAVE_IEEEFP TRUE)
#endif()

set(AVX_TEST_CODE "
    #ifdef __AVX__
    #include <immintrin.h>
    int foo() { unsigned int nXCRLow, nXCRHigh;
    __asm__ (\"xgetbv\" : \"=a\" (nXCRLow), \"=d\" (nXCRHigh) : \"c\" (0));
    float fEpsilon = 0.0000000000001f;
    __m256 ymm_small = _mm256_set_ps(fEpsilon,fEpsilon,fEpsilon,fEpsilon,fEpsilon,fEpsilon,fEpsilon,fEpsilon);
    return (int)nXCRLow + _mm256_movemask_ps(ymm_small); }
    int main(int argc, char**) { if( argc == 0 ) return foo(); return 0; }
    #else
    some_error
    #endif
    ")

check_cxx_source_compiles("${AVX_TEST_CODE}" HAVE_AVX_AT_COMPILE_TIME)

if(NOT HAVE_AVX_AT_COMPILE_TIME)   
    set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS} -mavx)
    check_cxx_source_compiles("${AVX_TEST_CODE}" HAVE_AVX_AT_COMPILE_TIME) 
    
    if(HAVE_AVX_AT_COMPILE_TIME)
        set(AVXFLAGS -mavx)
    endif()
    unset(CMAKE_REQUIRED_FLAGS)
endif()       

if(HAVE_AVX_AT_COMPILE_TIME)
    add_definitions(-DHAVE_AVX_AT_COMPILE_TIME)
endif() 

set(SSE_TEST_CODE "
    #ifdef __SSE__
    #include <xmmintrin.h>
    void foo() { float fEpsilon = 0.0000000000001f; __m128 xmm_small = _mm_load1_ps(&fEpsilon); }  int main() { return 0; }
    #else
    some_error
    #endif
")

check_cxx_source_compiles("${SSE_TEST_CODE}" HAVE_SSE_AT_COMPILE_TIME)

if(NOT HAVE_SSE_AT_COMPILE_TIME)   
    set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS} -msse)
    check_cxx_source_compiles("${SSE_TEST_CODE}" HAVE_SSE_AT_COMPILE_TIME) 
    
    if(HAVE_SSE_AT_COMPILE_TIME)
        set(SSEFLAGS -msse)
    endif()
    unset(CMAKE_REQUIRED_FLAGS)
endif()       

if(HAVE_SSE_AT_COMPILE_TIME)
    add_definitions(-DHAVE_SSE_AT_COMPILE_TIME)
endif() 

if(WIN32)
# windows
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -W4 -wd4127 -wd4251 -wd4275 -wd4786 -wd4100 -wd4245 -wd4206 -wd4018 -wd4389")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4 -wd4127 -wd4251 -wd4275 -wd4786 -wd4100 -wd4245 -wd4206 -wd4018 -wd4389")
    check_include_file("search.h" HAVE_SEARCH_H)
    check_function_exists(localtime_r HAVE_LOCALTIME_R)
    check_include_file("dbmalloc.h" HAVE_LIBDBMALLOC)
    configure_file(${CMAKE_MODULE_PATH}/cpl_config.h.vc.cmake ${GDAL_ROOT_BINARY_DIR}/port/cpl_config.h @ONLY)
else()
# linux
    option(GDAL_USE_CPL_MULTIPROC_PTHREAD "Set to ON if you want to use pthreads based multiprocessing support." ON)
    set(CPL_MULTIPROC_PTHREAD ${GDAL_USE_CPL_MULTIPROC_PTHREAD})
    check_c_source_compiles("
        #define _GNU_SOURCE
        #include <pthread.h>
        int main() { return (PTHREAD_MUTEX_RECURSIVE); }
        " HAVE_PTHREAD_MUTEX_RECURSIVE)

    check_c_source_compiles("
        #define _GNU_SOURCE
        #include <pthread.h>
        int main() { return (PTHREAD_MUTEX_ADAPTIVE_NP); }
        " HAVE_PTHREAD_MUTEX_ADAPTIVE_NP)

    check_c_source_compiles("
        #define _GNU_SOURCE
        #include <pthread.h>
        int main() { pthread_spinlock_t spin; return 1; }
        " HAVE_PTHREAD_SPINLOCK)

    check_c_source_compiles("
        #define _GNU_SOURCE
        #include <sys/mman.h>
        int main() { return (mremap(0,0,0,0,0)); }
        " HAVE_5ARGS_MREMAP)

    check_function_exists(atoll HAVE_ATOLL)
    check_function_exists(strtof HAVE_DECL_STRTOF)
    check_include_file("inttypes.h" HAVE_INTTYPES_H)

    check_type_size("long long" LONG_LONG)
        
    check_include_file("strings.h" HAVE_STRINGS_H)
    check_include_file("string.h" HAVE_STRING_H)
    
    check_function_exists(strtof HAVE_STRTOF)
    
    check_include_file("sys/stat.h" HAVE_SYS_STAT_H)
    
    check_function_exists(readlink HAVE_READLINK)
    check_function_exists(posix_spawnp HAVE_POSIX_SPAWNP)
    check_function_exists(vfork HAVE_VFORK)
    check_function_exists(lstat HAVE_LSTAT)
    
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(CPL_CONFIG_EXTRAS "#include \"cpl_config_extras.h\"")
        set(MACOSX_FRAMEWORK TRUE)
    else()
        set(MACOSX_FRAMEWORK FALSE)
    endif()

    
    check_c_source_compiles("
        #if defined(__MINGW32__)
        #ifndef __MSVCRT_VERSION__
        #define __MSVCRT_VERSION__ 0x0601
        #endif
        #endif
        #include <sys/types.h>
        #include <sys/stat.h>
        int main() { struct __stat64 buf; _stat64( \"\", &buf ); return 0; }
    " NO_UNIX_STDIO_64)
    
    if(NO_UNIX_STDIO_64)
        set(VSI_STAT64 _stat64)
        set(VSI_STAT64_T __stat64)    
    endif()
    
    check_function_exists(ftell64 HAVE_FTELL64)
    if(HAVE_FTELL64)
        set(VSI_FTELL64 "ftell64")
    else()
        check_function_exists(ftello64 HAVE_FTELLO64)
        if(HAVE_FTELLO64)
            set(VSI_FTELL64 "ftello64")
        endif()
    endif()
    
    check_function_exists(fseek64 HAVE_FSEEK64)
    if(HAVE_FSEEK64)
        set(VSI_FSEEK64 "fseek64")
    else()
        check_function_exists(fseeko64 HAVE_FSEEKO64)
        if(HAVE_FSEEKO64)
            set(VSI_FSEEK64 "fseeko64")
        endif()          
    endif()    
    
    if(NOT VSI_FTELL64 AND NOT VSI_FSEEK64)
        check_c_source_compiles("
            #define _LARGEFILE64_SOURCE
            #include <stdio.h>
            int main() { long long off=0; fseeko64(NULL, off, SEEK_SET); off = ftello64(NULL); return 0; }
        " VSI_NEED_LARGEFILE64_SOURCE)

        if(VSI_NEED_LARGEFILE64_SOURCE)
            set(VSI_FTELL64 "ftello64")
            set(VSI_FSEEK64 "fseeko64")
        endif()
    endif()    
   
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        if(NOT VSI_FTELL64 AND NOT VSI_FSEEK64)
            set(VSI_FTELL64 "ftello")
            set(VSI_FSEEK64 "fseeko")
        endif()
        set(VSI_STAT64 stat)
        set(VSI_STAT64_T stat)
    else()
        if(NOT VSI_FTELL64 AND NOT VSI_FSEEK64)
            check_function_exists(ftello HAVE_FTELLO)
            if(HAVE_FTELLO)
                set(VSI_FTELL64 "ftello")
            endif()
            
            check_function_exists(fseeko HAVE_FSEEKO)
            if(HAVE_FSEEKO)
                set(VSI_FSEEK64 "fseeko")
            endif()  
        endif()
        check_function_exists(stat64 HAVE_STAT64)
        
        if(HAVE_STAT64)
            set(VSI_STAT64 stat64)
            set(VSI_STAT64_T stat64)
        else()
            set(VSI_STAT64 stat)
            set(VSI_STAT64_T stat)
        endif()   
    endif()    
        
    check_function_exists(fopen64 HAVE_FOPEN64)
    if(HAVE_FOPEN64)
        set(VSI_FOPEN64 "fopen64")
    else()
        set(VSI_FOPEN64 "fopen")
    endif()
        
    check_function_exists(ftruncate64 HAVE_FTRUNCATE64)
    if(HAVE_FTRUNCATE64)
        set(VSI_FTRUNCATE64 "ftruncate64")
    else()
        set(VSI_FTRUNCATE64 "ftruncate")
    endif()
    
    set(UNIX_STDIO_64 TRUE)
    set(VSI_LARGE_API_SUPPORTED TRUE)

    check_c_source_compiles("
        #define _XOPEN_SOURCE 700
        #include <locale.h>
        int main() {
            locale_t alocale = newlocale (LC_NUMERIC_MASK, \"C\", 0);
            locale_t oldlocale = uselocale(alocale);
            uselocale(oldlocale);
            freelocale(alocale);
            return 0;
        }
        " HAVE_USELOCALE)

    set(CMAKE_REQUIRED_FLAGS "-fvisibility=hidden")
    check_c_source_compiles("
        int visible() { return 0; } __attribute__ ((visibility(\"default\")))
        int hidden() { return 0; }
        int main() { return 0; }
    " HAVE_HIDE_INTERNAL_SYMBOLS )

    unset(CMAKE_REQUIRED_FLAGS)
    check_c_source_compiles("
        int main() { int i; __sync_add_and_fetch(&i, 1); __sync_sub_and_fetch(&i, 1); __sync_bool_compare_and_swap(&i, 0, 1); return 0; }
    " HAVE_GCC_ATOMIC_BUILTINS )

    check_c_source_compiles("
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <netdb.h>
        int main() { getaddrinfo(0,0,0,0); return 0; }
    " HAVE_GETADDRINFO )

    check_c_source_compiles("
        #include <unistd.h>
        int main () { return (sysconf(_SC_PHYS_PAGES)); return 0; }
    " HAVE_SC_PHYS_PAGES )


    if(HAVE_HIDE_INTERNAL_SYMBOLS)
        option(GDAL_HIDE_INTERNAL_SYMBOLS "Set to ON to hide internal symbols." OFF)
        if(GDAL_HIDE_INTERNAL_SYMBOLS)
            set(USE_GCC_VISIBILITY_FLAG TRUE)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
        endif()
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2 -fPIC -fno-strict-aliasing -Wall -Wdeclaration-after-statement")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -fPIC -fno-strict-aliasing -Wall")

    configure_file(${CMAKE_MODULE_PATH}/cpl_config.h.cmake ${CMAKE_BINARY_DIR}/cpl_config.h @ONLY)
endif()

add_definitions (-DHAVE_CONFIG_H)

configure_file(${CMAKE_MODULE_PATH}/uninstall.cmake.in ${CMAKE_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY)
