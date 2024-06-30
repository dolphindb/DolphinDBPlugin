#if defined(_MSC_VER) || __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3)
#pragma once
#endif

#ifndef mwmathutil_h
#define mwmathutil_h

/* Copyright 2003-2010 The MathWorks, Inc. */

/* Only define EXTERN_C if it hasn't been defined already. This allows
 * individual modules to have more control over managing their exports.
 */
#ifndef EXTERN_C

#ifdef __cplusplus
  #define EXTERN_C extern "C"
#else
  #define EXTERN_C extern
#endif

#endif

#include "tmwtypes.h"


/* abs(a) */
EXTERN_C double muDoubleScalarAbs(double a);


/* ceil(a) */
EXTERN_C double muDoubleScalarCeil(double a);


/* floor(a) */
EXTERN_C double muDoubleScalarFloor(double a);


/* round(a) */
EXTERN_C double muDoubleScalarRound(double a);


/* fix(a) */
EXTERN_C double muDoubleScalarFix(double xr);


/* acos(a) */
EXTERN_C double muDoubleScalarAcos(double a);


/* acosh(a) */
EXTERN_C double muDoubleScalarAcosh(double a);


/* asin(a) */
EXTERN_C double muDoubleScalarAsin(double a);


/* asinh(a) */
EXTERN_C double muDoubleScalarAsinh(double a);


/* atan(a) */
EXTERN_C double muDoubleScalarAtan(double a);


/* atanh(a) */
EXTERN_C double muDoubleScalarAtanh(double a);


/* cos(a) */
EXTERN_C double muDoubleScalarCos(double a);

/* cosh(a) */
EXTERN_C double muDoubleScalarCosh(double a);


/* exp(a) */	
EXTERN_C double muDoubleScalarExp(double a);


/* log */
/*A warning 'Log of zero' should be issued by users for a = 0 */
EXTERN_C double muDoubleScalarLog(double a);


/* log10 */
/*A warning 'Log of zero' should be issued by users for a = 0 */
EXTERN_C double muDoubleScalarLog10(double a);


/* atan2(a,b) */
EXTERN_C double muDoubleScalarAtan2(double a, double b);


/* max(a,b) */
EXTERN_C double muDoubleScalarMax(double a, double b);


/* min(a,b) */
EXTERN_C double muDoubleScalarMin(double a, double b);


/* power(a,b) */
EXTERN_C double muDoubleScalarPower(double a, double b);


/* sin(a) */
EXTERN_C double muDoubleScalarSin(double a);


/* [s,c] = sincos(a) */
EXTERN_C void muDoubleScalarSinCos(double a, double* s, double* c);


/* sign(a) */
EXTERN_C double muDoubleScalarSign(double a);


/* sinh(a) */
EXTERN_C double muDoubleScalarSinh(double a);


/* sqrt(a) */
EXTERN_C double muDoubleScalarSqrt(double a);


/* tan(a) */
EXTERN_C double muDoubleScalarTan(double a);


/* tanh(a) */
EXTERN_C double muDoubleScalarTanh(double a);


/* mod(a,b) */
EXTERN_C double muDoubleScalarMod(double a, double b);


/* rem(a,b) */
EXTERN_C double muDoubleScalarRem(double a, double b);


/* hypot(a,b) */
EXTERN_C double muDoubleScalarHypot(double a,double b);

/* frexp(a,b) */
EXTERN_C double muDoubleScalarFrexp(double x, int *eptr);


/* abs(cmplx a) */
EXTERN_C double muDoubleComplexScalarAbs(double xr, double xi);

/* [br, bi] = exp(ar, ai) */
EXTERN_C void muDoubleComplexScalarExp(double *pyr, double *pyi, double xr, double xi);

/* hypot(ar, ai) */
EXTERN_C double muDoubleComplexScalarHypot(double ar, double ai, double br, double bi);

