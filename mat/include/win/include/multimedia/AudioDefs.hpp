/*
 * AudioDefs.h - Definitions for audio I/O
 *
 *  Copyright 1995-2012 The MathWorks, Inc.
 */

#ifndef AUDIODEFS_H
#define AUDIODEFS_H


/* these follow Simulink data types, in simstruc_types.h */
typedef enum
{
    AudioDataType_Double = 0,  /* double          */
    AudioDataType_Single,      /* float           */
    AudioDataType_Int8,        /* char            */
    AudioDataType_Uint8,       /* unsigned char   */
    AudioDataType_Int16,       /* short           */
    AudioDataType_Uint16,      /* unsigned short  */
    AudioDataType_Int32,       /* long            */
    AudioDataType_Uint32,      /* unsigned long   */
    AudioDataType_INVALID,     /* bool            */
    AudioDataType_Int24,       /* 24-bit signed   */
    AudioDataType_Uint24,      /* 24-bit unsigned */
    AudioDataType_NUM_TYPES
} AudioDataType;

static const int AudioDataTypeSize[]  = {8,4,1,1,2,2,4,4,1,3,3};
static const int AudioDataTypeFloat[] = {1,1,0,0,0,0,0,0,0,0,0};

/* An enumeration of typical audio data types which is used on several masks. */
typedef enum {
    AudioDT_Invalid = 0,
    AudioDT_Derived = 1,
    AudioDT_Uint8,
    AudioDT_Int16,
    AudioDT_Int24,
    AudioDT_Single,
    AudioDT_NUM_TYPES
} AudioDTE_Common;

/* The default mapping from AudioDataTypes onto common audio data types */
static const AudioDTE_Common AudioDataTypeToCommonDTE[] = {
    AudioDT_Single, 
    AudioDT_Single,
    AudioDT_Uint8,  
    AudioDT_Uint8,
    AudioDT_Int16,  
    AudioDT_Int16,
    AudioDT_Int24,  
    AudioDT_Int24,
    AudioDT_Invalid,
    AudioDT_Int24,  
    AudioDT_Int24
};

/* Mapping from common data types to Audio Data Types */
static const AudioDataType CommonDTEToAudioDataType[] = {
    AudioDataType_INVALID,
    AudioDataType_INVALID,
    AudioDataType_Uint8,
    AudioDataType_Int16,
    AudioDataType_Int24,
    AudioDataType_Single
};

typedef struct
{
    unsigned char isValid;       /* zero if the file has no audio */

    /* The following fields refer to the PCM format of the audio data as encoded in the file/device */
    unsigned char isFloat;       /* true if the samples are in floating-point format */
    double sampleRate;           /* audio sample rate */
    int numBits;                 /* audio bit depth */
    int numChannels;             /* number of audio channels (typically 1 or two) */

    /* The following fields refer to the format of the data in MATLAB/Simulink */
    AudioDataType dataType;      /* data type of audio samples */
    int frameSize;               /* number of audio samples per frame */

    /* The name of the compression format, if we are writing a file */
    const char* audioCompressor; /* set to NULL for none */
} MMAudioInfo;


#endif  /* AUDIODEFS_H */


