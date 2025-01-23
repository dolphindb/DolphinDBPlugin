/*
 * CONFIDENTIAL AND CONTAINING PROPRIETARY TRADE SECRETS
 * Copyright 1984-2012 The MathWorks, Inc.
 * The source code contained in this listing contains proprietary and
 * confidential trade secrets of The MathWorks, Inc.   The use, modification,
 * or development of derivative work based on the code or ideas obtained
 * from the code is prohibited without the express written permission of The
 * MathWorks, Inc.  The disclosure of this code to any party not authorized
 * by The MathWorks, Inc. is strictly forbidden.
 * CONFIDENTIAL AND CONTAINING PROPRIETARY TRADE SECRETS
 */

/*
 *  32-bit API wrapper for libmwblas
 *  WARNING: This module is a temporary module specifically designed to 
 *           bridge the incompatiblity between Embedded MATLAB (which lacks
 *           a 64-bit integer type) and 64-bit BLAS only.
 *           Do not link to this module otherwise.
 *           Link to libmwblas instead.
 */   
#if defined(_MSC_VER)
# pragma once
#endif
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3))
# pragma once
#endif

#ifndef blascompat32_h
#define blascompat32_h

#ifndef EXTERN_C
#ifdef __cplusplus
  #define EXTERN_C extern "C"
#else
  #define EXTERN_C extern
#endif
#endif

#ifndef LIBMWBLASCOMPAT32_API
  #define LIBMWBLASCOMPAT32_API
#endif

#include <tmwtypes.h>

#if defined(_WIN32) 
#define FORTRAN_WRAPPER(x) x
#else
#define FORTRAN_WRAPPER(x) x ## _
#endif

#ifndef COMPLEX_TYPES
#define COMPLEX_TYPES
  typedef struct{float r,i;} complex;
  typedef struct{double r,i;} doublecomplex;
#endif
  
