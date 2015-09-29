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
# include (CheckCXXSourceCompiles)
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

check_type_size ("int" SIZEOF_INT)
check_type_size ("unsigned long" SIZEOF_UNSIGNED_LONG)
check_type_size ("void*" SIZEOF_VOIDP)

#check_include_file("ieeefp.h" HAVE_IEEEFP_H)
#if(HAVE_IEEEFP_H)
    set(HAVE_IEEEFP TRUE)
#endif()


find_package(Iconv)
if(ICONV_FOUND)
    set(HAVE_ICONV ${ICONV_FOUND} CACHE INTERNAL "enable POSIX iconv support")

    if(${ICONV_SECOND_ARGUMENT_IS_CONST})
        set(ICONV_CONST "const")
    endif()

    if(${ICONV_SECOND_ARGUMENT_CPP_IS_CONST})
        set(ICONV_CPP_CONST "const")
    endif()
else()
    option(GDAL_USE_INTERNAL_ICONV "Set to ON to use internal iconv." ON)
    if(GDAL_USE_INTERNAL_ICONV)
        #todo: get iconv from remote repo
    endif()
endif()

if(NOT ${HAVE_ICONV})
    message(WARNING "No iconv support")
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

    check_c_source_compiles("
        int main() { long long off=0; }
        " HAVE_LONG_LONG)
    check_include_file("strings.h" HAVE_STRINGS_H)
    check_include_file("string.h" HAVE_STRING_H)
    check_function_exists(strtof HAVE_STRTOF)
    check_include_file("sys/stat.h" HAVE_SYS_STAT_H)
    check_function_exists(readlink HAVE_READLINK)
    check_function_exists(posix_spawnp HAVE_POSIX_SPAWNP)
    check_function_exists(vfork HAVE_VFORK)
    check_function_exists(lstat HAVE_LSTAT)

    set(CPL_CONFIG_EXTRAS "// #include \"cpl_config_extras.h\"")
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(MACOSX_FRAMEWORK TRUE)
    else()
        set(MACOSX_FRAMEWORK FALSE)
    endif()

    check_function_exists(fopen64 HAVE_FOPEN64)
    check_function_exists(stat64 HAVE_STAT64)
    if(HAVE_FOPEN64 AND HAVE_STAT64)
        set(UNIX_STDIO_64 TRUE)
        set(VSI_LARGE_API_SUPPORTED TRUE)
        set(VSI_FSEEK64 "fseeko64")
        set(VSI_FTELL64 "ftello64")
        set(VSI_FOPEN64 "fopen64")
        set(VSI_STAT64 "stat64")
        set(VSI_TRANCATE64 "ftruncate64")
    else()
        set(UNIX_STDIO_64 FALSE)
        set(VSI_LARGE_API_SUPPORTED FALSE)
        set(VSI_FSEEK64 "fseek")
        set(VSI_FTELL64 "ftell")
        set(VSI_FOPEN64 "fopen")
        set(VSI_STAT64 "stat")
        set(VSI_TRANCATE64 "ftruncate")
    endif()

    set(VSI_STAT64_T ${VSI_STAT64})

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

    check_c_source_compiles("
        #define _LARGEFILE64_SOURCE
        #include <stdio.h>
        int main() { long long off=0; fseeko64(NULL, off, SEEK_SET); off = ftello64(NULL); return 0; }
    " VSI_NEED_LARGEFILE64_SOURCE)

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

    configure_file(${CMAKE_MODULE_PATH}/cpl_config.h.cmake ${GDAL_ROOT_BINARY_DIR}/port/cpl_config.h @ONLY)
endif()

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} PARENT_SCOPE)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} PARENT_SCOPE)

message(STATUS "cpl_config.h is configured")
