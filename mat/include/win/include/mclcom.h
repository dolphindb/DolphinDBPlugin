#ifndef MCLCOM_H
#define MCLCOM_H

#if defined(_MSC_VER)
# pragma once
#endif
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3))
# pragma once
#endif

/* Struct for passing data conversion flags */
typedef struct _MCLCONVERSION_FLAGS
{
    mwArrayFormat InputFmt;     /* Input array format */
    long nInputInd;             /* Input array format indirection flag */
    mwArrayFormat OutputFmt;    /* Output array format */
    long nOutputInd;            /* Output array format indirection flag */
    bool bAutoResize;           /* Auto-resize-output flag for Excel ranges */
    bool bTranspose;            /* Transpose-output flag */
    long nTransposeInd;         /* Transpose-output indirection flag */
    mxClassID nCoerceNumeric;   /* Coerce-all-numeric-input-to-type flag */
    mwDateFormat InputDateFmt;  /* Input date format */
    mxComplexity Complexity;    /* Input-is-complex flag */
    bool bReal;                 /* Copy-to-real/imag-buffer flag */
    bool bOutputAsDate;         /* Coerce-output-to-date flag */
    long nDateBias;             /* Date bias to use in date conversion */
    mwReplaceMissingData ReplaceMissing; /* Value to substitute for missing data for inputs*/
	mwReplaceMatlabNaN   ReplaceMatlabNaN; /*Value to substitute matlab NaN for outputs*/
} _MCLCONVERSION_FLAGS, *MCLCONVERSION_FLAGS;

#endif
