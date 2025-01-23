/*
 * VideoDefs.h - Definitions for video I/O
 *
 *  Copyright 1995-2013 The MathWorks, Inc.
 */

#ifndef VIDEODEFS_H
#define VIDEODEFS_H

typedef enum
{
    VideoFrame_RowMajor = 0,
    VideoFrame_ColumnMajor
} VideoFrameOrientation;

typedef enum
{
    LineOrder_TopDown = 0,
    LineOrder_BottomUp
} VideoFrameLineOrder;

typedef enum {
    FOURCC_INVALID,
    FOURCC_RGB,
    FOURCC_GREY,
    FOURCC_YUY2
} FourCCType;

static const int    NumBandsForFourCCType[] = {0, 3, 1, 3};

/* Ratio of the total image size in bytes to the size of a full band for different FourCC Types */
static const double TotalSizeToOneFullBandSizeInBytesRatio[] = {0.0, 3.0, 1.0, 2.0};

typedef enum {
    FILETYPE_INVALID,
    FILETYPE_AVI,
    FILETYPE_WAV,
    FILETYPE_WMV,
    FILETYPE_WMA,
    FILETYPE_FLAC,
    FILETYPE_OGG,
    FILETYPE_MPEG4,
    FILETYPE_MJ2000,
    FILETYPE_NUMTYPES
} MMFileType;


/* these follow Simulink data types, in simstruc_types.h */
typedef enum
{
    VideoDataType_Double = 0,   /* double */
    VideoDataType_Single,       /* float */
    VideoDataType_Int8,         /* char */
    VideoDataType_Uint8,        /* unsigned char */
    VideoDataType_Int16,        /* short */
    VideoDataType_Uint16,       /* unsigned short */
    VideoDataType_Int32,        /* long */
    VideoDataType_Uint32,       /* unsigned long */
    VideoDataType_Boolean,      /* bool */
    VideoDataType_INVALID,
    VideoDataType_NUMTYPES
} VideoDataType;

static const int VideoDataTypeSize[] = {8,4,1,1,2,2,4,4,1,0};

typedef struct
{
    unsigned char isValid;    /* Zero if the file has no video */

    /* The following fields refer to the format of the video data as encoded in the file/device */
    double frameRate;         /* video num frames per second */
    double frameRateComputed; /* video num frames per second, as computed by the video framework.
                               * The reason that this field is necessary is that on Windows, 
                               * this number is less accurate than the frameRate field, yet it is essential
                               * to use this value in methods that determine which frame to emit.
                               * The frameRate field determines the Sample Time on the 
                               * Simulink wire, to be consistent with the UNIX implementation.
                               */
    char fourcc[4];           /* set to the FOURCC code of the video encoding (e.g. RGB, YUV)*/
    int numPorts;             /* The number of separate signals (ports) used for the image */
    int numBands;             /* The number of bands (or color planes) in the image */
    int bandWidth[3];         /* width of the video bands (RGB or intensity/chroma if YCbCr) */
    int bandHeight[3];        /* height of the video bands (RGB or intensity/chroma if YCbCr) */
    
    /* The following fields refer to the format of the data in MATLAB/Simulink */
    VideoDataType dataType;   /* data type of incoming video image pixels */
    VideoFrameOrientation orientation;  /* row- or column-major */
    
    /* The name of the compression format, if we are writing a file */
    const char* videoCompressor;        /* set to NULL for none */
    unsigned char useMMReader; /* useMMReader = (ispc && isVideoCompressed) */
} MMVideoInfo;


#endif  /* VIDEODEFS_H */