/* [br, bi] = log(ar, ai) */
EXTERN_C void muDoubleComplexScalarLog(double *pyr, double *pyi, double xr, double xi);

/* [cr, ci] = power(ar, ai, br, bi) */
EXTERN_C void muDoubleComplexScalarPower(double *cr, double *ci, double ar, double ai, double br, double bi);

/* [br, bi] = sqrt(ar, ai) */
EXTERN_C void muDoubleComplexScalarSqrt(double *br, double *bi, double ar, double ai);

/* [br, bi] = sign(ar, ai) */
EXTERN_C void muDoubleComplexScalarSign(double *br, double *bi, double ar, double ai);

/* [br, bi] = sin(ar, ai) */
EXTERN_C void muDoubleComplexScalarSin(double *br, double *bi, double ar, double ai);

/* [br, bi] = cos(ar, ai) */
EXTERN_C void muDoubleComplexScalarCos(double *br, double *bi, double ar, double ai);

/* [br, bi] = cos(ar, ai) */
EXTERN_C creal_T muDoubleComplexScalarCos2(creal_T x);

/* [Sr, Si, Cr, Ci] = sincos(ar, ai) */
EXTERN_C void muDoubleComplexScalarSinCos(double *pySr, double *pySi, double *pyCr, double *pyCi,
                                          double xr, double xi);

/* [br, bi] = tan(ar, ai) */
EXTERN_C void muDoubleComplexScalarTan(double *br, double *bi, double ar, double ai);

/* [br, bi] = atan(ar, ai) */
EXTERN_C void muDoubleComplexScalarAtan(double *br, double *bi, double ar, double ai);

/* [br, bi] = asin(ar, ai) */
EXTERN_C void muDoubleComplexScalarAsin(double *br, double *bi, double ar, double ai);

/* [br, bi] = acos(ar, ai) */
EXTERN_C void muDoubleComplexScalarAcos(double *br, double *bi, double ar, double ai);

/* [br, bi] = sinh(ar, ai) */
EXTERN_C void muDoubleComplexScalarSinh(double *br, double *bi, double ar, double ai);

/* [br, bi] = cosh(ar, ai) */
EXTERN_C void muDoubleComplexScalarCosh(double *br, double *bi, double ar, double ai);

/* [br, bi] = tanh(ar, ai) */
EXTERN_C void muDoubleComplexScalarTanh(double *br, double *bi, double ar, double ai);

/* [br, bi] = asinh(ar, ai) */
EXTERN_C void muDoubleComplexScalarAsinh(double *br, double *bi, double ar, double ai);

/* [br, bi] = acosh(ar, ai) */
EXTERN_C void muDoubleComplexScalarAcosh(double *br, double *bi, double ar, double ai);


/* Single-precision functions */


/* abs(a) */
EXTERN_C float muSingleScalarAbs(float a);


/* ceil(a) */
EXTERN_C float muSingleScalarCeil(float a);


/* floor(a) */
EXTERN_C float muSingleScalarFloor(float a);


/* round(a) */
EXTERN_C float muSingleScalarRound(float a);


/* fix(a) */
EXTERN_C float muSingleScalarFix(float xr);


/* acos(a) */
EXTERN_C float muSingleScalarAcos(float a);


/* acosh(a) */
EXTERN_C float muSingleScalarAcosh(float a);


/* asin(a) */
EXTERN_C float muSingleScalarAsin(float a);


/* asinh(a) */
EXTERN_C float muSingleScalarAsinh(float a);


/* atan(a) */
EXTERN_C float muSingleScalarAtan(float a);


/* atanh(a) */
EXTERN_C float muSingleScalarAtanh(float a);


/* cos(a) */
EXTERN_C float muSingleScalarCos(float a);


/* cosh(a) */
EXTERN_C float muSingleScalarCosh(float a);


/* exp(a) */	
EXTERN_C float muSingleScalarExp(float a);


