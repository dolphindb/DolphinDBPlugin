/*
 * PUBLISHED header for covrt, the runtime library for Code Coverage
 *
 * Copyright 1984-2013 The MathWorks, Inc.
 *
 */

#ifndef covrt_h
#define covrt_h

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

#ifndef LIBCOVRT_API
#define LIBCOVRT_API
#endif

#include <setjmp.h>
#include <stdio.h>
#include "matrix.h"


/*
 * MATLAB INTERNAL USE ONLY :: Instance specific runtime data.
 */
typedef struct covrtInstanceData covrtInstanceData;

/*
 * MATLAB INTERNAL USE ONLY :: Instance type
 */
typedef struct covrtInstance {
    covrtInstanceData* data;
} covrtInstance;


/* one instance per mex */
extern covrtInstance gCoverageLoggingInstance;


/*
 * MATLAB INTERNAL USE ONLY :: Enable/Disable Coverage Logging during mex execution
 */
EXTERN_C LIBCOVRT_API void covrtEnableCoverageLogging( bool enable );


/*
 * MATLAB INTERNAL USE ONLY :: Enabled/Disable use of cv.mex
 */
EXTERN_C LIBCOVRT_API void covrtUseCV(bool useCV);

/*
 * MATLAB INTERNAL USE ONLY :: reset flag
 */
EXTERN_C LIBCOVRT_API void covrtResetUpdateFlag();


/*
 * MATLAB INTERNAL USE ONLY :: Allocate instance data
 */
EXTERN_C LIBCOVRT_API void covrtAllocateInstanceData( covrtInstance* instance );

/*
 * MATLAB INTERNAL USE ONLY :: Free instance data
 */
EXTERN_C LIBCOVRT_API void covrtFreeInstanceData( covrtInstance* instance );

/*
 * MATLAB INTERNAL USE ONLY :: Free instance data
 */
EXTERN_C LIBCOVRT_API mxArray* covrtSerializeInstanceData( covrtInstance* instance );


/*
 * MATLAB INTERNAL USE ONLY :: Coverage engine script initalization callback
 */
EXTERN_C LIBCOVRT_API void covrtScriptStart(covrtInstance* instance, 
                                            unsigned int cvId);

/*
 * MATLAB INTERNAL USE ONLY :: Initialize Script
 */
EXTERN_C LIBCOVRT_API void covrtScriptInit( covrtInstance* instance,
                                            const char* path,
                                            unsigned int cvId, 
                                            unsigned int fcnCnt,
                                            unsigned int basicBlockCnt,
                                            unsigned int ifCnt,
                                            unsigned int testobjectiveCnt,
	                                    unsigned int saturationCnt,
                                            unsigned int switchCnt,
                                            unsigned int forCnt,
                                            unsigned int whileCnt,
                                            unsigned int condCnt,
                                            unsigned int mcdcCnt);


/*
 * MATLAB INTERNAL USE ONLY :: Initialize Function
 */
EXTERN_C LIBCOVRT_API void covrtFcnInit( covrtInstance* instance,
                                         unsigned int cvId, 
                                         unsigned int fcnIdx,
                                         const char *name,
                                         int charStart,
                                         int charExprEnd,
                                         int charEnd);

/*
 * MATLAB INTERNAL USE ONLY :: Initialize Basic Block
 */
EXTERN_C LIBCOVRT_API void covrtBasicBlockInit( covrtInstance* instance,
                                         unsigned int cvId, 
                                         unsigned int fcnIdx,
                                         int charStart,
                                         int charExprEnd,
                                         int charEnd);

/*
 * MATLAB INTERNAL USE ONLY :: Initialize If
 */
EXTERN_C LIBCOVRT_API void covrtIfInit( covrtInstance* instance,
                                        unsigned int cvId, 
                                        unsigned int ifIdx,
                                        int charStart,
                                        int charExprEnd,
                                        int charElseStart,
                                        int charEnd);


/*
 * MATLAB INTERNAL USE ONLY :: Initialize Mcdc
 */
EXTERN_C LIBCOVRT_API void covrtMcdcInit( covrtInstance* instance,
                                          unsigned int cvId, 
                                          unsigned int mcdcIdx,
                                          int charStart,
                                          int charEnd,
                                          int condCnt,
                                          int firstCondIdx,
                                          int* condStart,
                                          int* condEnd,
                                          int postFixLength,
                                          int* postFixExprs);

