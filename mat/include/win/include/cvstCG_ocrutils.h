/* Copyright 2013 The MathWorks, Inc. */
#ifndef _OCRUTILS_
#define _OCRUTILS_

#ifndef OCRUTILS_API
#    define OCRUTILS_API
#endif

#ifndef EXTERN_C
#  ifdef __cplusplus
#    define EXTERN_C extern "C"
#  else
#    define EXTERN_C extern
#  endif
#endif


#ifdef MATLAB_MEX_FILE
#include "tmwtypes.h"
#else
#include "rtwtypes.h"
#endif

EXTERN_C OCRUTILS_API
int32_T tesseractRecognizeTextUint8(void **, const uint8_T * I, char ** utf8Text,
                                    const int32_T width, const int32_T height,
                                    const char * textLayout, const char * charSet,
                                    const char * tessdata, const char * lang);

EXTERN_C OCRUTILS_API
int32_T tesseractRecognizeTextLogical(void **, const boolean_T * I, char ** utf8Text,
                                      const int32_T width, const int32_T height,
                                      const char * textLayout, const char * charSet,
                                      const char * tessdata, const char * lang);

EXTERN_C OCRUTILS_API
void cleanupTesseract(void *);

EXTERN_C OCRUTILS_API
void copyTextAndCleanup(char * src, uint8_T * dest, const size_t length);

EXTERN_C OCRUTILS_API
int32_T getTextFromMetadata(void * ocrMetadata, char ** utf8Text);

EXTERN_C OCRUTILS_API
void cleanupMetadata(void *);

EXTERN_C OCRUTILS_API
void collectMetadata(void * tessAPI, void ** ocrMetadata,
                     int32_T * numChars, int32_T * numWords, int32_T * numTextLines,
                     int32_T * numParagraphs, int32_T * numBlocks);

EXTERN_C OCRUTILS_API
void copyMetadata(void * ocrMetadata,
                  double * charBBox,
                  int32_T * charWordIndex,
                  float * charConfidence,
                  double * wordBBox,
                  int32_T * wordTextLineIndex,
                  float * wordConfidence,
                  int32_T * wordCharacterIndex,
                  double * textlineBBox,
                  int32_T * textlineParagraphIndex,
                  float * textlineConfidence,
                  int32_T * textlineCharacterIndex,
                  double * paragraphBBox,
                  int32_T * paragraphBlockIndex,
                  float * paragraphConfidence,
                  int32_T * paragraphCharacterIndex,
                  double * blockBBox,
                  int32_T * blockPageIndex,
                  float * blockConfidence,
                  int32_T * blockCharacterIndex);
#endif  