EXTERN_C LIBMWBLASCOMPAT32_API 
#define isamax32 FORTRAN_WRAPPER(isamax32)
int isamax32(const int *n32, const float  *sx, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API 
#define idamax32 FORTRAN_WRAPPER(idamax32)
int idamax32(const int *n32, const double *dx, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API 
#define icamax32 FORTRAN_WRAPPER(icamax32)
int icamax32(const int *n32, const creal32_T *cx, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API 
#define izamax32 FORTRAN_WRAPPER(izamax32)
int izamax32(const int *n32, const creal_T *zx, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API 
#define sasum32 FORTRAN_WRAPPER(sasum32)
float sasum32(const int *n32, const float  *sx, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API 
#define dasum32 FORTRAN_WRAPPER(dasum32)
double dasum32(const int *n32, const double *dx, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define saxpy32 FORTRAN_WRAPPER(saxpy32)
void saxpy32(const int *n32, const float *sa, const float  *sx,
             const int *incx32, float *sy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define daxpy32 FORTRAN_WRAPPER(daxpy32)
void daxpy32(const int *n32, const double *da, const double *dx,
             const int *incx32, double *dy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define caxpy32 FORTRAN_WRAPPER(caxpy32)
void caxpy32(const int *n32, const creal32_T *ca, const creal32_T *cx,
             const int *incx32, creal32_T *cy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define zaxpy32 FORTRAN_WRAPPER(zaxpy32)
void zaxpy32(const int *n32, const creal_T *za, const creal_T *zx,
             const int *incx32, creal_T *zy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define scopy32 FORTRAN_WRAPPER(scopy32)
void scopy32(const int *n32, const float *sx, const int *incx32, 
             float *sy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define dcopy32 FORTRAN_WRAPPER(dcopy32)
void dcopy32(const int *n32, const double *dx, const int *incx32, 
             double *dy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define ccopy32 FORTRAN_WRAPPER(ccopy32)
void ccopy32(const int *n32, const creal32_T *cx, const int *incx32, 
             creal32_T *cy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define zcopy32 FORTRAN_WRAPPER(zcopy32)
void zcopy32(const int *n32, creal_T *zx, const int *incx32, 
             creal_T *zy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define cdotc32 FORTRAN_WRAPPER(cdotc32)
complex cdotc32(const int *n32, const creal32_T *cx, const int *incx32,
              const creal32_T *cy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define zdotc32 FORTRAN_WRAPPER(zdotc32)
doublecomplex zdotc32(const int *n32, const creal_T *zx, const int *incx32,
               const creal_T *zy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define sdot32 FORTRAN_WRAPPER(sdot32)
float sdot32(const int *n32, const float *sx, const int *incx32, 
             const float  *sy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define ddot32 FORTRAN_WRAPPER(ddot32)
double ddot32(const int *n32, const double *dx, const int *incx32, 
              const double *dy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define cdotu32 FORTRAN_WRAPPER(cdotu32)
complex cdotu32(const int *n32, const creal32_T *cx, const int *incx32, 
              const creal32_T *cy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define zdotu32 FORTRAN_WRAPPER(zdotu32)
doublecomplex zdotu32(const int *n32, const creal_T *zx, const int *incx32, 
               const creal_T *zy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define sgemm32 FORTRAN_WRAPPER(sgemm32)
void sgemm32(char *transa, char *transb, const int *m32, const int *n32,
             const int *k32, const float *alpha, const float *a, const int *lda32,
             const float *b, const int *ldb32, const float *beta, float *c,
             const int *ldc32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define dgemm32 FORTRAN_WRAPPER(dgemm32)
void dgemm32(char *transa, char *transb, const int *m32, const int *n32,
             const int *k32, const double *alpha, const double *a, const int *lda32,
             const double *b, const int *ldb32, const double *beta, double  *c,
             const int *ldc32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define cgemm32 FORTRAN_WRAPPER(cgemm32)
void cgemm32(char *transa, char *transb, const int *m32, const int *n32,
             const int *k32, const creal32_T *alpha, const creal32_T *a, const int *lda32,
             const creal32_T *b, const int *ldb32, const creal32_T *beta, creal32_T *c,
             const int *ldc32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define zgemm32 FORTRAN_WRAPPER(zgemm32)
void zgemm32(char *transa, char *transb, const int *m32, const int *n32,
             const int *k32, const creal_T *alpha, const creal_T *a, const int *lda32,
             const creal_T *b, const int *ldb32, const creal_T *beta, creal_T *c,
             const int *ldc32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define sgemv32 FORTRAN_WRAPPER(sgemv32)
void sgemv32(char *trans, const int *m32, const int *n32, const float *alpha,
             const float *a, const int *lda32, const float *x, const int *incx32, 
             const float *beta, float *y, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define dgemv32 FORTRAN_WRAPPER(dgemv32)
void dgemv32(char *trans, const int *m32, const int *n32, const double *alpha,
             const double *a, const int *lda32, const double *x, const int *incx32, 
             const double *beta, double *y, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define cgemv32 FORTRAN_WRAPPER(cgemv32)
void cgemv32(char *trans, const int *m32, const int *n32, const creal32_T *alpha,
             const creal32_T *a, const int *lda32, const creal32_T *x, const int *incx32, 
             const creal32_T *beta, creal32_T *y, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define zgemv32 FORTRAN_WRAPPER(zgemv32)
void zgemv32(char *trans, const int *m32, const int *n32, const creal_T *alpha,
             const creal_T *a, const int *lda32, const creal_T *x, const int *incx32, 
             const creal_T *beta, creal_T *y, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define cgerc32 FORTRAN_WRAPPER(cgerc32)
void cgerc32(const int *m32, const int *n32, const creal32_T *alpha, const creal32_T *x,
             const int *incx32, const creal32_T *y, const int *incy32,
             creal32_T *a, const int *lda32);     

EXTERN_C LIBMWBLASCOMPAT32_API
#define zgerc32 FORTRAN_WRAPPER(zgerc32)
void zgerc32(const int *m32, const int *n32, const creal_T *alpha, const creal_T *x,
             const int *incx32, creal_T *y, const int *incy32,
             creal_T *a, const int *lda32);
     
EXTERN_C LIBMWBLASCOMPAT32_API
#define sger32 FORTRAN_WRAPPER(sger32)
void sger32(const int *m32, const int *n32, const float *alpha, const float  *x,
            const int *incx32, const float *y, const int *incy32, 
            float *a, const int *lda32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define dger32 FORTRAN_WRAPPER(dger32)
void dger32(const int *m32, const int *n32, const double *alpha, const double *x,
            const int *incx32, const double *y, const int *incy32,
            double *a, const int *lda32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define cgeru32 FORTRAN_WRAPPER(cgeru32)
void cgeru32(const int *m32, const int *n32, const creal32_T *alpha, const creal32_T *x,
             const int *incx32, const creal32_T *y, const int *incy32,
             creal32_T *a, const int *lda32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define zgeru32 FORTRAN_WRAPPER(zgeru32)
void zgeru32(const int *m32, const int *n32, const creal_T *alpha, const creal_T *x,
             const int *incx32, creal_T *y, const int *incy32,
             creal_T *a, const int *lda32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define snrm232 FORTRAN_WRAPPER(snrm232)
float snrm232(const int *n32, const float *x, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define dnrm232 FORTRAN_WRAPPER(dnrm232)
double dnrm232(const int *n32, const double *x, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define scnrm232 FORTRAN_WRAPPER(scnrm232)
float scnrm232(const int *n32, const creal32_T *x, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define dznrm232 FORTRAN_WRAPPER(dznrm232)
double dznrm232(const int *n32, const creal_T *x, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define srotg32 FORTRAN_WRAPPER(srotg32)
void srotg32(float  *sa, float  *sb, float  *c, float  *s);

EXTERN_C LIBMWBLASCOMPAT32_API
#define drotg32 FORTRAN_WRAPPER(drotg32)
void drotg32(double *da, double *db, double *c, double *s);

EXTERN_C LIBMWBLASCOMPAT32_API
#define crotg32 FORTRAN_WRAPPER(crotg32)
void crotg32(creal32_T *ca, creal32_T *cb, creal32_T *c, creal32_T *s);

EXTERN_C LIBMWBLASCOMPAT32_API
#define zrotg32 FORTRAN_WRAPPER(zrotg32)
void zrotg32(creal_T *ca, creal_T *cb, creal_T *c, creal_T *s);

EXTERN_C LIBMWBLASCOMPAT32_API
#define srot32 FORTRAN_WRAPPER(srot32)
void srot32(const int *n32, float  *sx, const int *incx32, float  *sy, 
            const int *incy32, const float *c, const float *s);

EXTERN_C LIBMWBLASCOMPAT32_API
#define drot32 FORTRAN_WRAPPER(drot32)
void drot32(const int *n32, double *dx, const int *incx32, double *dy,
            const int *incy32, const double *c, const double *s);

EXTERN_C LIBMWBLASCOMPAT32_API
#define csrot32 FORTRAN_WRAPPER(csrot32)
void csrot32(const int *n32, creal32_T *cx, const int *incx32, creal32_T *cy, 
             const int *incy32, const float *c, const creal32_T *s);

EXTERN_C LIBMWBLASCOMPAT32_API
#define zdrot32 FORTRAN_WRAPPER(zdrot32)
void zdrot32(const int *n32, creal_T *cx, const int *incx32, creal_T *cy, 
             const int *incy32, const double *c, const creal_T *s);

EXTERN_C LIBMWBLASCOMPAT32_API
#define sscal32 FORTRAN_WRAPPER(sscal32)
void sscal32(const int *n32, const float *sa, float  *sx, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define dscal32 FORTRAN_WRAPPER(dscal32)
void dscal32(const int *n32, const double *da, double *dx, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define cscal32 FORTRAN_WRAPPER(cscal32)
void cscal32(const int *n32, const creal32_T *ca, creal32_T *cx, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define zscal32 FORTRAN_WRAPPER(zscal32)
void zscal32(const int *n32, const creal_T *za, creal_T *zx, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define sswap32 FORTRAN_WRAPPER(sswap32)
void sswap32(const int *n32, float  *sx, const int *incx32, float  *sy, 
             const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define dswap32 FORTRAN_WRAPPER(dswap32)
void dswap32(const int *n32, double *dx, const int *incx32, double *dy,
             const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define cswap32 FORTRAN_WRAPPER(cswap32)
void cswap32(const int *n32, creal32_T *cx, const int *incx32, 
             creal32_T *cy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define zswap32 FORTRAN_WRAPPER(zswap32)
void zswap32(const int *n32, creal_T *zx, const int *incx32, 
             creal_T *zy, const int *incy32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define strsm32 FORTRAN_WRAPPER(strsm32)
void strsm32(char   *side, char   *uplo, char   *transa, 
             char *diag, const int *m32, const int *n32,
             const float *alpha, const float *a, const int *lda32, 
             float  *b, const int *ldb32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define dtrsm32 FORTRAN_WRAPPER(dtrsm32)
void dtrsm32(char   *side, char   *uplo, char   *transa, 
             char *diag, const int *m32, const int *n32, 
             const double *alpha, const double *a, const int *lda32, 
             double *b, const int *ldb32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define ctrsm32 FORTRAN_WRAPPER(ctrsm32)
void ctrsm32(char   *side, char   *uplo, char   *transa, 
             char *diag, const int *m32, const int *n32, 
             const creal32_T *alpha, const creal32_T *a, const int *lda32, 
             creal32_T *b, const int *ldb32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define ztrsm32 FORTRAN_WRAPPER(ztrsm32)
void ztrsm32(char   *side, char   *uplo, char   *transa,
             char *diag, const int *m32, const int *n32,
             const creal_T *alpha, const creal_T *a, const int *lda32, 
             creal_T *b, const int *ldb32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define strsv32 FORTRAN_WRAPPER(strsv32)
void strsv32(char   *uplo, char   *trans, char   *diag,
                const int *n32, const float *a, const int *lda32,
                float *x, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define dtrsv32 FORTRAN_WRAPPER(dtrsv32)
void dtrsv32(char   *uplo, char   *trans, char   *diag,
                const int *n32, const double *a, const int *lda32, 
                double *x, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define ctrsv32 FORTRAN_WRAPPER(ctrsv32)
void ctrsv32(char   *uplo, char   *trans, char   *diag, 
                const int *n32, const creal32_T *a, const int *lda32, 
                creal32_T *x, const int *incx32);

EXTERN_C LIBMWBLASCOMPAT32_API
#define ztrsv32 FORTRAN_WRAPPER(ztrsv32)
void ztrsv32(char   *uplo, char   *trans, char   *diag,
                const int *n32, const creal_T *a, const int *lda32, 
                creal_T *x, const int *incx32);

#endif /* blascompat32_h */
