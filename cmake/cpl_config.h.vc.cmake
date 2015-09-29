
/* We define this here in general so that a VC++ build will publically
   declare STDCALL interfaces even if an application is built against it
   using MinGW */

#ifndef CPL_DISABLE_STDCALL
#  define CPL_STDCALL __stdcall
#endif

/* Define if you have the vprintf function.  */
#cmakedefine01 HAVE_VSNPRINTF
#cmakedefine01 HAVE_SNPRINTF
#if defined(_MSC_VER) && (_MSC_VER < 1500)
#  define vsnprintf _vsnprintf
#endif
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#  define snprintf _snprintf
#endif

#cmakedefine01 HAVE_GETCWD
/* gmt_notunix.h from GMT project also redefines getcwd. See #3138 */
#ifndef getcwd
#define getcwd _getcwd
#endif

/* Define if you have the ANSI C header files.  */
#ifndef STDC_HEADERS
#cmakedefine01 STDC_HEADERS
#endif

/* Define to 1 if you have the <assert.h> header file. */
#cmakedefine01 HAVE_ASSERT_H

/* Define to 1 if you have the <fcntl.h> header file.  */
#cmakedefine01 HAVE_FCNTL_H

/* Define if you have the <unistd.h> header file.  */
#cmakedefine HAVE_UNISTD_H

/* Define if you have the <stdint.h> header file.  */
#cmakedefine HAVE_STDINT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine01 HAVE_SYS_TYPES_H

/* Define to 1 if you have the <locale.h> header file. */
#cmakedefine01 HAVE_LOCALE_H

#cmakedefine01 HAVE_FLOAT_H

#cmakedefine01 HAVE_ERRNO_H

#cmakedefine01 HAVE_SEARCH_H

/* Define to 1 if you have the <direct.h> header file. */
#cmakedefine01 HAVE_DIRECT_H

/* Define to 1 if you have the `localtime_r' function. */
#cmakedefine01 HAVE_LOCALTIME_R

#cmakedefine01 HAVE_DLFCN_H
#cmakedefine01 HAVE_DBMALLOC_H
#cmakedefine01 HAVE_LIBDBMALLOC
#cmakedefine01 WORDS_BIGENDIAN

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT @SIZEOF_INT@

/* The size of a `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG @SIZEOF_UNSIGNED_LONG@

/* The size of `void*', as computed by sizeof. */
#define SIZEOF_VOIDP @SIZEOF_VOID_P@

/* Set the native cpu bit order */
#define HOST_FILLORDER @HOST_FILLORDER@

/* Define as 0 or 1 according to the floating point format suported by the
   machine */
#cmakedefine01 HAVE_IEEEFP

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#  ifndef inline
#    define inline __inline
#  endif
#endif

#define lfind _lfind

#if defined(_MSC_VER) && (_MSC_VER < 1310)
#  define VSI_STAT64 _stat
#  define VSI_STAT64_T _stat
#else
#  define VSI_STAT64 _stat64
#  define VSI_STAT64_T __stat64
#endif

/* VC6 doesn't known intptr_t */
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
    typedef int intptr_t;
#endif

/* #define CPL_DISABLE_DLL */