/* log */
/*A warning 'Log of zero' should be issued by users for a = 0 */
EXTERN_C float muSingleScalarLog(float a);


/* log10 */
/*A warning 'Log of zero' should be issued by users for a = 0 */
EXTERN_C float muSingleScalarLog10(float a);


/* atan2(a,b) */
EXTERN_C float muSingleScalarAtan2(float a, float b);


/* max(a,b) */
EXTERN_C float muSingleScalarMax(float a, float b);


/* min(a,b) */
EXTERN_C float muSingleScalarMin(float a, float b);


/* power(a,b) */
EXTERN_C float muSingleScalarPower(float a, float b);


/* sin(a) */
EXTERN_C float muSingleScalarSin(float a);


/* [s,c] = sincos(a) */
EXTERN_C void muSingleScalarSinCos(float a, float* s, float* c);


/* sign(a) */
EXTERN_C float muSingleScalarSign(float a);


/* sinh(a) */
EXTERN_C float muSingleScalarSinh(float a);


/* sqrt(a) */
EXTERN_C float muSingleScalarSqrt(float a);


/* tan(a) */
EXTERN_C float muSingleScalarTan(float a);


/* tanh(a) */
EXTERN_C float muSingleScalarTanh(float a);


/* mod(a,b) */
EXTERN_C float muSingleScalarMod(float a, float b);


/* rem(a,b) */
EXTERN_C float muSingleScalarRem(float a, float b);


/* hypot(a,b) */
EXTERN_C float muSingleScalarHypot(float a,float b);

/* frexp(a,b) */
EXTERN_C float muSingleScalarFrexp(float x, int *eptr);

/* abs(cmplx a) */
EXTERN_C float muSingleComplexScalarAbs(float xr, float xi);

/* [br, bi] = exp(ar, ai) */
EXTERN_C void muSingleComplexScalarExp(float *pyr, float *pyi, float xr, float xi);

/* hypot(ar, ai) */
EXTERN_C float muSingleComplexScalarHypot(float ar, float ai, float br, float bi);

/* [br, bi] = log(ar, ai) */
EXTERN_C void muSingleComplexScalarLog(float *pyr, float *pyi, float xr, float xi);

/* [cr, ci] = power(ar, ai, br, bi) */
EXTERN_C void muSingleComplexScalarPower(float *cr, float *ci, float ar, float ai, float br, float bi);

/* [br, bi] = sqrt(ar, ai) */
EXTERN_C void muSingleComplexScalarSqrt(float *br, float *bi, float ar, float ai);

/* [br, bi] = sign(ar, ai) */
EXTERN_C void muSingleComplexScalarSign(float *br, float *bi, float ar, float ai);

/* [br, bi] = sin(ar, ai) */
EXTERN_C void muSingleComplexScalarSin(float *br, float *bi, float ar, float ai);

/* [br, bi] = cos(ar, ai) */
EXTERN_C void muSingleComplexScalarCos(float *br, float *bi, float ar, float ai);

/* [br, bi] = cos(ar, ai) */
EXTERN_C creal32_T muSingleComplexScalarCos2(creal32_T x);

/* [Sr, Si, Cr, Ci] = sincos(ar, ai) */
EXTERN_C void muSingleComplexScalarSinCos(float *pySr, float *pySi, float *pyCr, float *pyCi,
                                          float xr, float xi);

/* [br, bi] = tan(ar, ai) */
EXTERN_C void muSingleComplexScalarTan(float *br, float *bi, float ar, float ai);

/* [br, bi] = atan(ar, ai) */
EXTERN_C void muSingleComplexScalarAtan(float *br, float *bi, float ar, float ai);

/* [br, bi] = asin(ar, ai) */
EXTERN_C void muSingleComplexScalarAsin(float *br, float *bi, float ar, float ai);

/* [br, bi] = acos(ar, ai) */
EXTERN_C void muSingleComplexScalarAcos(float *br, float *bi, float ar, float ai);

