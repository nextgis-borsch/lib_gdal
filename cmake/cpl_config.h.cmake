/* Define if you want to use pthreads based multiprocessing support */
#cmakedefine01 CPL_MULTIPROC_PTHREAD

/* Define to 1 if you have the `PTHREAD_MUTEX_RECURSIVE' constant. */
#cmakedefine01 HAVE_PTHREAD_MUTEX_RECURSIVE

/* Define to 1 if you have the `PTHREAD_MUTEX_ADAPTIVE_NP' constant. */
#cmakedefine01 HAVE_PTHREAD_MUTEX_ADAPTIVE_NP

/* Define to 1 if you have the `pthread_spinlock_t' type. */
#cmakedefine01 HAVE_PTHREAD_SPINLOCK

/* Define to 1 if you have the 5 args `mremap' function. */
#cmakedefine01 HAVE_5ARGS_MREMAP

/* Define to 1 if you have the <assert.h> header file. */
#cmakedefine01 HAVE_ASSERT_H

/* Define to 1 if you have the `atoll' function. */
#cmakedefine01 HAVE_ATOLL

/* Define to 1 if you have the <dbmalloc.h> header file. */
#cmakedefine01 HAVE_DBMALLOC_H

/* Define to 1 if you have the declaration of `strtof', and to 0 if you don't.*/
#cmakedefine01 HAVE_DECL_STRTOF

/* Define to 1 if you have the <direct.h> header file. */
#cmakedefine01 HAVE_DIRECT_H

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine01 HAVE_DLFCN_H

/* Define to 1 if you have the <errno.h> header file. */
#cmakedefine01 HAVE_ERRNO_H

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine01 HAVE_FCNTL_H

/* Define to 1 if you have the `getcwd' function. */
#cmakedefine01 HAVE_GETCWD

/* Define if you have the iconv() function and it works. */
#cmakedefine01 HAVE_ICONV

/* Define as 0 or 1 according to the floating point format suported by the
   machine */
#cmakedefine01 HAVE_IEEEFP

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine01 HAVE_INTTYPES_H

/* Define to 1 if you have the <locale.h> header file. */
#cmakedefine01 HAVE_LOCALE_H

/* Define to 1, if your compiler supports long long data type */
#cmakedefine01 HAVE_LONG_LONG

/* Define to 1 if you have the `snprintf' function. */
#cmakedefine01 HAVE_SNPRINTF

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine01 HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine01 HAVE_STDLIB_H

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine01 HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine01 HAVE_STRING_H

/* Define to 1 if you have the `strtof' function. */
#cmakedefine01 HAVE_STRTOF

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine01 HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine01 HAVE_SYS_TYPES_H

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine01 HAVE_UNISTD_H

/* Define to 1 if you have the `vsnprintf' function. */
#cmakedefine01 HAVE_VSNPRINTF

/* Define to 1 if you have the `readlink' function. */
#cmakedefine01 HAVE_READLINK

/* Define to 1 if you have the `posix_spawnp' function. */
#cmakedefine01 HAVE_POSIX_SPAWNP

/* Define to 1 if you have the `vfork' function. */
#cmakedefine01 HAVE_VFORK

/* Define to 1 if you have the `lstat' function. */
#cmakedefine01 HAVE_LSTAT

/* Set the native cpu bit order (FILLORDER_LSB2MSB or FILLORDER_MSB2LSB) */
#define HOST_FILLORDER @HOST_FILLORDER@

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST @ICONV_CONST@

/* For .cpp files, define as const if the declaration of iconv() needs const. */
#define ICONV_CPP_CONST @ICONV_CPP_CONST@

/* Define for Mac OSX Framework build */
#cmakedefine MACOSX_FRAMEWORK

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT @SIZEOF_INT@

/* The size of `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG @SIZEOF_UNSIGNED_LONG@

/* The size of `void*', as computed by sizeof. */
#define SIZEOF_VOIDP @SIZEOF_VOIDP@

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine01 STDC_HEADERS

/* Define to 1 if you have fseek64, ftell64 */
#cmakedefine01 UNIX_STDIO_64

/* Define to 1 if you want to use the -fvisibility GCC flag */
#cmakedefine01 USE_GCC_VISIBILITY_FLAG

/* Define to 1 if GCC atomic builtins are available */
#cmakedefine01 HAVE_GCC_ATOMIC_BUILTINS

/* Define to name of 64bit fopen function */
#define VSI_FOPEN64 @VSI_FOPEN64@

/* Define to name of 64bit ftruncate sfunction */
#define VSI_FTRUNCATE64 @VSI_TRANCATE64@

/* Define to name of 64bit fseek func */
#define VSI_FSEEK64 @VSI_FSEEK64@

/* Define to name of 64bit ftell func */
#define VSI_FTELL64 @VSI_FTELL64@

/* Define to 1, if you have 64 bit STDIO API */
#cmakedefine01 VSI_LARGE_API_SUPPORTED

/* Define to 1, if you have LARGEFILE64_SOURCE */
#cmakedefine01 VSI_NEED_LARGEFILE64_SOURCE

/* Define to name of 64bit stat function */
#define VSI_STAT64 @VSI_STAT64@

/* Define to name of 64bit stat structure */
#define VSI_STAT64_T @VSI_STAT64_T@

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#cmakedefine01 WORDS_BIGENDIAN

/* Use this file to override settings in instances where you're doing FAT compiles
   on Apple.  It is currently off by default because it doesn't seem to work with 
   newish ( XCode >= 3/28/11) XCodes */
/* #include "cpl_config_extras.h" */
@CPL_CONFIG_EXTRAS@

/* Define to 1 if you have the `getaddrinfo' function. */
#cmakedefine01 HAVE_GETADDRINFO

/* Define to 1 if you have the _SC_PHYS_PAGES' constant. */
#cmakedefine01 HAVE_SC_PHYS_PAGES

/* Define to 1 if you have the `uselocale' function. */
#cmakedefine01 HAVE_USELOCALE