/*
 * MATLAB INTERNAL USE ONLY :: Initialize Switch
 */
EXTERN_C LIBCOVRT_API void covrtSwitchInit( covrtInstance* instance,
                                            unsigned int cvId, 
                                            unsigned int switchIdx,
                                            int charStart,
                                            int charExprEnd,
                                            int charEnd,
                                            unsigned int caseCnt,
                                            int *caseStart,
                                            int *caseExprEnd);



/*
 * MATLAB INTERNAL USE ONLY :: Initialize For
 */
EXTERN_C LIBCOVRT_API void covrtForInit( covrtInstance* instance,
                                         unsigned int cvId, 
                                         unsigned int forIdx,
                                         int charStart,
                                         int charExprEnd,
                                         int charEnd);


/*
 * MATLAB INTERNAL USE ONLY :: Initialize While
 */
EXTERN_C LIBCOVRT_API void covrtWhileInit( covrtInstance* instance,
                                           unsigned int cvId, 
                                           unsigned int whileIdx,
                                           int charStart,
                                           int charExprEnd,
                                           int charEnd);


/*
 * MATLAB INTERNAL USE ONLY :: Initialize MCDC
 */
EXTERN_C LIBCOVRT_API void covrtMCDCInit( covrtInstance* instance,
                                          unsigned int cvId, 
                                          unsigned int mcdcIdx,
                                          int charStart,
                                          int charEnd,
                                          unsigned int condCnt,
                                          unsigned int firstCondIdx,
                                          int *condStart,
                                          int *condEnd,
                                          unsigned int pfxLength,
                                          int *pfixExpr);

/*
 * MATLAB INTERNAL USE ONLY :: Log Function
 */
EXTERN_C LIBCOVRT_API void covrtLogFcn(covrtInstance* instance,
                                       uint32_T covId, 
                                       uint32_T fcnId);

/*
 * MATLAB INTERNAL USE ONLY :: Log Basic Block
 */
EXTERN_C LIBCOVRT_API void covrtLogBasicBlock(covrtInstance* instance,
                                       uint32_T covId, 
                                       uint32_T basicBlockId);

/*
 * MATLAB INTERNAL USE ONLY :: Log If
 */
EXTERN_C LIBCOVRT_API int32_T covrtLogIf(covrtInstance* instance,
                                         uint32_T covId, 
                                         uint32_T fcnId, 
                                         int32_T ifId, 
                                         int32_T condition);

/*
 * MATLAB INTERNAL USE ONLY :: Log Cond
 */
EXTERN_C LIBCOVRT_API int32_T covrtLogCond(covrtInstance* instance,
                                            uint32_T covId, 
                                            uint32_T fcnId, 
                                            int32_T condId, 
                                            int32_T condition);

/*
 * MATLAB INTERNAL USE ONLY :: Log If
 */
EXTERN_C LIBCOVRT_API void covrtLogFor(covrtInstance* instance,
                                        uint32_T covId, 
                                        uint32_T fcnId, 
                                        int32_T forId, 
                                        int32_T entryOrExit);

/*
 * MATLAB INTERNAL USE ONLY :: Log While
 */
EXTERN_C LIBCOVRT_API int32_T covrtLogWhile(covrtInstance* instance,
                                            uint32_T covId,
                                            uint32_T fcnId, 
                                            int32_T whileId, 
                                            int32_T condition);

/*
 * MATLAB INTERNAL USE ONLY :: Log Switch
 */
EXTERN_C LIBCOVRT_API void covrtLogSwitch(covrtInstance* instance,
                                            uint32_T covId, 
                                            uint32_T fcnId, 
                                            int32_T switchId, 
                                            int32_T caseId);

/*
 * MATLAB INTERNAL USE ONLY :: Log If
 */
EXTERN_C LIBCOVRT_API int32_T covrtLogMcdc(covrtInstance* instance,
                                            uint32_T covId, 
                                            uint32_T fcnId, 
                                            int32_T mcdcId, 
                                            int32_T condition);

/*
 * MATLAB INTERNAL USE ONLY :: Log decision in block
 */
EXTERN_C LIBCOVRT_API void covrtLogBlockDec(covrtInstance* instance,
                                            uint32_T covId, 
                                            int32_T decId, 
                                            int32_T eleIdx, 
                                            int32_T decVal);

#endif /* covrt_h */