/* [br, bi] = sinh(ar, ai) */
EXTERN_C void muSingleComplexScalarSinh(float *br, float *bi, float ar, float ai);

/* [br, bi] = cosh(ar, ai) */
EXTERN_C void muSingleComplexScalarCosh(float *br, float *bi, float ar, float ai);

/* [br, bi] = tanh(ar, ai) */
EXTERN_C void muSingleComplexScalarTanh(float *br, float *bi, float ar, float ai);

/* [br, bi] = asinh(ar, ai) */
EXTERN_C void muSingleComplexScalarAsinh(float *br, float *bi, float ar, float ai);

/* [br, bi] = acosh(ar, ai) */
EXTERN_C void muSingleComplexScalarAcosh(float *br, float *bi, float ar, float ai);


#ifdef __WATCOMC__
#pragma aux muDoubleScalarAbs value [8087];
#pragma aux muDoubleScalarCeil value [8087];
#pragma aux muDoubleScalarFloor value [8087];
#pragma aux muDoubleScalarRound value [8087];
#pragma aux muDoubleScalarAcos value [8087];
#pragma aux muDoubleScalarAcosh value [8087];
#pragma aux muDoubleScalarAsin value [8087];
#pragma aux muDoubleScalarAsinh value [8087];
#pragma aux muDoubleScalarAtan value [8087];
#pragma aux muDoubleScalarAtanh value [8087];
#pragma aux muDoubleScalarCos value [8087];
#pragma aux muDoubleScalarCosh value [8087];
#pragma aux muDoubleScalarExp value [8087];
#pragma aux muDoubleScalarLog value [8087];
#pragma aux muDoubleScalarLog10 value [8087];
#pragma aux muDoubleScalarAtan2 value [8087]
#pragma aux muDoubleScalarMax value [8087]
#pragma aux muDoubleScalarMin value [8087]
#pragma aux muDoubleScalarPower value [8087]
#pragma aux muDoubleScalarSin value [8087];
#pragma aux muDoubleScalarSinCos value [8087];
#pragma aux muDoubleScalarSign value [8087];
#pragma aux muDoubleScalarSinh value [8087];
#pragma aux muDoubleScalarSqrt value [8087];
#pragma aux muDoubleScalarTan value [8087];
#pragma aux muDoubleScalarTanh value [8087];
#pragma aux muDoubleScalarMod value [8087]
#pragma aux muDoubleScalarRem value [8087]
#pragma aux muDoubleScalarHypot value [8087];
#pragma aux muDoubleScalarFix value [8087];
#pragma aux muDoubleScalarFrexp value [8087];
#pragma aux muDoubleComplexScalarAbs value [8087];
#pragma aux muDoubleComplexScalarExp value [8087];
#pragma aux muDoubleComplexScalarHypot value [8087];
#pragma aux muDoubleComplexScalarLog value [8087];
#pragma aux muDoubleComplexScalarPower value [8087];
#pragma aux muDoubleComplexScalarSqrt value [8087];
#pragma aux muDoubleComplexScalarSign value [8087];
#pragma aux muDoubleComplexScalarSin value [8087];
#pragma aux muDoubleComplexScalarCos value [8087];
#pragma aux muDoubleComplexScalarSinCos value [8087];
#pragma aux muDoubleComplexScalarTan value [8087];
#pragma aux muDoubleComplexScalarAtan value [8087];
#pragma aux muDoubleComplexScalarAsin value [8087];
#pragma aux muDoubleComplexScalarAcos value [8087];
#pragma aux muDoubleComplexScalarSinh value [8087];
#pragma aux muDoubleComplexScalarCosh value [8087];
#pragma aux muDoubleComplexScalarTanh value [8087];
#pragma aux muDoubleComplexScalarAsinh value [8087];
#pragma aux muDoubleComplexScalarAcosh value [8087];
#pragma aux muSingleScalarAbs value [8087];
#pragma aux muSingleScalarCeil value [8087];
#pragma aux muSingleScalarFloor value [8087];
#pragma aux muSingleScalarRound value [8087];
#pragma aux muSingleScalarAcos value [8087];
#pragma aux muSingleScalarAcosh value [8087];
#pragma aux muSingleScalarAsin value [8087];
#pragma aux muSingleScalarAsinh value [8087];
#pragma aux muSingleScalarAtan value [8087];
#pragma aux muSingleScalarAtanh value [8087];
#pragma aux muSingleScalarCos value [8087];
#pragma aux muSingleScalarCosh value [8087];
#pragma aux muSingleScalarExp value [8087];
#pragma aux muSingleScalarLog value [8087];
#pragma aux muSingleScalarLog10 value [8087];
#pragma aux muSingleScalarAtan2 value [8087]
#pragma aux muSingleScalarMax value [8087]
#pragma aux muSingleScalarMin value [8087]
#pragma aux muSingleScalarPower value [8087]
#pragma aux muSingleScalarSin value [8087];
#pragma aux muSingleScalarSinCos value [8087];
#pragma aux muSingleScalarSign value [8087];
#pragma aux muSingleScalarSinh value [8087];
#pragma aux muSingleScalarSqrt value [8087];
#pragma aux muSingleScalarTan value [8087];
#pragma aux muSingleScalarTanh value [8087];
#pragma aux muSingleScalarMod value [8087]
#pragma aux muSingleScalarRem value [8087]
#pragma aux muSingleScalarHypot value [8087];
#pragma aux muSingleScalarFix value [8087];
#pragma aux muSingleScalarFrexp value [8087];
#pragma aux muSingleComplexScalarAbs value [8087];
#pragma aux muSingleComplexScalarExp value [8087];
#pragma aux muSingleComplexScalarHypot value [8087];
#pragma aux muSingleComplexScalarLog value [8087];
#pragma aux muSingleComplexScalarPower value [8087];
#pragma aux muSingleComplexScalarSqrt value [8087];
#pragma aux muSingleComplexScalarSign value [8087];
#pragma aux muSingleComplexScalarSin value [8087];
#pragma aux muSingleComplexScalarCos value [8087];
#pragma aux muSingleComplexScalarSinCos value [8087];
#pragma aux muSingleComplexScalarTan value [8087];
#pragma aux muSingleComplexScalarAtan value [8087];
#pragma aux muSingleComplexScalarAsin value [8087];
#pragma aux muSingleComplexScalarAcos value [8087];
#pragma aux muSingleComplexScalarSinh value [8087];
#pragma aux muSingleComplexScalarCosh value [8087];
#pragma aux muSingleComplexScalarTanh value [8087];
#pragma aux muSingleComplexScalarAsinh value [8087];
#pragma aux muSingleComplexScalarAcosh value [8087];
#endif

