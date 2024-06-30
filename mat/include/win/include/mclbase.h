/*
* PUBLISHed header for libmclbase, the mclbase library.
*
* Copyright 1984-2013 The MathWorks, Inc.
*/

#if defined(_MSC_VER)
# pragma once
#endif
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3))
# pragma once
#endif

#ifndef mclbase_published_api_h
#define mclbase_published_api_h

/* SWIG can't see these definitions, which are stored in package.h, so we
   duplicate them here. */
#ifdef _MSC_VER
  #define DLL_EXPORT_SYM __declspec(dllexport)
  #define DLL_IMPORT_SYM __declspec(dllimport)
#elif __GNUC__ >= 4
  #define DLL_EXPORT_SYM __attribute__ ((visibility("default")))
  #define DLL_IMPORT_SYM __attribute__ ((visibility("default")))
#else
  #define DLL_EXPORT_SYM
  #define DLL_IMPORT_SYM
#endif


#ifndef EXTERN_C
#  ifdef __cplusplus
#    define EXTERN_C extern "C"
#  else
#    define EXTERN_C extern
#  endif
#endif

#ifndef LIBMWMCLBASE_API
#  define LIBMWMCLBASE_API DLL_IMPORT_SYM
#endif

#ifndef LIBMWMCLBASE_API_EXTERN_C
#  define LIBMWMCLBASE_API_EXTERN_C EXTERN_C LIBMWMCLBASE_API
#endif

LIBMWMCLBASE_API_EXTERN_C void mclSetLastErrIdAndMsg(const char* newid, const char* newmsg);

 
LIBMWMCLBASE_API_EXTERN_C const char* mclGetLastErrorMessage();

 
/* Get stack trace string when error happens
*/
LIBMWMCLBASE_API_EXTERN_C int mclGetStackTrace(char*** stack);

 
/* Free the stack trace string allocated earlier 
*/
LIBMWMCLBASE_API_EXTERN_C int mclFreeStackTrace(char*** stack, int nStackDepth);


#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include "tmwtypes.h"

/* Use cases for mcl runtime libraries.
 * Used to configure MVM use.
 */
#define mclUndefined 0
#define mclNoMvm 1
#define mclStandaloneApp 2
#define mclStandaloneContainer 3
#define mclJavaBuilder 4
#define mclMcc 5

/* C-compatible definition of MVM ID type. */
typedef long MVMID_t;


LIBMWMCLBASE_API_EXTERN_C void mclAcquireMutex(void);


LIBMWMCLBASE_API_EXTERN_C void mclReleaseMutex(void);


LIBMWMCLBASE_API_EXTERN_C bool mclIsMCRInitialized();


LIBMWMCLBASE_API_EXTERN_C bool mclIsJVMEnabled();


LIBMWMCLBASE_API_EXTERN_C const char* mclGetLogFileName();


LIBMWMCLBASE_API_EXTERN_C bool mclIsNoDisplaySet();


LIBMWMCLBASE_API_EXTERN_C bool mclInitializeApplication(const char** options, size_t count);


LIBMWMCLBASE_API_EXTERN_C bool mclTerminateApplication(void);


LIBMWMCLBASE_API_EXTERN_C bool mclIsMcc();


typedef int (*mclOutputHandlerFcn)(const char *s);


#include <string.h>


/* Extract the path from a file name specified by either absolute or
 * relative path. For example:
 *
 *   /home/foo/bar.exe -> /home/foo
 *   ./bar.exe -> <full path to cwd>/bar.exe
 *   bar.exe -> <empty string>
 *
 * Returns a pointer to the memory passed in by the caller. 
 */
LIBMWMCLBASE_API_EXTERN_C void separatePathName(const char *fullname, char *buf, size_t bufLen);


typedef void* HMCRINSTANCE;


LIBMWMCLBASE_API_EXTERN_C bool mclFreeStrArray(char **array, size_t elements);


#include "matrix.h"


LIBMWMCLBASE_API_EXTERN_C void mclFreeArrayList(int nargs, mxArray** ppxArgs);


LIBMWMCLBASE_API_EXTERN_C mxArray *mclCreateCellArrayFromArrayList(int narray, mxArray *parray[]);


LIBMWMCLBASE_API_EXTERN_C mxArray* mclCreateSharedCopy(mxArray* px);


LIBMWMCLBASE_API_EXTERN_C mxArray* mclCreateEmptyArray(void);


LIBMWMCLBASE_API_EXTERN_C mxArray* mclCreateSimpleFunctionHandle(mxFunctionPtr fcn);


LIBMWMCLBASE_API_EXTERN_C mxArray* mclMxSerialize(const mxArray * pa);


LIBMWMCLBASE_API_EXTERN_C mxArray* mclMxDeserialize(const void* ps, size_t len);


LIBMWMCLBASE_API_EXTERN_C void mclMxDestroyArray(mxArray* pa, bool onInterpreterThread);

LIBMWMCLBASE_API_EXTERN_C bool mclMxIsA(mxArray* pa, const char *cname);

LIBMWMCLBASE_API_EXTERN_C bool mclMxIsRef(mxArray* pa);

LIBMWMCLBASE_API_EXTERN_C bool mclMxRefIsA(mxArray* pa,
                                           const char *cname);

LIBMWMCLBASE_API_EXTERN_C mxArray* mclMxReleaseRef(mxArray * pa);

LIBMWMCLBASE_API_EXTERN_C MVMID_t mclMxRefLocalMvm(mxArray *pa);

LIBMWMCLBASE_API_EXTERN_C int mclMxEnterNewArrayListContext(void);

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*mclMxArrayApplyFcn)(mxArray *);
#ifdef __cplusplus
}
#endif


LIBMWMCLBASE_API_EXTERN_C void mclMxApplyToAllArraysOnArrayList(mclMxArrayApplyFcn pfcn);


LIBMWMCLBASE_API_EXTERN_C void mclMxExitArrayListContext(int prev_context_offset, mxArray **foldList, int listLength, bool bIgnored);


LIBMWMCLBASE_API_EXTERN_C void mclMakeMxArrayLocalScope(mxArray *p);


/* Main functions passed to mclRunMain must be of this type. This typedef
 * must be placed OUTSIDE of an extern "C" block to ensure that it has the
 * right linkage in the automatically generated MCLMCRRT proxy API. See
 * mclmcrrt/GenLibProxyAPI.pl for more details.
 */
typedef int (*mclMainFcnType)(int, const char **);


LIBMWMCLBASE_API_EXTERN_C int mclRunMain(mclMainFcnType run_main,
               int argc,
               const char **argv);

#endif /* mclbase_h */
