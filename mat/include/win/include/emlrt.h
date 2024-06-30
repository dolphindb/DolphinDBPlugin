/*
 * PUBLISHED header for emlrt, the runtime library for MATLAB Coder
 *
 * Copyright 1984-2015 The MathWorks, Inc.
 *
 */

#ifndef emlrt_h
#define emlrt_h

#if defined(_MSC_VER)
# pragma once
#endif
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3))
# pragma once
#endif

/*
 * Only define EXTERN_C if it hasn't been defined already. This allows
 * individual modules to have more control over managing their exports.
 */
#ifndef EXTERN_C

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

#endif

#ifndef LIBEMLRT_API
#define LIBEMLRT_API
#endif

#include <setjmp.h>
#include <stdio.h>
#include "matrix.h"
#include <stdarg.h>

/*
 * MATLAB INTERNAL USE ONLY :: MEX Version
 */
#define EMLRT_VERSION_R2016A 0x2016A
#define EMLRT_VERSION_INFO  EMLRT_VERSION_R2016A

/*
 * MATLAB INTERNAL USE ONLY :: Thread local context type
 */
typedef void *emlrtCTX;
typedef const void *emlrtConstCTX;

/*
 * MATLAB INTERNAL USE ONLY :: MEX error function
 */
typedef void (*EmlrtErrorFunction)(const char* aIdentifier, const CHAR16_T* aMessage, emlrtCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Prototype of free functions.
 */
typedef void (*EmlrtHeapReferenceFreeFcn)(void *);

/*
 * MATLAB INTERNAL USE ONLY :: Prototypes of OpenMP lock functions.
 */
typedef void (*EmlrtLockeeFunction)(emlrtConstCTX aTLS, void *aData);
typedef void (*EmlrtLockerFunction)(EmlrtLockeeFunction aLockee, emlrtConstCTX aTLS, void *aData);
extern void emlrtLockerFunction(EmlrtLockeeFunction aLockee, emlrtConstCTX aTLS, void *aData);
extern emlrtCTX emlrtGetRootTLSGlobal(void);
#define emlrtCallLockeeFunction(emlrtLockeeFcnPtr,emlrtLockeeArg0,emlrtLockeeArg1) emlrtLockeeFcnPtr(emlrtLockeeArg0,emlrtLockeeArg1)

/*
 * MATLAB INTERNAL USE ONLY :: FileManager file identifier and pointer
 */
typedef int16_T emlrtFid;
typedef FILE * emlrtFilePtr;

/*
 * MATLAB INTERNAL USE ONLY :: Prototypes of file functions.
 */
typedef emlrtFilePtr(*EmlrtFopenFunction)(const char *aFname, const char *aPerm);
typedef int (*EmlrtFcloseFunction)(emlrtFilePtr aPtr);
extern emlrtFilePtr emlrtFopenFunction(const char *aFname, const char *aPerm);
extern int emlrtFcloseFunction(emlrtFilePtr aPtr);

/*
 * MATLAB INTERNAL USE ONLY :: Runtime message identifier
 */
typedef struct emlrtMsgIdentifier
{
    const char *fIdentifier;
    const struct emlrtMsgIdentifier *fParent;
    boolean_T bParentIsCell;
} emlrtMsgIdentifier;

/*
 * MATLAB INTERNAL USE ONLY :: Runtime stack info
 */
typedef struct emlrtRSInfo
{
    int32_T     lineNo;
    const char *fcnName;
    const char *pathName;
} emlrtRSInfo;

/*
 * MATLAB INTERNAL USE ONLY :: Runtime call stack
 */
typedef struct emlrtStack
{
    emlrtRSInfo             *site;
    emlrtCTX                 tls;
    const struct emlrtStack *prev;
} emlrtStack;

/*
 * MATLAB INTERNAL USE ONLY :: Runtime call stack
 */
typedef struct emlrtCallStack
{
    uint32_T     fRTStackPointer;
    uint32_T*    fRTStackSize;
} emlrtCallStack;

/*
 * MATLAB INTERNAL USE ONLY :: MEX Context
 */
typedef struct emlrtContext
{
    boolean_T   bFirstTime;
    boolean_T   bInitialized;
    unsigned int fVersionInfo;
    EmlrtErrorFunction fErrorFunction;
    const char *fFunctionName;
    struct emlrtCallStack* fRTCallStack;
    boolean_T   bDebugMode;
    uint32_T fSigWrd[4];
    void * fSigMem;
} emlrtContext;

/*
 * MATLAB INTERNAL USE ONLY :: External Mode Simulation
 */
typedef struct emlrtExternalSim
{
    void        *fESim;
    uint8_T     *fIOBuffer;
    uint8_T     *fIOBufHead;
    size_t       fIOBufSize;
} emlrtExternalSim;

/*
 * MATLAB INTERNAL USE ONLY :: Array bounds check parameters
 */
typedef struct emlrtBCInfo
{
    int32_T     iFirst;
    int32_T     iLast;
    int32_T     lineNo;
    int32_T     colNo;
    const char *aName;
    const char *fName;
    const char *pName;
    int32_T     checkKind;
}   emlrtBCInfo;


/*
 * MATLAB INTERNAL USE ONLY :: Equality check parameters
 */
typedef struct emlrtECInfo
{
    int32_T     nDims;
    int32_T     lineNo;
    int32_T     colNo;
    const char *fName;
    const char *pName;
}   emlrtECInfo;

/*
 * MATLAB INTERNAL USE ONLY :: Array bounds check parameters
 */
typedef struct
{
    int32_T     lineNo;
    int32_T     colNo;
    const char *fName;
    const char *pName;
}   emlrtRTEInfo;

typedef emlrtRTEInfo emlrtMCInfo;

/* MATLAB INTERNAL USE ONLY :: Reference to global runtime context */
extern emlrtContext emlrtContextGlobal;

/*
 * MATLAB INTERNAL USE ONLY :: Initialize file I/O
 */
EXTERN_C LIBEMLRT_API void emlrtInitFileio(emlrtCTX aTLS,
                                           EmlrtFopenFunction aFopenFcn,
                                           EmlrtFcloseFunction aFcloseFcn);

/*
 * MATLAB INTERNAL USE ONLY :: Open and register a file
 */
EXTERN_C LIBEMLRT_API emlrtFid emlrtFmgrOpen(const emlrtCTX aTLS,
                                             const void * const aFname,
                                             const void * const aPermission,
                                             const boolean_T aIsAutoFlush);

/*
 * MATLAB INTERNAL USE ONLY :: Retrieve a FILE * from an FID
 */
EXTERN_C LIBEMLRT_API emlrtFilePtr emlrtFmgrFilestar(const emlrtCTX aTLS,
                                                     const emlrtFid aFid);

/*
 * MATLAB INTERNAL USE ONLY :: Is the file auto flush
 */
EXTERN_C LIBEMLRT_API boolean_T emlrtFmgrAutoflush(const emlrtCTX aTLS,
                                                   const emlrtFid aFid);

/*
 * MATLAB INTERNAL USE ONLY :: Close and unregister a file
 */
EXTERN_C LIBEMLRT_API boolean_T emlrtFmgrClose(const emlrtCTX aTLS,
                                               const emlrtFid aFid);

/*
 * MATLAB INTERNAL USE ONLY :: Close all open FIDs
 */
EXTERN_C LIBEMLRT_API boolean_T emlrtFmgrCloseAll(const emlrtCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Dispatch to mexPrintf
 */
EXTERN_C LIBEMLRT_API int emlrtMexVprintf(const char *aFmt, va_list aVargs);

/*
* MATLAB INTERNAL USE ONLY :: Dispatch to mexPrintf
*/
EXTERN_C LIBEMLRT_API int emlrtMexPrintf(emlrtConstCTX aTLS, const char *aFmt, ...);

/*
 * MATLAB INTERNAL USE ONLY :: Query first-time sentinel
 */
EXTERN_C LIBEMLRT_API boolean_T emlrtFirstTimeR2012b(emlrtCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Create an mxArray alias
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtAlias(const mxArray *in);

/*
 * MATLAB INTERNAL USE ONLY :: Create a persistent mxArray alias
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtAliasP(const mxArray *in);

/*
 * MATLAB INTERNAL USE ONLY :: Return a vector of mxArray to MATLAB
 */
EXTERN_C LIBEMLRT_API void emlrtReturnArrays(const int aNlhs, mxArray *aLHS[], const mxArray *const aRHS[]);

/*
 * MATLAB INTERNAL USE ONLY :: Protect mxArray from being overwritten if necessary
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtProtectR2012b(const mxArray *pa, int , bool , int reservedNumEl);

/*
 * MATLAB INTERNAL USE ONLY :: License check
 */
EXTERN_C LIBEMLRT_API void emlrtLicenseCheckR2012b(emlrtCTX aTLS, const char *aFeatureKey, const int b);

/*
 * MATLAB INTERNAL USE ONLY :: Verify default fimath
 */
EXTERN_C LIBEMLRT_API void emlrtCheckDefaultFimathR2008b(const mxArray **ctFimath);

/*
 * MATLAB INTERNAL USE ONLY :: Clear mxArray allocation count
 */
EXTERN_C LIBEMLRT_API void emlrtClearAllocCountR2012b(emlrtCTX aTLS, boolean_T bM, uint32_T iL, const char* ctDTO);

/*
 * MATLAB INTERNAL USE ONLY :: Load a specified library
 */
EXTERN_C LIBEMLRT_API int32_T emlrtLoadLibrary(const char *aFullname);

/*
 * MATLAB INTERNAL USE ONLY :: Clear mxArray allocation check
 */
EXTERN_C LIBEMLRT_API void emlrtClearAllocCheck(void);

/*
 * MATLAB INTERNAL USE ONLY :: Update mxArray allocation count
 */
EXTERN_C LIBEMLRT_API void emlrtUpdateAllocCount(int32_T delta);

/*
 * MATLAB INTERNAL USE ONLY :: Update persistent mxArray allocation count
 */
EXTERN_C LIBEMLRT_API void emlrtUpdateAllocCountP(int32_T delta);

/*
 * MATLAB INTERNAL USE ONLY :: Check mxArray allocation count
 */
EXTERN_C LIBEMLRT_API void emlrtCheckAllocCount(emlrtCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Assign to an mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtAssign(const mxArray **lhs, const mxArray *rhs);

/*
 * MATLAB INTERNAL USE ONLY :: Assign to a persistent mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtAssignP(const mxArray **lhs, const mxArray *rhs);

/*
 * MATLAB INTERNAL USE ONLY :: Array bounds check
 */
EXTERN_C LIBEMLRT_API int32_T emlrtBoundsCheckR2012b(int32_T indexValue, emlrtBCInfo *aInfo, emlrtCTX aTLS);

#ifdef INT_TYPE_64_IS_SUPPORTED
/*
 * MATLAB INTERNAL USE ONLY :: Array bounds check for int64
 */
EXTERN_C LIBEMLRT_API int64_T emlrtBoundsCheckInt64(const int64_T indexValue, const emlrtBCInfo * const aInfo, const emlrtCTX aTLS);
#endif

/*
 * MATLAB INTERNAL USE ONLY :: Dynamic array bounds check
 */
EXTERN_C LIBEMLRT_API int32_T emlrtDynamicBoundsCheckR2012b(int32_T indexValue, int32_T loBound, int32_T hiBound, emlrtBCInfo *aInfo, emlrtConstCTX aTLS);

#ifdef INT_TYPE_64_IS_SUPPORTED
/*
 * MATLAB INTERNAL USE ONLY :: Dynamic array bounds check for int64
 */
EXTERN_C LIBEMLRT_API int64_T emlrtDynamicBoundsCheckInt64(const int64_T indexValue, const int32_T loBound, const int32_T hiBound, const emlrtBCInfo * const aInfo, const emlrtConstCTX aTLS);
#endif

/*
 * MATLAB INTERNAL USE ONLY :: Perform integer multiplication, raise runtime error
 *                             if the operation overflows.
 */
EXTERN_C LIBEMLRT_API size_t emlrtSizeMulR2012b(size_t s1, size_t s2, const emlrtRTEInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Create an mxArray string from a C string
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateString(const char *in);

/*
 * MATLAB INTERNAL USE ONLY :: Create an mxArray string from a single char
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateString1(char c);

/*
 * MATLAB INTERNAL USE ONLY :: Create a struct matrix mxArray
 */
EXTERN_C LIBEMLRT_API mxArray *emlrtCreateStructMatrix(int32_T m, int32_T n, int32_T nfields, const char **field_names);

/*
 * MATLAB INTERNAL USE ONLY :: Create a struct matrix mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateStructArray(int32_T ndim, const int32_T *pdim, int32_T nfields, const char **field_names);

/*
 * MATLAB INTERNAL USE ONLY :: Create an enum
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateEnumR2012b(emlrtConstCTX aTLS, const char *name, const mxArray *data);

/*
 * MATLAB INTERNAL USE ONLY :: Add a field to a struct matrix mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateField(const mxArray *mxStruct, const char *fldName);

/*
 * MATLAB INTERNAL USE ONLY :: Add a field to a struct matrix mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtAddField(const mxArray *mxStruct, const mxArray *mxField, const char *fldName, int index);

/*
 * MATLAB INTERNAL USE ONLY :: Get a field from a struct matrix mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtGetFieldR2013a(emlrtConstCTX aTLS, const mxArray *mxStruct, int aIndex, const char *fldName);

/*
 * MATLAB INTERNAL USE ONLY :: Set field value in structure array, given index and field name
 */
EXTERN_C LIBEMLRT_API void emlrtSetField(mxArray *mxStruct, int aIndex, const char *fldName, mxArray *mxValue);

/*
 * MATLAB INTERNAL USE ONLY :: Create a cell array
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateCellArrayR2014a(int32_T ndim, const int32_T *pdim);

/*
 * MATLAB INTERNAL USE ONLY :: Create a cell matrix mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateCellMatrix(int32_T m, int32_T n);

/*
 * MATLAB INTERNAL USE ONLY :: Set a cell element
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtSetCell(const mxArray *mxCellArray, int aIndex, const mxArray *mxCell);

/*
 * MATLAB INTERNAL USE ONLY :: Get a cell element
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtGetCell(emlrtConstCTX aTLS, const emlrtMsgIdentifier *aMsgId, const mxArray *mxCell, int aIndex);

/*
 * MATLAB INTERNAL USE ONLY :: Check if empty
 */
EXTERN_C LIBEMLRT_API const bool emlrtIsEmpty(const mxArray *mxCell);

/*
 * MATLAB INTERNAL USE ONLY :: Create a numeric matrix mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateNumericMatrix(int32_T m, int32_T n, int32_T classID, int32_T nComplexFlag);

/*
 * MATLAB INTERNAL USE ONLY :: Create a numeric matrix mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateNumericArray(int32_T ndim, const void *pdim, int32_T classID, int32_T nComplexFlag);

/*
 * MATLAB INTERNAL USE ONLY :: Create a scaled numeric matrix mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateScaledNumericArrayR2008b(int32_T ndim, const void *pdim, int32_T classID, int32_T nComplexFlag, int32_T aScale);

/*
 * MATLAB INTERNAL USE ONLY :: Create a double scalar mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateDoubleScalar(real_T in);

/*
 * MATLAB INTERNAL USE ONLY :: Create a logical matrix mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateLogicalArray(int32_T ndim, const int32_T *dims);

/*
 * MATLAB INTERNAL USE ONLY :: Create a 2-D logical matrix mxArray
 */
EXTERN_C LIBEMLRT_API mxArray *emlrtCreateLogicalMatrix(int32_T aN, int32_T aM);

/*
 * MATLAB INTERNAL USE ONLY :: Create a logical scalar mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateLogicalScalar(boolean_T in);

/*
 * MATLAB INTERNAL USE ONLY :: Create a character array mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateCharArray(int32_T ndim, const int32_T *dims);

/*
 * MATLAB INTERNAL USE ONLY :: Create a FI mxArray from a value mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCreateFIR2013b(emlrtConstCTX aTLS, const mxArray *fimath, const mxArray *ntype, const char *fitype, const mxArray *fival, const bool fmIsLocal, const bool aForceComplex);

/*
 * MATLAB INTERNAL USE ONLY :: Set the dimensions of an mxArray.
 */
EXTERN_C LIBEMLRT_API int emlrtSetDimensions(mxArray *aMx, const int32_T *dims, int32_T ndims);

/*
 * MATLAB INTERNAL USE ONLY :: Get the intarray from a FI mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtImportFiIntArrayR2008b(const mxArray *aFiMx);

/*
 * MATLAB INTERNAL USE ONLY :: Get the enum int from an MCOS enumeration mxArray.
 */
EXTERN_C LIBEMLRT_API const int32_T emlrtGetEnumElementR2009a(const mxArray *aEnum, int aIndex);

/*
 * MATLAB INTERNAL USE ONLY :: Get the enum int from an MCOS enumeration mxArray.
 */
EXTERN_C LIBEMLRT_API const mxArray * emlrtGetEnumUnderlyingArrayR2009a(const mxArray *aEnum);

/*
 * MATLAB INTERNAL USE ONLY :: Convert MATLAB mxArray data format to C data format
 */
EXTERN_C LIBEMLRT_API void emlrtMatlabDataToCFormat(const mxArray* inputMx, const mxArray* cformatMx);

/*
 * MATLAB INTERNAL USE ONLY :: Convert C data format to MATLAB data format
 */
EXTERN_C LIBEMLRT_API void emlrtMatlabDataFromCFormat(const mxArray* outputMx, const mxArray* cformatMx);

/*
 * MATLAB INTERNAL USE ONLY :: Try to coerce mxArray to provided class; return same mxArray if not possible.
 */
EXTERN_C LIBEMLRT_API const mxArray *emlrtCoerceToClassR2014b(const mxArray* inputMx, const char *className);

/*
 * MATLAB INTERNAL USE ONLY :: Destroy an mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtDestroyArray(const mxArray **pa);

/*
 * MATLAB INTERNAL USE ONLY :: Destroy a vector of mxArrays
 */
EXTERN_C LIBEMLRT_API void emlrtDestroyArrays(int32_T narrays, const mxArray **parrays);

/*
 * MATLAB INTERNAL USE ONLY :: Free the imaginary part of a matrix if all the imaginary elements are zero
 */
EXTERN_C LIBEMLRT_API void emlrtFreeImagIfZero(const mxArray *pa);

/*
 * MATLAB INTERNAL USE ONLY :: Display an mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtDisplayR2012b(const mxArray *pa, const char *name, emlrtMCInfo* aLoc, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Double check parameters
 */
typedef struct
{
    int32_T     lineNo;
    int32_T     colNo;
    const char *fName;
    const char *pName;
    int32_T     checkKind; /* see src/cg_ir/base/Node.hpp::CG_Node_CheckEnum */
}   emlrtDCInfo;

/*
 * MATLAB INTERNAL USE ONLY :: Check that d can be safely cast to int.
 */
EXTERN_C LIBEMLRT_API real_T emlrtIntegerCheckR2012b(real_T d, emlrtDCInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Check that d is not NaN.
 */
EXTERN_C LIBEMLRT_API real_T emlrtNotNanCheckR2012b(real_T d, emlrtDCInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Check that d >= 0.
 */
EXTERN_C LIBEMLRT_API real_T emlrtNonNegativeCheckR2012b(real_T d, emlrtDCInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Check that the loop has an integer number of iterations.
 */
EXTERN_C LIBEMLRT_API void emlrtForLoopVectorCheckR2012b(real_T start, real_T step, real_T end, mxClassID classID, int n, emlrtRTEInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: fetch a global variable
 */
EXTERN_C LIBEMLRT_API void emlrtPutGlobalVariable(const char *name, const mxArray *parray);

/*
 * MATLAB INTERNAL USE ONLY :: fetch a global variable
 */
EXTERN_C LIBEMLRT_API const mxArray * emlrtGetGlobalVariable(const char *name);

/*
 * MATLAB INTERNAL USE ONLY :: Call out to MATLAB
 */
EXTERN_C LIBEMLRT_API const mxArray * emlrtCallMATLABR2012b(emlrtConstCTX aTLS, int32_T nlhs, const mxArray **plhs, int32_T nrhs, const mxArray **prhs, const char *cmd, boolean_T tmp, emlrtMCInfo* aLoc);

/*
 * MATLAB INTERNAL USE ONLY :: Maintain heap memory check information upon entering a mex function.
 */
EXTERN_C LIBEMLRT_API void emlrtHeapMemCheckInitialize(emlrtCTX aTLS, int aMaxHeapSize);

/*
 * MATLAB INTERNAL USE ONLY :: Maintain heap memory check information upon leaving a mex function.
 */
EXTERN_C LIBEMLRT_API void emlrtHeapMemCheckTerminate(void);

/*
 * MATLAB INTERNAL USE ONLY :: Register the mex function to be unloaded.
 */
EXTERN_C LIBEMLRT_API void emlrtHeapMemCheckPrepareUnload(emlrtCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Maintain heap memory check information upon unload a mex function.
 *                             Memory leak check is performed here both for normal and error exit.
 */
EXTERN_C LIBEMLRT_API void emlrtHeapMemCheckUnloadMex(emlrtCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: malloc with memory integrity check
 */
EXTERN_C LIBEMLRT_API void* emlrtMalloc(emlrtConstCTX aTLS, size_t aSize);

/*
 * MATLAB INTERNAL USE ONLY :: calloc with memory integrity check
 */
EXTERN_C LIBEMLRT_API void* emlrtCalloc(emlrtConstCTX aTLS, size_t aNum, size_t aSize);

/*
 * MATLAB INTERNAL USE ONLY :: free with memory integrity check
 */
EXTERN_C LIBEMLRT_API void emlrtFree(void *aPtr);

/*
 * MATLAB INTERNAL USE ONLY :: malloc for MEX
 */
EXTERN_C LIBEMLRT_API void* emlrtMallocMex(size_t aSize);

/*
 * MATLAB INTERNAL USE ONLY :: calloc for MEX
 */
EXTERN_C LIBEMLRT_API void* emlrtCallocMex(size_t aNum, size_t aSize);

/*
 * MATLAB INTERNAL USE ONLY :: free for MEX
 */
EXTERN_C LIBEMLRT_API void emlrtFreeMex(void *aPtr);

/*
 * MATLAB INTERNAL USE ONLY :: Enter a new function within a MEX call.
 */
EXTERN_C LIBEMLRT_API void emlrtHeapReferenceStackEnterFcnR2012b(emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Enter a new function within a MEX call.
 */
EXTERN_C LIBEMLRT_API void emlrtHeapReferenceStackEnterFcn(void);

/*
 * MATLAB INTERNAL USE ONLY :: Leave a scope within a MEX call.
 */
EXTERN_C LIBEMLRT_API void emlrtHeapReferenceStackLeaveScope(emlrtConstCTX aTLS, int aAllocCount);

/*
 * MATLAB INTERNAL USE ONLY :: Leave a function within a MEX call.
 */
EXTERN_C LIBEMLRT_API void emlrtHeapReferenceStackLeaveFcnR2012b(emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Leave a function within a MEX call.
 */
EXTERN_C LIBEMLRT_API void emlrtHeapReferenceStackLeaveFcn(void);

/*
 * MATLAB INTERNAL USE ONLY :: Push a new entry to the heap reference stack.
 */
EXTERN_C LIBEMLRT_API void emlrtPushHeapReferenceStackR2012b(emlrtConstCTX aTLS, void *aHeapReference, EmlrtHeapReferenceFreeFcn aFreeFcn);

/*
 * MATLAB INTERNAL USE ONLY :: Initialize a character mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtInitCharArrayR2013a(emlrtConstCTX aTLS, int32_T n, const mxArray *a, const char *s);

/*
 * MATLAB INTERNAL USE ONLY :: Initialize a logical mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtInitLogicalArray(int32_T n, const mxArray *a, const boolean_T *b);

/*
 * MATLAB INTERNAL USE ONLY :: Initialize an integral array from a multiword type
 */
EXTERN_C LIBEMLRT_API void emlrtInitIntegerArrayFromMultiword(const mxArray *aOut, const void *aInData);

/*
 * MATLAB INTERNAL USE ONLY :: Export a numeric mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtExportNumericArrayR2013b(emlrtConstCTX aTLS, const mxArray *aOut, const void *aInData, int32_T aElementSize);

/*
 * MATLAB INTERNAL USE ONLY :: Auto-generated mexFunction
 */
typedef void (*emlrtMexFunction)(int, mxArray*[], int, const mxArray*[]);

/*
 * MATLAB INTERNAL USE ONLY :: Auto-generated entry-point
 */
typedef struct emlrtEntryPoint
{
    const char *fName;
    emlrtMexFunction fMethod;
} emlrtEntryPoint;

/*
 * MATLAB INTERNAL USE ONLY :: Lookup an entry point
 */
EXTERN_C LIBEMLRT_API int emlrtGetEntryPointIndexR2016a(emlrtConstCTX aTLS, int nrhs, const mxArray *prhs[], const char *aEntryPointNames[], int aNumEntryPoints);

/*
 * MATLAB INTERNAL USE ONLY :: Decode wide character strings in mxArray struct array.
 * The struct array consists of name resolution entries which may contain file paths.
 * If the file path is using UTF-8, then the string is plain 7-bit ASCII but encoded.
 * We need to decode those if necessary. The resulting mxArray strings (for file paths)
 * will be proper UTF-16 strings.
 */
EXTERN_C LIBEMLRT_API void emlrtNameCapturePostProcessR2013b(const mxArray ** mxInfo);

/*
 * MATLAB INTERNAL USE ONLY :: Decode string array into mxArray object containing
 * name resolution data.
 */
EXTERN_C LIBEMLRT_API void emlrtNameCaptureMxArrayR2016a(const char *mxInfoEncoded[], uint32_T uncompressedSize, const mxArray **mxInfo);

/*
 * MATLAB INTERNAL USE ONLY :: Parallel runtime error exception
 */
#ifdef __cplusplus
class EmlrtParallelRunTimeError
{
  public:
    EmlrtParallelRunTimeError();
    virtual ~EmlrtParallelRunTimeError();
};
#endif

/*
 * MATLAB INTERNAL USE ONLY :: Report if we are in a parallel region
 */
EXTERN_C LIBEMLRT_API boolean_T emlrtIsInParallelRegion(emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Enter a parallel region.
 */
EXTERN_C LIBEMLRT_API void emlrtEnterParallelRegion(emlrtConstCTX aTLS, boolean_T aInParallelRegion);

/*
 * MATLAB INTERNAL USE ONLY :: Exit a parallel region.
 */
EXTERN_C LIBEMLRT_API void emlrtExitParallelRegion(emlrtConstCTX aTLS, boolean_T aInParallelRegion);

/*
 * MATLAB INTERNAL USE ONLY :: Check if we're running on the MATLAB thread.
 */
EXTERN_C LIBEMLRT_API bool emlrtIsMATLABThread(emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Record the occurrence of a parallel warning.
 */
EXTERN_C LIBEMLRT_API bool emlrtSetWarningFlag(emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Report a parallel runtime error
 */
EXTERN_C LIBEMLRT_API void emlrtReportParallelRunTimeError(emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Ensure active thread is MATLAB
 */
EXTERN_C LIBEMLRT_API void emlrtAssertMATLABThread(emlrtConstCTX aTLS, emlrtMCInfo* aLoc);

/*
 * MATLAB INTERNAL USE ONLY :: Push the current jmp_buf environment
 */
EXTERN_C LIBEMLRT_API void emlrtPushJmpBuf(emlrtConstCTX aTLS, jmp_buf *volatile *aJBEnviron);

/*
 * MATLAB INTERNAL USE ONLY :: Pop the current jmp_buf environment
 */
EXTERN_C LIBEMLRT_API void emlrtPopJmpBuf(emlrtConstCTX aTLS, jmp_buf *volatile *aJBEnviron);

/*
 * MATLAB INTERNAL USE ONLY :: Set the current jmp_buf environment
 */
EXTERN_C LIBEMLRT_API void emlrtSetJmpBuf(emlrtConstCTX aTLS, jmp_buf *aJBEnviron);

/*
 * MATLAB INTERNAL USE ONLY :: Create a shallow copy of an mxArray
 */
EXTERN_C LIBEMLRT_API const mxArray * emlrtCreateReference(const mxArray *pa);

/*
 * MATLAB INTERNAL USE ONLY :: Division by zero error
 */
EXTERN_C LIBEMLRT_API void emlrtDivisionByZeroErrorR2012b(const emlrtRTEInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Integer overflow error
 */
EXTERN_C LIBEMLRT_API void emlrtIntegerOverflowErrorR2012b(const emlrtRTEInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Raise C heap allocation failure
 */
EXTERN_C LIBEMLRT_API void emlrtHeapAllocationErrorR2012b(const emlrtRTEInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Error with given message ID and args.
 */
EXTERN_C LIBEMLRT_API void emlrtErrorWithMessageIdR2012b(emlrtConstCTX aTLS, const emlrtRTEInfo *aInfo, const char *aMsgID, int aArgCount, ...);

/*
 * MATLAB INTERNAL USE ONLY :: Error with given message ID and args.
 */
EXTERN_C LIBEMLRT_API void emlrtErrMsgIdAndTxt(emlrtCTX aTLS, const char *aMsgID, int aArgCount, ...);

/*
 * MATLAB INTERNAL USE ONLY :: Error with given message ID and explicit message text.
 */
EXTERN_C LIBEMLRT_API void emlrtErrMsgIdAndExplicitTxt(emlrtConstCTX aTLS, const emlrtRTEInfo *aInfo, const char *aMsgID, int32_T aStrlen, const char *aMsgTxt);

/*
 * MATLAB INTERNAL USE ONLY :: Convert a message ID to a heap-allocated LCP string.
 * This function is used by Stateflow run-time library only.
 */
EXTERN_C LIBEMLRT_API char* emlrtTranslateMessageIDtoLCPstring(const char* aMsgID);

/*
 * MATLAB INTERNAL USE ONLY :: Convert a UTF16 message string to a heap-allocated LCP string.
 * This function is used by Stateflow run-time library only.
 */
EXTERN_C LIBEMLRT_API char* emlrtTranslateUTF16MessagetoLCPstring(const CHAR16_T* aUTF16Msg);

typedef struct
{
    const char * MexFileName;
    const char * TimeStamp;
    const char * buildDir;
    int32_T numFcns;
    int32_T numHistogramBins;
} emlrtLocationLoggingFileInfoType;

typedef struct
{
    const char * FunctionName;
    int32_T FunctionID;
    int32_T numInstrPoints;
} emlrtLocationLoggingFunctionInfoType;

typedef struct
{
    real_T NumberOfZeros;
    real_T NumberOfPositiveValues;
    real_T NumberOfNegativeValues;
    real_T TotalNumberOfValues;
    real_T SimSum;
    real_T HistogramOfPositiveValues[256];
    real_T HistogramOfNegativeValues[256];
} emlrtLocationLoggingHistogramType;

typedef struct
{
    real_T SimMin;
    real_T SimMax;
    int32_T OverflowWraps;
    int32_T Saturations;
    boolean_T IsAlwaysInteger;
    emlrtLocationLoggingHistogramType * HistogramTable;
} emlrtLocationLoggingDataType;

typedef struct
{
    int32_T MxInfoID;
    int32_T TextStart;
    int32_T TextLength;
    int16_T Reason;
    boolean_T MoreLocations;
} emlrtLocationLoggingLocationType;

/*
 * MATLAB INTERNAL USE ONLY :: Get LocationLogging Info
 */
EXTERN_C LIBEMLRT_API mxArray* emlrtLocationLoggingPullLog(const char* const MexFileName, bool pullCompReportFromMexFunction);

/*
 * MATLAB INTERNAL USE ONLY :: Save LocationLogging Info
 */
EXTERN_C LIBEMLRT_API void emlrtLocationLoggingPushLog(const emlrtLocationLoggingFileInfoType* const fileInfo,
                                                       const emlrtLocationLoggingFunctionInfoType* const functionInfoTable,
                                                       const emlrtLocationLoggingDataType* const dataTables,
                                                       const emlrtLocationLoggingLocationType* const locationTables,
                                                       const uint8_T *serializedReport,
                                                       size_t sizeofSerializedReport,
                                                       const int32_T* numFieldsTables,
                                                       const char** fieldNamesTables);

/*
 * MATLAB INTERNAL USE ONLY :: Clear LocationLogging Info
 */
EXTERN_C LIBEMLRT_API bool emlrtLocationLoggingClearLog(const char* const MexFileName);

/*
 * MATLAB INTERNAL USE ONLY :: List entries in LocationLogging Info
 */
EXTERN_C LIBEMLRT_API mxArray* emlrtLocationLoggingListLogs(void);

/*
 * MATLAB INTERNAL USE ONLY :: Add instrumentation results to FPT Repository
 */
EXTERN_C LIBEMLRT_API void addResultsToFPTRepository(const char* const blkSID);

/*
 * MATLAB INTERNAL USE ONLY :: Initialize a runtime stack
 */
EXTERN_C LIBEMLRT_API void emlrtEnterRtStackR2012b(emlrtCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Terminate a runtime stack
 */
EXTERN_C LIBEMLRT_API void emlrtLeaveRtStackR2012b(emlrtCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Push to runtime stack
 */
EXTERN_C LIBEMLRT_API void emlrtPushRtStackR2012b(const struct emlrtRSInfo *aRSInfo, emlrtCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Pop from runtime stack
 */
EXTERN_C LIBEMLRT_API void emlrtPopRtStackR2012b(const struct emlrtRSInfo *aRSInfo, emlrtCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Serialize a byte
 */
EXTERN_C LIBEMLRT_API void emlrtSerializeByte(uint8_T b);

/*
 * MATLAB INTERNAL USE ONLY :: Deserialize a byte
 */
EXTERN_C LIBEMLRT_API uint8_T emlrtDeserializeByte(void);

/*
 * MATLAB INTERNAL USE ONLY :: Terminate serializing
 */
EXTERN_C LIBEMLRT_API void emlrtSerializeTerminate(void);

/*
 * MATLAB INTERNAL USE ONLY :: Deserialize a double
 */
EXTERN_C LIBEMLRT_API real64_T emlrtDeserializeDouble(void);

/*
 * MATLAB INTERNAL USE ONLY :: Serialize a double
 */
EXTERN_C LIBEMLRT_API void emlrtSerializeDouble(real64_T d);

/*
 * MATLAB INTERNAL USE ONLY :: Deserialize a single
 */
EXTERN_C LIBEMLRT_API real32_T emlrtDeserializeSingle(void);

/*
 * MATLAB INTERNAL USE ONLY :: Serialize a single
 */
EXTERN_C LIBEMLRT_API void emlrtSerializeSingle(real32_T d);

/*
 * MATLAB INTERNAL USE ONLY :: Deserialize a char
 */
EXTERN_C LIBEMLRT_API char emlrtDeserializeChar(void);

/*
 * MATLAB INTERNAL USE ONLY :: Serialize a char
 */
EXTERN_C LIBEMLRT_API void emlrtSerializeChar(char c);

/*
 * MATLAB INTERNAL USE ONLY :: Deserialize a logical
 */
EXTERN_C LIBEMLRT_API boolean_T emlrtDeserializeLogical(void);

/*
 * MATLAB INTERNAL USE ONLY :: Serialize a logical
 */
EXTERN_C LIBEMLRT_API void emlrtSerializeLogical(boolean_T b);

/*
 * MATLAB INTERNAL USE ONLY :: Initialize serializing
 */
EXTERN_C LIBEMLRT_API void emlrtSerializeInitialize(boolean_T isDeserialize, boolean_T isVerification, const char *projectName, uint32_T aCheckSumLen, const uint32_T *aChecksum);

/*
 * MATLAB INTERNAL USE ONLY :: Get the address of the Ctrl-C flag.
 */
EXTERN_C LIBEMLRT_API const volatile char* emlrtGetBreakCheckFlagAddressR2012b(void);

/*
 * MATLAB INTERNAL USE ONLY :: Check for Ctrl+C (break)
 */
EXTERN_C LIBEMLRT_API void emlrtBreakCheckR2012b(emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Equality check for 1-D sizes
 */
EXTERN_C LIBEMLRT_API void emlrtSizeEqCheck1DR2012b(int32_T dim1, int32_T dim2, emlrtECInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Equality check for size vectors
 */
EXTERN_C LIBEMLRT_API void emlrtSizeEqCheckNDR2012b(const int32_T* dims1, const int32_T* dims2, emlrtECInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: equality check
 */
EXTERN_C LIBEMLRT_API void emlrtDimSizeEqCheckR2012b(int32_T dim1, int32_T dim2, emlrtECInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: greater than or equal to check
 */
EXTERN_C LIBEMLRT_API void emlrtDimSizeGeqCheckR2012b(int32_T dim1, int32_T dim2, emlrtECInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Check size compatibility for A(I1,..IN) = B assignment in MATLAB.
 */
EXTERN_C LIBEMLRT_API void emlrtSubAssignSizeCheckR2012b(const int32_T* dims1, int32_T nDims1, const int32_T* dims2, int32_T nDims2, emlrtECInfo *aInfo, emlrtConstCTX aTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Allocate thread-local storage.
 */
EXTERN_C LIBEMLRT_API void *emlrtAllocTLS(emlrtConstCTX aMaster, int32_T aTeamTID);

/*
 * MATLAB INTERNAL USE ONLY :: Allocate thread local storage for a parallel region.
 */
EXTERN_C LIBEMLRT_API int32_T emlrtAllocRegionTLSs(emlrtCTX aTLS, boolean_T aInParallelRegion, int32_T aMaxThreads, int32_T aNumThreads);

/*
 * MATLAB INTERNAL USE ONLY :: Allocate the root thread-local storage.
 */
EXTERN_C LIBEMLRT_API void emlrtCreateRootTLS(emlrtCTX *aRootTLS, struct emlrtContext *aContext, EmlrtLockerFunction aLockerFunction, int32_T aNumProcs);

/*
 * MATLAB INTERNAL USE ONLY :: Deallocate the root thread-local storage.
 */
EXTERN_C LIBEMLRT_API void emlrtDestroyRootTLS(emlrtCTX *aRootTLS);

/*
 * MATLAB INTERNAL USE ONLY :: Set Jit Simulation Mode.
 */
EXTERN_C LIBEMLRT_API void emlrtSetSimThruJIT(emlrtCTX aTLS, boolean_T aSimThruJIT);

EXTERN_C LIBEMLRT_API char * emlrtExtractMessageId2015b(emlrtConstCTX aTLS, const struct emlrtMsgIdentifier *aMsgId);

/*
 * MATLAB INTERNAL USE ONLY :: Check the class of an mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtCheckClass(const char *msgName, const mxArray *pa, const char *className);

/*
 * MATLAB INTERNAL USE ONLY :: Check the size, class and complexness of an mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtCheckBuiltInR2012b(emlrtConstCTX aTLS,
                                                   const struct emlrtMsgIdentifier *aMsgId, const mxArray *pa,
                                                   const char *className, boolean_T complex, uint32_T nDims, const void *pDims);

/*
 * MATLAB INTERNAL USE ONLY :: Check the size, class and complexness of a variable-size mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtCheckVsBuiltInR2012b(emlrtConstCTX aTLS,
                                                     const struct emlrtMsgIdentifier *aMsgId, const mxArray *pa,
                                                     const char *className, boolean_T complex, uint32_T nDims, const void *pDims,
                                                     const boolean_T *aDynamic, int32_T *aOutSizes);

/*
 * MATLAB INTERNAL USE ONLY :: Check the type of a FI mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtCheckFiR2012b(emlrtConstCTX aTLS,
                                              const struct emlrtMsgIdentifier *aMsgId, const mxArray *aFi, boolean_T aComplex,
                                              uint32_T aNDims, const void *aVDims,
                                              const mxArray *aFimath, const mxArray *aNumericType);

/*
 * MATLAB INTERNAL USE ONLY :: Check the type of a variable-size FI mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtCheckVsFiR2012b(emlrtConstCTX aTLS,
                                                const struct emlrtMsgIdentifier *aMsgId, const mxArray *aFi, boolean_T aComplex,
                                                uint32_T aNDims, const void *aVDims,
                                                const boolean_T *aDynamic, const mxArray *aFimath, const mxArray *aNumericType);

/*
 * MATLAB INTERNAL USE ONLY :: Check the type of a static-size struct mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtCheckStructR2012b(emlrtConstCTX aTLS, const emlrtMsgIdentifier *aMsgId, const mxArray *s, int32_T nFields, const char **fldNames, uint32_T nDims, const void *pDims);

/*
 * MATLAB INTERNAL USE ONLY :: Check the type of a variable-size struct mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtCheckVsStructR2012b(emlrtConstCTX aTLS, const struct emlrtMsgIdentifier *aMsgId, const mxArray *s,
                                                    int32_T nFields, const char **fldNames,
                                                    uint32_T nDims, const void *pDims,
                                                    const boolean_T *aDynamic, int32_T *aOutSizes);

/*
 * MATLAB INTERNAL USE ONLY :: Check the checksum of an mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtCheckArrayChecksumR2014a(emlrtConstCTX aTLS, const char *aName, const uint32_T *aCtCsVal, const mxArray *aRtMxArray, const boolean_T aIsGlobalVar);

/*
 * MATLAB INTERNAL USE ONLY :: Check the type of a static-size struct mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtCheckEnumR2012b(emlrtConstCTX aTLS, const char *enumName, int32_T nEnumElements, const char **enumNames, const int32_T *enumValues);

/*
 * MATLAB INTERNAL USE ONLY :: Import a character array.
 */
EXTERN_C LIBEMLRT_API void emlrtImportCharArrayR2015b(emlrtConstCTX aTLS, const mxArray *aSrc, char_T *aDst, int32_T aNumel);

/*
 * MATLAB INTERNAL USE ONLY :: Import a character.
 */
EXTERN_C LIBEMLRT_API void emlrtImportCharR2015b(emlrtConstCTX aTLS, const mxArray *aSrc, char_T *aDst);

/*
 * MATLAB INTERNAL USE ONLY :: Set the actual size of a variable-size array
 */
EXTERN_C LIBEMLRT_API void emlrtSetVsSizesR2008b(const mxArray *pa, uint32_T nDimsMax, int32_T *aOutSizes);

/*
 * MATLAB INTERNAL USE ONLY :: Import an mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtImportArrayR2015b(emlrtConstCTX aTLS, const mxArray *aIn, void *aOutData, int32_T aElementSize, boolean_T aComplex);

/*
 * MATLAB INTERNAL USE ONLY :: Import an mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtImportArrayR2015b_SameComplex(emlrtConstCTX aTLS, const mxArray *aIn, void *aOutData, int32_T aElementSize);

/*
 * MATLAB INTERNAL USE ONLY :: Import a FI mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtImportVsFiArrayR2011b(const mxArray *aFiMx, const mxArray *aIntMx, void *aOutData, int32_T aElementSize, boolean_T aComplex, uint32_T nDimsMax, int32_T *aOutSizes);

/*
 * MATLAB INTERNAL USE ONLY :: Set the actual sizes of a dynamic FI array
 */
EXTERN_C LIBEMLRT_API void emlrtSetVsFiSizes(const mxArray *aFi, uint32_T nDimsExpected, int32_T *aOutSizes);

/*
 * MATLAB INTERNAL USE ONLY :: Get double uniform random values in (0,1)
 */
EXTERN_C LIBEMLRT_API void emlrtRandu(real_T * const aRanduBuffer, const int32_T aNumel);

/*
 * MATLAB INTERNAL USE ONLY :: Get double normal random values
 */
EXTERN_C LIBEMLRT_API void emlrtRandn(real_T * const aRandnBuffer, const int32_T aNumel);

/*
 * MATLAB INTERNAL USE ONLY :: Check the type of a static-sized cell mxArray
 */
EXTERN_C LIBEMLRT_API void emlrtCheckCell(emlrtConstCTX aTLS, const emlrtMsgIdentifier *aMsgId, const mxArray *s,
                                          uint32_T nDims, const void *pDims, const boolean_T *aDynamic);

/*
 * MATLAB INTERNAL USE ONLY :: Check the type of a variable-sized cell mxArray
 * and assign the size of the mxArray to aOutSizes
 */
EXTERN_C LIBEMLRT_API void emlrtCheckVsCell(emlrtConstCTX aTLS, const struct emlrtMsgIdentifier *aMsgId, const mxArray *s,
                                            uint32_T nDims, const void *pDims,
                                            const boolean_T *aDynamic, int32_T *aOutSizes);

/*
 * MATLAB INTERNAL USE ONLY :: Check a cell mxArray with unassigned base type
 */
EXTERN_C LIBEMLRT_API void emlrtCheckCellWithUnassignedBase(emlrtConstCTX aTLS,
                                                            const struct emlrtMsgIdentifier *aMsgId,
                                                            const mxArray *pa);


#ifdef __WATCOMC__
#pragma aux emlrtIntegerCheckR2009b value [8087];
#pragma aux emlrtIntegerCheckR2011a value [8087];

#pragma aux emlrtNonNegativeCheckR2009b value [8087];
#pragma aux emlrtNonNegativeCheckR2011a value [8087];

#pragma aux emlrtNotNanCheckR2009b value [8087];
#pragma aux emlrtNotNanCheckR2011a value [8087];

#pragma aux emlrtExtDeserializeDouble value [8087];

#pragma aux emlrtDeserializeDouble value [8087];
#pragma aux emlrtDeserializeSingle value [8087];
#endif

#endif /* emlrt_h */