/* Integer Scalar functions. */
/* Integer Abs Family. */
EXTERN_C uint8_T muIntScalarAbs_uint8(uint8_T a);

EXTERN_C int8_T muIntScalarAbs_sint8(int8_T a);

EXTERN_C uint16_T muIntScalarAbs_uint16(uint16_T a);

EXTERN_C int16_T muIntScalarAbs_sint16(int16_T a);

EXTERN_C uint32_T muIntScalarAbs_uint32(uint32_T a);

EXTERN_C int32_T muIntScalarAbs_sint32(int32_T a);

/* Integer Max Family. */
EXTERN_C uint8_T muIntScalarMax_uint8(uint8_T a, uint8_T b);

EXTERN_C int8_T muIntScalarMax_sint8(int8_T a, int8_T b);

EXTERN_C uint16_T muIntScalarMax_uint16(uint16_T a, uint16_T b);

EXTERN_C int16_T muIntScalarMax_sint16(int16_T a, int16_T b);

EXTERN_C uint32_T muIntScalarMax_uint32(uint32_T a, uint32_T b);

EXTERN_C int32_T muIntScalarMax_sint32(int32_T a, int32_T b);

/* Integer Min Family. */
EXTERN_C uint8_T muIntScalarMin_uint8(uint8_T a, uint8_T b);

EXTERN_C int8_T muIntScalarMin_sint8(int8_T a, int8_T b);

EXTERN_C uint16_T muIntScalarMin_uint16(uint16_T a, uint16_T b);

EXTERN_C int16_T muIntScalarMin_sint16(int16_T a, int16_T b);

EXTERN_C uint32_T muIntScalarMin_uint32(uint32_T a, uint32_T b);

EXTERN_C int32_T muIntScalarMin_sint32(int32_T a, int32_T b);

/* Integer Mod Family. */
EXTERN_C uint8_T muIntScalarMod_uint8(uint8_T a, uint8_T b);

EXTERN_C int8_T muIntScalarMod_sint8(int8_T a, int8_T b);

EXTERN_C uint16_T muIntScalarMod_uint16(uint16_T a, uint16_T b);

EXTERN_C int16_T muIntScalarMod_sint16(int16_T a, int16_T b);

EXTERN_C uint32_T muIntScalarMod_uint32(uint32_T a, uint32_T b);

EXTERN_C int32_T muIntScalarMod_sint32(int32_T a, int32_T b);

/* Integer Rem Family. */
EXTERN_C uint8_T muIntScalarRem_uint8(uint8_T a, uint8_T b);

EXTERN_C int8_T muIntScalarRem_sint8(int8_T a, int8_T b);

EXTERN_C uint16_T muIntScalarRem_uint16(uint16_T a, uint16_T b);

EXTERN_C int16_T muIntScalarRem_sint16(int16_T a, int16_T b);

EXTERN_C uint32_T muIntScalarRem_uint32(uint32_T a, uint32_T b);

EXTERN_C int32_T muIntScalarRem_sint32(int32_T a, int32_T b);

/* Integer Sign Family. */
EXTERN_C uint8_T muIntScalarSign_uint8(uint8_T a);

EXTERN_C int8_T muIntScalarSign_sint8(int8_T a);

EXTERN_C uint16_T muIntScalarSign_uint16(uint16_T a);

EXTERN_C int16_T muIntScalarSign_sint16(int16_T a);

EXTERN_C uint32_T muIntScalarSign_uint32(uint32_T a);

EXTERN_C int32_T muIntScalarSign_sint32(int32_T a);

/* isFinite(a) */
EXTERN_C boolean_T muDoubleScalarIsFinite(double a);

/* isNaN(a) */
EXTERN_C boolean_T muDoubleScalarIsNaN(double a);

/* isInf(a) */
EXTERN_C boolean_T muDoubleScalarIsInf(double a);

/* isFinite(ar, ai) */
EXTERN_C boolean_T muDoubleComplexScalarIsFinite(double ar, double ai);

/* isNaN(a) */
EXTERN_C boolean_T muDoubleComplexScalarIsNaN(double ar, double ai);

/* isInf(ar, ai) */
EXTERN_C boolean_T muDoubleComplexScalarIsInf(double ar, double ai);


/* Single-precision functions */

/* isFinite(a) */
EXTERN_C boolean_T muSingleScalarIsFinite(float a);

/* isNaN(a) */
EXTERN_C boolean_T muSingleScalarIsNaN(float a);

/* isInf(a) */
EXTERN_C boolean_T muSingleScalarIsInf(float a);

/* isFinite(ar, ai) */
EXTERN_C boolean_T muSingleComplexScalarIsFinite(float ar, float ai);

/* isNaN(a) */
EXTERN_C boolean_T muSingleComplexScalarIsNaN(float ar, float ai);

/* isInf(ar, ai) */
EXTERN_C boolean_T muSingleComplexScalarIsInf(float ar, float ai);


#endif /* mwmathutil_h */
