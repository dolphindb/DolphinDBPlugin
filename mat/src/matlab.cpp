#include <Exceptions.h>
#include <ScalarImp.h>
#include <Util.h>
#include "matlab.h"
#include <math.h>
#include <mat.h>
#include <string>
#include <map>

std::string wstringToString(const std::wstring &wstrInput, unsigned int uCodePage)
{
#ifdef WINDOWS
    std::string strAnsi = "";
    if (wstrInput.length() == 0)
    {
        return strAnsi;
    }
    int iLength = ::WideCharToMultiByte(uCodePage, 0, wstrInput.c_str(), -1, NULL, 0, NULL, NULL);
    char *szDest = new char[iLength + 1];
    memset((void *)szDest, 0, (iLength + 1) * sizeof(char));
    ::WideCharToMultiByte(uCodePage, 0, wstrInput.c_str(), -1, szDest, iLength, NULL, NULL);
    strAnsi = szDest;
    delete[] szDest;
    return strAnsi;

#else
    std::string strLocale = setlocale(LC_ALL, "");
    const wchar_t *pSrc = wstrInput.c_str();
    unsigned int iDestSize = wcstombs(NULL, pSrc, 0) + 1;
    char *szDest = new char[iDestSize];
    memset(szDest, 0, iDestSize);
    wcstombs(szDest, pSrc, iDestSize);
    std::string strResult = szDest;
    delete[] szDest;
    setlocale(LC_ALL, strLocale.c_str());
    return strResult;
#endif
}

int mIndex = 1024;

/*std::unordered_map<std::string, Mutex> mutexMap;
Mutex mapMutex;*/

Mutex mutexLock;

bool convertToDTBool(mxArray *var, ConstantSP &sp)
{
    char buffer[mIndex];
    mxClassID type = mxGetClassID(var);
    int rank = mxGetNumberOfDimensions(var);
    if (rank > 2)
        throw RuntimeException("Matrix above two dimensions is not supported");
    //const long unsigned int*dims=mxGetDimensions(var);
    int row, col = rank == 0 ? 0 : mxGetN(var);
    if (rank == 0)
        row = 0;
    else if (rank == 1)
        row = 1;
    else
    {
        row = mxGetM(var);
    }
    sp = Util::createMatrix(DT_BOOL, col, row, col);
    if (col == 0)
        return true;
    int index = 0, freq = row * col / mIndex;
    switch (type)
    {
    case mxLOGICAL_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getBoolBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j] != 0;
            }
            ((VectorSP)sp)->setBool(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getBoolBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j] != 0;
        }
        ((VectorSP)sp)->setBool(index, legacy, p);
        break;
    }
    case mxINT8_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getBoolBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == CHAR_MIN)
                    p[j] = CHAR_MIN;
                else
                    p[j] = tmp[index + j] != 0;
            }
            ((VectorSP)sp)->setBool(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getBoolBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == CHAR_MIN)
                p[j] = CHAR_MIN;
            else
                p[j] = tmp[index + j] != 0;
        }
        ((VectorSP)sp)->setBool(index, legacy, p);
        break;
    }
    case mxUINT8_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getBoolBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j] != 0;
            }
            ((VectorSP)sp)->setBool(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getBoolBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j] != 0;
        }
        ((VectorSP)sp)->setBool(index, legacy, p);
        break;
    }
    case mxINT16_CLASS:
    {
        short *tmp = (short *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getBoolBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == SHRT_MIN)
                    p[j] = CHAR_MIN;
                else
                    p[j] = tmp[index + j] != 0;
            }
            ((VectorSP)sp)->setBool(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getBoolBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == SHRT_MIN)
                p[j] = CHAR_MIN;
            else
                p[j] = tmp[index + j] != 0;
        }
        ((VectorSP)sp)->setBool(index, legacy, p);
        break;
    }
    case mxUINT16_CLASS:
    {
        short *tmp = (short *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getBoolBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j] != 0;
            }
            ((VectorSP)sp)->setBool(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getBoolBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j] != 0;
        }
        ((VectorSP)sp)->setBool(index, legacy, p);
        break;
    }
    case mxINT32_CLASS:
    {
        int *tmp = (int *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getBoolBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == INT_MIN)
                    p[j] = CHAR_MIN;
                else
                    p[j] = tmp[index + j] != 0;
            }
            ((VectorSP)sp)->setBool(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getBoolBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == INT_MIN)
                p[j] = CHAR_MIN;
            else
                p[j] = tmp[index + j] != 0;
        }
        ((VectorSP)sp)->setBool(index, legacy, p);
        break;
    }
    case mxUINT32_CLASS:
    {
        int *tmp = (int *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getBoolBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j] != 0;
            }
            ((VectorSP)sp)->setBool(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getBoolBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j] != 0;
        }
        ((VectorSP)sp)->setBool(index, legacy, p);
        break;
    }
    case mxINT64_CLASS:
    {
        long long *tmp = (long long *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getBoolBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == LONG_LONG_MIN)
                    p[j] = CHAR_MIN;
                else
                    p[j] = tmp[index + j] != 0;
            }
            ((VectorSP)sp)->setBool(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getBoolBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == LONG_LONG_MIN)
                p[j] = CHAR_MIN;
            else
                p[j] = tmp[index + j] != 0;
        }
        ((VectorSP)sp)->setBool(index, legacy, p);
        break;
    }
    case mxSINGLE_CLASS:
    {
        float *tmp = (float *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getBoolBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                float t = tmp[index + j];
                if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                    p[j] = CHAR_MIN;
                else
                    p[j] = t != 0;
            }
            ((VectorSP)sp)->setBool(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getBoolBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            float t = tmp[index + j];
            if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                p[j] = CHAR_MIN;
            else
                p[j] = t != 0;
        }
        ((VectorSP)sp)->setBool(index, legacy, p);
        break;
    }
    case mxDOUBLE_CLASS:
    {
        double *tmp = (double *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getBoolBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                double t = tmp[index + j];
                if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                    p[j] = CHAR_MIN;
                else
                    p[j] = t != 0;
            }
            ((VectorSP)sp)->setBool(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getBoolBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            double t = tmp[index + j];
            if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                p[j] = CHAR_MIN;
            else
                p[j] = t != 0;
        }
        ((VectorSP)sp)->setBool(index, legacy, p);
        break;
    }
    default:
        throw RuntimeException("Can't convert mat " + std::string(mxGetClassName(var)) + " to DolphinDB CHAR");
    }
    return true;
}

bool convertToDTChar(mxArray *var, ConstantSP &sp)
{
    char buffer[mIndex];
    mxClassID type = mxGetClassID(var);
    int rank = mxGetNumberOfDimensions(var);
    if (rank > 2)
        throw RuntimeException("Matrix above two dimensions is not supported");
    //const long unsigned int*dims=mxGetDimensions(var);
    int row, col = rank == 0 ? 0 : mxGetN(var);
    if (rank == 0)
        row = 0;
    else if (rank == 1)
        row = 1;
    else
    {
        row = mxGetM(var);
    }
    sp = Util::createMatrix(DT_CHAR, col, row, col);
    if (col == 0)
        return true;
    int index = 0, freq = row * col / mIndex;
    switch (type)
    {
    case mxLOGICAL_CLASS:
    {
        bool *tmp = (bool *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getCharBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setChar(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getCharBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setChar(index, legacy, p);
        break;
    }
    case mxINT8_CLASS:
    case mxUINT8_CLASS:
    {
        void *tmp = mxGetData(var);
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        ((VectorSP)sp)->setChar(index, row * col, (char *)tmp);
        break;
    }
    case mxINT16_CLASS:
    {
        short *tmp = (short *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getCharBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == SHRT_MIN)
                    p[j] = CHAR_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setChar(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getCharBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == SHRT_MIN)
                p[j] = CHAR_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setChar(index, legacy, p);
        break;
    }
    case mxUINT16_CLASS:
    {
        short *tmp = (short *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getCharBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setChar(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getCharBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setChar(index, legacy, p);
        break;
    }
    case mxINT32_CLASS:
    {
        int *tmp = (int *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getCharBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == INT_MIN)
                    p[j] = CHAR_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setChar(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getCharBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == INT_MIN)
                p[j] = CHAR_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setChar(index, legacy, p);
        break;
    }
    case mxUINT32_CLASS:
    {
        uint32_t *tmp = (uint32_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getCharBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setChar(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getCharBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setChar(index, legacy, p);
        break;
    }
    case mxINT64_CLASS:
    {
        long long *tmp = (long long *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getCharBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == LONG_LONG_MIN)
                    p[j] = CHAR_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setChar(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getCharBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == LONG_LONG_MIN)
                p[j] = CHAR_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setChar(index, legacy, p);
        break;
    }
    case mxSINGLE_CLASS:
    {
        float *tmp = (float *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getCharBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                float t = tmp[index + j];
                t = t > 0 ? t + 0.5 : t - 0.5;
                if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                    p[j] = CHAR_MIN;
                else
                    p[j] = t;
            }
            ((VectorSP)sp)->setChar(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getCharBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            float t = tmp[index + j];
            t = t > 0 ? t + 0.5 : t - 0.5;
            if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                p[j] = CHAR_MIN;
            else
                p[j] = t;
        }
        ((VectorSP)sp)->setChar(index, legacy, p);
        break;
    }
    case mxDOUBLE_CLASS:
    {
        double *tmp = (double *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            char *p = ((VectorSP)sp)->getCharBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                double t = tmp[index + j];
                t = t > 0 ? t + 0.5 : t - 0.5;
                if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                    p[j] = CHAR_MIN;
                else
                    p[j] = t;
            }
            ((VectorSP)sp)->setChar(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        char *p = ((VectorSP)sp)->getCharBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            double t = tmp[index + j];
            t = t > 0 ? t + 0.5 : t - 0.5;
            if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                p[j] = CHAR_MIN;
            else
                p[j] = t;
        }
        ((VectorSP)sp)->setChar(index, legacy, p);
        break;
    }
    default:
        throw RuntimeException("Can't convert mat " + std::string(mxGetClassName(var)) + " to DolphinDB CHAR");
    }
    return true;
}

bool convertToDTShort(mxArray *var, ConstantSP &sp)
{
    short buffer[mIndex];
    mxClassID type = mxGetClassID(var);
    int rank = mxGetNumberOfDimensions(var);
    if (rank > 2)
        throw RuntimeException("Matrix above two dimensions is not supported");
    //const long unsigned int*dims=mxGetDimensions(var);
    int row, col = rank == 0 ? 0 : mxGetN(var);
    if (rank == 0)
        row = 0;
    else if (rank == 1)
        row = 1;
    else
    {
        row = mxGetM(var);
    }
    sp = Util::createMatrix(DT_SHORT, col, row, col);
    if (col == 0)
        return true;
    int index = 0, freq = row * col / mIndex;
    switch (type)
    {
    case mxLOGICAL_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            short *p = ((VectorSP)sp)->getShortBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setShort(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        short *p = ((VectorSP)sp)->getShortBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setShort(index, legacy, p);
        break;
    }
    case mxINT8_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            short *p = ((VectorSP)sp)->getShortBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == CHAR_MIN)
                    p[j] = SHRT_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setShort(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        short *p = ((VectorSP)sp)->getShortBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == CHAR_MIN)
                p[j] = SHRT_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setShort(index, legacy, p);
        break;
    }
    case mxUINT8_CLASS:
    {
        uint8_t *tmp = (uint8_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            short *p = ((VectorSP)sp)->getShortBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setShort(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        short *p = ((VectorSP)sp)->getShortBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setShort(index, legacy, p);
        break;
    }
    case mxINT16_CLASS:
    case mxUINT16_CLASS:
    {
        void *tmp = mxGetData(var);
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        ((VectorSP)sp)->setShort(index, row * col, (short *)(tmp));
    }
    break;
    case mxINT32_CLASS:
    {
        int *tmp = (int *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            short *p = ((VectorSP)sp)->getShortBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == INT_MIN)
                    p[j] = SHRT_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setShort(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        short *p = ((VectorSP)sp)->getShortBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == INT_MIN)
                p[j] = SHRT_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setShort(index, legacy, p);
        break;
    }
    case mxUINT32_CLASS:
    {
        int32_t *tmp = (int32_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            short *p = ((VectorSP)sp)->getShortBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setShort(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        short *p = ((VectorSP)sp)->getShortBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setShort(index, legacy, p);
        break;
    }
    case mxINT64_CLASS:
    {
        long long *tmp = (long long *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            short *p = ((VectorSP)sp)->getShortBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == LONG_LONG_MIN)
                    p[j] = SHRT_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setShort(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        short *p = ((VectorSP)sp)->getShortBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == LONG_LONG_MIN)
                p[j] = SHRT_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setShort(index, legacy, p);
        break;
    }
    case mxSINGLE_CLASS:
    {
        float *tmp = (float *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            short *p = ((VectorSP)sp)->getShortBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                float t = tmp[index + j];
                t = t > 0 ? t + 0.5 : t - 0.5;
                if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                    p[j] = SHRT_MIN;
                else
                    p[j] = t;
            }
            ((VectorSP)sp)->setShort(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        short *p = ((VectorSP)sp)->getShortBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            float t = tmp[index + j];
            t = t > 0 ? t + 0.5 : t - 0.5;
            if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                p[j] = SHRT_MIN;
            else
                p[j] = t;
        }
        ((VectorSP)sp)->setShort(index, legacy, p);
        break;
    }
    case mxDOUBLE_CLASS:
    {
        double *tmp = (double *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            short *p = ((VectorSP)sp)->getShortBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                double t = tmp[index + j];
                t = t > 0 ? t + 0.5 : t - 0.5;
                if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                    p[j] = SHRT_MIN;
                else
                    p[j] = t;
            }
            ((VectorSP)sp)->setShort(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        short *p = ((VectorSP)sp)->getShortBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            double t = tmp[index + j];
            t = t > 0 ? t + 0.5 : t - 0.5;
            if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                p[j] = SHRT_MIN;
            else
                p[j] = t;
        }
        ((VectorSP)sp)->setShort(index, legacy, p);
        break;
    }
    default:
        throw RuntimeException("Can't convert mat " + std::string(mxGetClassName(var)) + " to DolphinDB SHORT");
    }
    return true;
}

bool convertToDTInt(mxArray *var, ConstantSP &sp)
{
    int buffer[mIndex];
    mxClassID type = mxGetClassID(var);
    int rank = mxGetNumberOfDimensions(var);
    if (rank > 2)
        throw RuntimeException("Matrix above two dimensions is not supported");
    //const long unsigned int*dims=mxGetDimensions(var);
    int row, col = rank == 0 ? 0 : mxGetN(var);
    if (rank == 0)
        row = 0;
    else if (rank == 1)
        row = 1;
    else
    {
        row = mxGetM(var);
    }
    sp = Util::createMatrix(DT_INT, col, row, col);
    if (col == 0)
        return true;
    int index = 0, freq = row * col / mIndex;
    switch (type)
    {
    case mxLOGICAL_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            int *p = ((VectorSP)sp)->getIntBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setInt(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        int *p = ((VectorSP)sp)->getIntBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setInt(index, legacy, p);
        break;
    }
    case mxINT8_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            int *p = ((VectorSP)sp)->getIntBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == CHAR_MIN)
                    p[j] = INT_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setInt(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        int *p = ((VectorSP)sp)->getIntBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == CHAR_MIN)
                p[j] = INT_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setInt(index, legacy, p);
        break;
    }
    case mxUINT8_CLASS:
    {
        uint8_t *tmp = (uint8_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            int *p = ((VectorSP)sp)->getIntBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setInt(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        int *p = ((VectorSP)sp)->getIntBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setInt(index, legacy, p);
        break;
    }
    case mxINT16_CLASS:
    {
        short *tmp = (short *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            int *p = ((VectorSP)sp)->getIntBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == SHRT_MIN)
                    p[j] = INT_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setInt(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        int *p = ((VectorSP)sp)->getIntBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == SHRT_MIN)
                p[j] = INT_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setInt(index, legacy, p);
        break;
    }
    case mxUINT16_CLASS:
    {
        uint16_t *tmp = (uint16_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            int *p = ((VectorSP)sp)->getIntBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setInt(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        int *p = ((VectorSP)sp)->getIntBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setInt(index, legacy, p);
        break;
    }
    case mxINT32_CLASS:
    case mxUINT32_CLASS:
    {
        void *tmp = mxGetData(var);
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        ((VectorSP)sp)->setInt(index, row * col, (int *)tmp);
        break;
    }
    case mxINT64_CLASS:
    {
        long long *tmp = (long long *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            int *p = ((VectorSP)sp)->getIntBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == LONG_LONG_MIN)
                    p[j] = INT_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setInt(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        int *p = ((VectorSP)sp)->getIntBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == LONG_LONG_MIN)
                p[j] = INT_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setInt(index, legacy, p);
        break;
    }
    case mxSINGLE_CLASS:
    {
        float *tmp = (float *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            int *p = ((VectorSP)sp)->getIntBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                float t = tmp[index + j];
                t = t > 0 ? t + 0.5 : t - 0.5;
                if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                    p[j] = INT_MIN;
                else
                    p[j] = t;
            }
            ((VectorSP)sp)->setInt(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        int *p = ((VectorSP)sp)->getIntBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            float t = tmp[index + j];
            t = t > 0 ? t + 0.5 : t - 0.5;
            if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                p[j] = INT_MIN;
            else
                p[j] = t;
        }
        ((VectorSP)sp)->setInt(index, legacy, p);
        break;
    }
    case mxDOUBLE_CLASS:
    {
        double *tmp = (double *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            int *p = ((VectorSP)sp)->getIntBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                double t = tmp[index + j];
                t = t > 0 ? t + 0.5 : t - 0.5;
                if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                    p[j] = INT_MIN;
                else
                    p[j] = t;
            }
            ((VectorSP)sp)->setInt(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        int *p = ((VectorSP)sp)->getIntBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            double t = tmp[index + j];
            t = t > 0 ? t + 0.5 : t - 0.5;
            if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                p[j] = INT_MIN;
            else
                p[j] = t;
        }
        ((VectorSP)sp)->setInt(index, legacy, p);
        break;
    }
    default:
        throw RuntimeException("Can't convert mat " + std::string(mxGetClassName(var)) + " to DolphinDB INT");
    }
    return true;
}

bool convertToDTLong(mxArray *var, ConstantSP &sp)
{
    long long buffer[mIndex];
    mxClassID type = mxGetClassID(var);
    int rank = mxGetNumberOfDimensions(var);
    if (rank > 2)
        throw RuntimeException("Matrix above two dimensions is not supported");
    //const long unsigned int*dims=mxGetDimensions(var);
    int row, col = rank == 0 ? 0 : mxGetN(var);
    if (rank == 0)
        row = 0;
    else if (rank == 1)
        row = 1;
    else
    {
        row = mxGetM(var);
    }
    sp = Util::createMatrix(DT_LONG, col, row, col);
    if (col == 0)
        return true;
    int index = 0, freq = row * col / mIndex;
    switch (type)
    {
    case mxLOGICAL_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            long long *p = ((VectorSP)sp)->getLongBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setLong(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        long long *p = ((VectorSP)sp)->getLongBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setLong(index, legacy, p);
        break;
    }
    case mxINT8_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            long long *p = ((VectorSP)sp)->getLongBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == CHAR_MIN)
                    p[j] = LONG_LONG_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setLong(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        long long *p = ((VectorSP)sp)->getLongBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == CHAR_MIN)
                p[j] = LONG_LONG_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setLong(index, legacy, p);
        break;
    }
    case mxUINT8_CLASS:
    {
        uint8_t *tmp = (uint8_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            long long *p = ((VectorSP)sp)->getLongBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setLong(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        long long *p = ((VectorSP)sp)->getLongBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setLong(index, legacy, p);
        break;
    }
    case mxINT16_CLASS:
    {
        short *tmp = (short *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            long long *p = ((VectorSP)sp)->getLongBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == SHRT_MIN)
                    p[j] = LONG_LONG_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setLong(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        long long *p = ((VectorSP)sp)->getLongBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == SHRT_MIN)
                p[j] = LONG_LONG_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setLong(index, legacy, p);
        break;
    }
    case mxUINT16_CLASS:
    {
        uint16_t *tmp = (uint16_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            long long *p = ((VectorSP)sp)->getLongBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setLong(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        long long *p = ((VectorSP)sp)->getLongBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setLong(index, legacy, p);
        break;
    }
    case mxINT32_CLASS:
    {
        int *tmp = (int *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            long long *p = ((VectorSP)sp)->getLongBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == INT_MIN)
                    p[j] = LONG_LONG_MIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setLong(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        long long *p = ((VectorSP)sp)->getLongBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == INT_MIN)
                p[j] = LONG_LONG_MIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setLong(index, legacy, p);
        break;
    }
    case mxUINT32_CLASS:
    {
        uint32_t *tmp = (uint32_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            long long *p = ((VectorSP)sp)->getLongBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setLong(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        long long *p = ((VectorSP)sp)->getLongBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setLong(index, legacy, p);
        break;
    }
    case mxINT64_CLASS:
    {
        void *tmp = mxGetData(var);
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        ((VectorSP)sp)->setLong(index, row * col, (long long *)tmp);
        break;
    }
    case mxSINGLE_CLASS:
    {
        float *tmp = (float *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            long long *p = ((VectorSP)sp)->getLongBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                float t = tmp[index + j];
                t = t > 0 ? t + 0.5 : t - 0.5;
                if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                    p[j] = LONG_LONG_MIN;
                else
                    p[j] = t;
            }
            ((VectorSP)sp)->setLong(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        long long *p = ((VectorSP)sp)->getLongBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            float t = tmp[index + j];
            t = t > 0 ? t + 0.5 : t - 0.5;
            if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                p[j] = LONG_LONG_MIN;
            else
                p[j] = t;
        }
        ((VectorSP)sp)->setLong(index, legacy, p);
        break;
    }
    case mxDOUBLE_CLASS:
    {
        double *tmp = (double *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            long long *p = ((VectorSP)sp)->getLongBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                double t = tmp[index + j];
                t = t > 0 ? t + 0.5 : t - 0.5;
                if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                    p[j] = LONG_LONG_MIN;
                else
                    p[j] = t;
            }
            ((VectorSP)sp)->setLong(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        long long *p = ((VectorSP)sp)->getLongBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            double t = tmp[index + j];
            t = t > 0 ? t + 0.5 : t - 0.5;
            if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                p[j] = LONG_LONG_MIN;
            else
                p[j] = t;
        }
        ((VectorSP)sp)->setLong(index, legacy, p);
        break;
    }
    default:
        throw RuntimeException("Can't convert mat " + std::string(mxGetClassName(var)) + " to DolphinDB LONG");
    }
    return true;
}

bool convertToDTFloat(mxArray *var, ConstantSP &sp)
{
    float buffer[mIndex];
    mxClassID type = mxGetClassID(var);
    int rank = mxGetNumberOfDimensions(var);
    if (rank > 2)
        throw RuntimeException("Matrix above two dimensions is not supported");
    //const long unsigned int*dims=mxGetDimensions(var);
    int row, col = rank == 0 ? 0 : mxGetN(var);
    if (rank == 0)
        row = 0;
    else if (rank == 1)
        row = 1;
    else
    {
        row = mxGetM(var);
    }
    sp = Util::createMatrix(DT_FLOAT, col, row, col);
    if (col == 0)
        return true;
    int index = 0, freq = row * col / mIndex;
    switch (type)
    {
    case mxLOGICAL_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            float *p = ((VectorSP)sp)->getFloatBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setFloat(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        float *p = ((VectorSP)sp)->getFloatBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setFloat(index, legacy, p);
        break;
    }
    case mxINT8_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            float *p = ((VectorSP)sp)->getFloatBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == CHAR_MIN)
                    p[j] = FLT_NMIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setFloat(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        float *p = ((VectorSP)sp)->getFloatBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == CHAR_MIN)
                p[j] = FLT_NMIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setFloat(index, legacy, p);
        break;
    }
    case mxUINT8_CLASS:
    {
        uint8_t *tmp = (uint8_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            float *p = ((VectorSP)sp)->getFloatBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setFloat(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        float *p = ((VectorSP)sp)->getFloatBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setFloat(index, legacy, p);
        break;
    }
    case mxINT16_CLASS:
    {
        short *tmp = (short *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            float *p = ((VectorSP)sp)->getFloatBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == SHRT_MIN)
                    p[j] = FLT_NMIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setFloat(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        float *p = ((VectorSP)sp)->getFloatBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == SHRT_MIN)
                p[j] = FLT_NMIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setFloat(index, legacy, p);
        break;
    }
    case mxUINT16_CLASS:
    {
        uint16_t *tmp = (uint16_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            float *p = ((VectorSP)sp)->getFloatBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setFloat(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        float *p = ((VectorSP)sp)->getFloatBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setFloat(index, legacy, p);
        break;
    }
    case mxINT32_CLASS:
    {
        int *tmp = (int *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            float *p = ((VectorSP)sp)->getFloatBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == INT_MIN)
                    p[j] = FLT_NMIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setFloat(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        float *p = ((VectorSP)sp)->getFloatBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == INT_MIN)
                p[j] = FLT_NMIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setFloat(index, legacy, p);
        break;
    }
    case mxUINT32_CLASS:
    {
        uint32_t *tmp = (uint32_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            float *p = ((VectorSP)sp)->getFloatBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setFloat(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        float *p = ((VectorSP)sp)->getFloatBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setFloat(index, legacy, p);
        break;
    }
    case mxINT64_CLASS:
    {
        long long *tmp = (long long *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            float *p = ((VectorSP)sp)->getFloatBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == LONG_LONG_MIN)
                    p[j] = FLT_NMIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setFloat(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        float *p = ((VectorSP)sp)->getFloatBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == LONG_LONG_MIN)
                p[j] = FLT_NMIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setFloat(index, legacy, p);
        break;
    }
    case mxSINGLE_CLASS:
    {
        float *tmp = (float *)(mxGetData(var));
        for (int i = 0; i < col; ++i)
        {
            for (int j = 0; j < row; ++j)
            {
                float t = tmp[i * row + j];
                if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                    tmp[i * row + j] = FLT_NMIN;
                else
                    tmp[i * row + j] = t;
            }
        }
        ((VectorSP)sp)->setFloat(0, row * col, tmp);
        break;
    }
    case mxDOUBLE_CLASS:
    {
        double *tmp = (double *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            float *p = ((VectorSP)sp)->getFloatBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                double t = tmp[index + j];
                if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                    p[j] = FLT_NMIN;
                else
                    p[j] = t;
            }
            ((VectorSP)sp)->setFloat(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        float *p = ((VectorSP)sp)->getFloatBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            double t = tmp[index + j];
            if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                p[j] = FLT_NMIN;
            else
                p[j] = t;
        }
        ((VectorSP)sp)->setFloat(index, legacy, p);
        break;
    }
    default:
        throw RuntimeException("Can't convert mat " + std::string(mxGetClassName(var)) + " to DolphinDB FLOAT");
    }
    return true;
}

bool convertToDTDouble(mxArray *var, ConstantSP &sp)
{
    double buffer[mIndex];
    mxClassID type = mxGetClassID(var);
    int rank = mxGetNumberOfDimensions(var);
    if (rank > 2)
        throw RuntimeException("Matrix above two dimensions is not supported");
    //const long unsigned int*dims=mxGetDimensions(var);
    int row, col = rank == 0 ? 0 : mxGetN(var);
    if (rank == 0)
        row = 0;
    else if (rank == 1)
        row = 1;
    else
    {
        row = mxGetM(var);
    }
    sp = Util::createMatrix(DT_DOUBLE, col, row, col);
    if (col == 0)
        return true;
    int index = 0, freq = row * col / mIndex;
    switch (type)
    {
    case mxLOGICAL_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            double *p = ((VectorSP)sp)->getDoubleBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setDouble(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        double *p = ((VectorSP)sp)->getDoubleBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setDouble(index, legacy, p);
        break;
    }
    case mxINT8_CLASS:
    {
        char *tmp = (char *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            double *p = ((VectorSP)sp)->getDoubleBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == CHAR_MIN)
                    p[j] = DBL_NMIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setDouble(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        double *p = ((VectorSP)sp)->getDoubleBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == CHAR_MIN)
                p[j] = DBL_NMIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setDouble(index, legacy, p);
        break;
    }
    case mxUINT8_CLASS:
    {
        uint8_t *tmp = (uint8_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            double *p = ((VectorSP)sp)->getDoubleBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setDouble(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        double *p = ((VectorSP)sp)->getDoubleBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setDouble(index, legacy, p);
        break;
    }
    case mxINT16_CLASS:
    {
        short *tmp = (short *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            double *p = ((VectorSP)sp)->getDoubleBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == SHRT_MIN)
                    p[j] = DBL_NMIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setDouble(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        double *p = ((VectorSP)sp)->getDoubleBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == SHRT_MIN)
                p[j] = DBL_NMIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setDouble(index, legacy, p);
        break;
    }
    case mxUINT16_CLASS:
    {
        uint16_t *tmp = (uint16_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            double *p = ((VectorSP)sp)->getDoubleBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setDouble(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        double *p = ((VectorSP)sp)->getDoubleBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setDouble(index, legacy, p);
        break;
    }
    case mxINT32_CLASS:
    {
        int *tmp = (int *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            double *p = ((VectorSP)sp)->getDoubleBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == INT_MIN)
                    p[j] = DBL_NMIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setDouble(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        double *p = ((VectorSP)sp)->getDoubleBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == INT_MIN)
                p[j] = DBL_NMIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setDouble(index, legacy, p);
        break;
    }
    case mxUINT32_CLASS:
    {
        uint32_t *tmp = (uint32_t *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            double *p = ((VectorSP)sp)->getDoubleBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setDouble(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        double *p = ((VectorSP)sp)->getDoubleBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setDouble(index, legacy, p);
        break;
    }
    case mxINT64_CLASS:
    {
        long long *tmp = (long long *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            double *p = ((VectorSP)sp)->getDoubleBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                if (tmp[index + j] == LONG_LONG_MIN)
                    p[j] = DBL_NMIN;
                else
                    p[j] = tmp[index + j];
            }
            ((VectorSP)sp)->setDouble(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        double *p = ((VectorSP)sp)->getDoubleBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            if (tmp[index + j] == LONG_LONG_MIN)
                p[j] = DBL_NMIN;
            else
                p[j] = tmp[index + j];
        }
        ((VectorSP)sp)->setDouble(index, legacy, p);
        break;
    }
    case mxSINGLE_CLASS:
    {
        float *tmp = (float *)(mxGetData(var));
        if (tmp == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < freq; ++i)
        {
            double *p = ((VectorSP)sp)->getDoubleBuffer(index, mIndex, buffer);
            for (int j = 0; j < mIndex; ++j)
            {
                float t = tmp[index + j];
                if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                    p[j] = DBL_NMIN;
                else
                    p[j] = t;
            }
            ((VectorSP)sp)->setDouble(index, mIndex, p);
            index += mIndex;
        }
        int legacy = row * col - index;
        double *p = ((VectorSP)sp)->getDoubleBuffer(index, legacy, buffer);
        for (int j = 0; j < legacy; ++j)
        {
            float t = tmp[index + j];
            if (std::isnan(t) || std::isinf(t) || t == FLT_NMIN)
                p[j] = DBL_NMIN;
            else
                p[j] = t;
        }
        ((VectorSP)sp)->setDouble(index, legacy, p);
        break;
    }
    case mxDOUBLE_CLASS:
    {
        double *tmp = (double *)(mxGetData(var));
        for (int i = 0; i < col; ++i)
        {
            for (int j = 0; j < row; ++j)
            {
                double t = tmp[i * row + j];
                if (std::isnan(t) || std::isinf(t) || t == DBL_NMIN)
                    tmp[i * row + j] = DBL_NMIN;
                else
                    tmp[i * row + j] = t;
            }
        }
        ((VectorSP)sp)->setDouble(0, row * col, tmp);
        break;
    }
    default:
        throw RuntimeException("Can't convert mat " + std::string(mxGetClassName(var)) + " to DolphinDB DOUBLE");
    }
    return true;
}

bool convertToDTString(mxArray *var, ConstantSP &sp)
{
    char buffer[2];
    mxClassID type = mxGetClassID(var);
    int rank = mxGetNumberOfDimensions(var);
    if (rank > 2)
        throw RuntimeException("Matrix above two dimensions is not supported");
    //const long unsigned int*dims=mxGetDimensions(var);
    int row, col = rank == 0 ? 0 : mxGetN(var);
    if (rank == 0)
        row = 0;
    else if (rank == 1)
        row = 1;
    else
    {
        row = mxGetM(var);
    }
    int index = 0, freq = row * col / mIndex;
    sp = Util::createVector(DT_STRING, 0, 0);
    if (col == 0)
        return true;
    int start[2] = {0, 0}, stride[2] = {1, 1}, edge[2] = {1, col};
    switch (type)
    {
    case mxCHAR_CLASS:
    {
        wchar_t tmp[col];
        short *data = (short *)mxGetData(var);
        if (data == NULL)
            throw RuntimeException("Out of memory");
        for (int i = 0; i < row; ++i)
        {
            for (int j = 0; j < col; ++j)
            {
                tmp[j + i * col] = data[j * row + i];
            }
            std::string strTmp = wstringToString(std::wstring(tmp + i * col, col), 0);
            ((VectorSP)sp)->appendString(&strTmp, 1);
        }
        break;
    }
    default:
        throw RuntimeException("Can't convert mat " + std::string(mxGetClassName(var)) + " to DolphinDB STRING");
    }
    return true;
}

DictionarySP load(string file, ConstantSP &ConstantSchema)
{
    DictionarySP ret = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
    bool schemaed = false;
    unordered_map<std::string, DATA_TYPE> typeMap;
    if (!ConstantSchema.isNull() && !ConstantSchema->isNull())
    {
        if (!ConstantSchema->isTable())
            throw IllegalArgumentException(__FUNCTION__, "schema must be a table containing column names and types.");
        schemaed = true;
        TableSP schema = (TableSP)ConstantSchema;
        VectorSP vecName = schema->getColumn("name");
        if (vecName == nullptr)
        {
            throw IllegalArgumentException(__FUNCTION__, "There is no column \"name\" in schema table");
        }
        if (vecName->getType() != DT_STRING)
        {
            throw IllegalArgumentException(__FUNCTION__, "The schema table column \"name\" type must be STRING");
        }
        VectorSP vecType = schema->getColumn("type");
        if (vecType == nullptr)
        {
            throw IllegalArgumentException(__FUNCTION__, "There is no column \"type\" in schema table");
        }
        if (vecType->getType() != DT_STRING)
        {
            throw IllegalArgumentException(__FUNCTION__, "The schema table column \"type\" type must be STRING");
        }
        if (vecName->size() != vecType->size())
        {
            throw IllegalArgumentException(__FUNCTION__, "The schema table column \"name\" and \"type\" size are not equal");
        }
        int len = vecName->size();
        for (int i = 0; i < len; ++i)
        {
            std::string nameTmp = vecName->getString(i);
            string sType = vecType->getString(i);
            std::transform(sType.begin(), sType.end(), sType.begin(), ::toupper);
            if (sType == "BOOL")
            {
                typeMap[nameTmp] = DT_BOOL;
            }
            else if (sType == "CHAR")
            {
                typeMap[nameTmp] = DT_CHAR;
            }
            else if (sType == "SHORT")
            {
                typeMap[nameTmp] = DT_SHORT;
            }
            else if (sType == "INT")
            {
                typeMap[nameTmp] = DT_INT;
            }
            else if (sType == "LONG")
            {
                typeMap[nameTmp] = DT_LONG;
            }
            else if (sType == "FLOAT")
            {
                typeMap[nameTmp] = DT_FLOAT;
            }
            else if (sType == "DOUBLE")
            {
                typeMap[nameTmp] = DT_DOUBLE;
            }
            else if (sType == "SYMBOL")
            {
                typeMap[nameTmp] = DT_SYMBOL;
            }
            else if (sType == "STRING")
            {
                typeMap[nameTmp] = DT_STRING;
            }
            else
            {
                throw IllegalArgumentException(__FUNCTION__, "The Type " + vecType->getString(i) + " is not supported");
            }
        }
    }
    MATFile *mat = matOpen(file.c_str(), "r");
    mxArray *var;
    if (mat == NULL)
        throw RuntimeException("File does not exist");
    const char *name;
    while (var = matGetNextVariable(mat, &name))
    {
        mxClassID type = mxGetClassID(var);
        DATA_TYPE dstType;
        if (schemaed)
        {
            if (typeMap.count(name) == 0)
            {
                continue;
            }
            else
                dstType = typeMap[name];
        }
        else
        {
            switch (type)
            {
            case mxLOGICAL_CLASS:
                dstType = DT_BOOL;
                break;
            case mxINT8_CLASS:
                dstType = DT_CHAR;
                break;
            case mxUINT8_CLASS:
            case mxINT16_CLASS:
                dstType = DT_SHORT;
                break;
            case mxUINT16_CLASS:
            case mxINT32_CLASS:
                dstType = DT_INT;
                break;
            case mxUINT32_CLASS:
            case mxINT64_CLASS:
                dstType = DT_LONG;
                break;
            case mxSINGLE_CLASS:
                dstType = DT_FLOAT;
                break;
            case mxDOUBLE_CLASS:
                dstType = DT_DOUBLE;
                break;
            case mxCHAR_CLASS:
                dstType = DT_STRING;
                break;
            default:
            {
                std::string tmp = mxGetClassName(var);
                mxDestroyArray(var);
                matClose(mat);
                throw IllegalArgumentException(__FUNCTION__, "The Type " + tmp + " is not supported");
            }
            }
        }

        ConstantSP sp;
        try
        {
            switch (dstType)
            {
            case DT_BOOL:
                convertToDTBool(var, sp);
                break;
            case DT_CHAR:
                convertToDTChar(var, sp);
                break;
            case DT_SHORT:
                convertToDTShort(var, sp);
                break;
            case DT_INT:
                convertToDTInt(var, sp);
                break;
            case DT_LONG:
                convertToDTLong(var, sp);
                break;
            case DT_FLOAT:
                convertToDTFloat(var, sp);
                break;
            case DT_DOUBLE:
                convertToDTDouble(var, sp);
                break;
            case DT_STRING:
                convertToDTString(var, sp);
                break;
            default:
                throw RuntimeException("unsupport DolphinDB type");
            }
        }
        catch (RuntimeException &e)
        {
            mxDestroyArray(var);
            matClose(mat);
            throw e;
        }
        string tmp(name);
        ret->set(tmp, sp);
        mxDestroyArray(var);
    }
    matClose(mat);
    return ret;
}

ConstantSP extractMatSchema(Heap *heap, vector<ConstantSP> &args)
{
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "File must be a string scalar");

    auto file = args[0]->getString();

    /*mapMutex.lock();
    auto mutexLock = mutexMap[file];
    mapMutex.unlock();*/

    LockGuard<Mutex> lk(&mutexLock);

    MATFile *pMF = matOpen(args[0]->getString().c_str(), "r");
    if (pMF == NULL)
        throw RuntimeException("File does not exist");
    const char *varName;
    mxArray *pMxArray;
    std::vector<ConstantSP> cols(2);
    std::vector<std::string> name = {"name", "type"};
    cols[0] = Util::createVector(DT_STRING, 0);
    cols[1] = Util::createVector(DT_STRING, 0);
    std::vector<std::string> dataName;
    std::vector<std::string> dataType;
    while (pMxArray = matGetNextVariable(pMF, &varName))
    {
        mxClassID type = mxGetClassID(pMxArray);
        switch (type)
        {
        case mxLOGICAL_CLASS:
            dataType.push_back("BOOL");
            break;
        case mxCHAR_CLASS:
            dataType.push_back("STRING");
            break;
        case mxINT8_CLASS:
            dataType.push_back("CHAR");
            break;
        case mxUINT8_CLASS:
        case mxINT16_CLASS:
            dataType.push_back("SHORT");
            break;
        case mxUINT16_CLASS:
        case mxINT32_CLASS:
            dataType.push_back("INT");
            break;
        case mxUINT32_CLASS:
        case mxINT64_CLASS:
            dataType.push_back("LONG");
            break;
        case mxSINGLE_CLASS:
            dataType.push_back("FLOAT");
            break;
        case mxDOUBLE_CLASS:
            dataType.push_back("DOUBLE");
            break;
        default:
        {
            std::string tmp = std::string(mxGetClassName(pMxArray));
            mxDestroyArray(pMxArray);
            matClose(pMF);
            throw IllegalArgumentException(__FUNCTION__, "The Type " + tmp + " is not supported");
        }
        }
        dataName.push_back(varName);
        mxDestroyArray(pMxArray);
    }
    matClose(pMF);
    ((VectorSP)(cols[0]))->appendString(dataName.data(), dataName.size());
    ((VectorSP)(cols[1]))->appendString(dataType.data(), dataType.size());
    TableSP ret = Util::createTable(name, cols);
    return ret;
}

ConstantSP loadMat(Heap *heap, vector<ConstantSP> &args)
{
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "File must be a string scalar");
    std::vector<std::string> dataName;
    DictionarySP ret;

    auto file = args[0]->getString();

    /*mapMutex.lock();
    auto mutexLock = mutexMap[file];
    mapMutex.unlock();*/

    LockGuard<Mutex> lk(&mutexLock);

    if (args.size() == 2)
    {
        ret = load(args[0]->getString(), args[1]);
    }
    else
    {
        ConstantSP tmp;
        ret = load(args[0]->getString(), tmp);
    }
    return ret;
}

long long ConvertToDTdatetime(double data)
{
    long double tmp = data;
    tmp -= 719529;
    tmp *= 24 * 60 * 60;
    long long ret = floor(tmp + 0.001);
    if (ret < 0)
        throw IllegalArgumentException(__FUNCTION__, "Data must be a double In MATLAB time after January 1, 1970");
    return ret;
}

ConstantSP convertToDatetime(Heap *heap, vector<ConstantSP> &args)
{
    if (args[0]->getType() != DT_DOUBLE ||
        (args[0]->getForm() != DF_SCALAR && args[0]->getForm() != DF_VECTOR && args[0]->getForm() != DF_MATRIX))
        throw IllegalArgumentException(__FUNCTION__, "Data must be a double scalar, vector or matrix");
    if (args[0]->getForm() == DF_SCALAR)
        return new DateTime(ConvertToDTdatetime(args[0]->getDouble()));
    else if (args[0]->getForm() == DF_VECTOR)
    {
        int size = args[0]->size();
        long long lBuffer[size];
        double dBuffer[size];
        double *p = ((VectorSP)args[0])->getDoubleBuffer(0, size, dBuffer);
        for (int i = 0; i < size; ++i)
        {
            lBuffer[i] = ConvertToDTdatetime(dBuffer[i]);
        }
        VectorSP ret = Util::createVector(DT_DATETIME, 0, 0);
        ret->appendLong(lBuffer, size);
        return ret;
    }
    else
    {
        VectorSP data = args[0];
        int row = data->rows(), col = data->columns();
        ConstantSP ret = Util::createMatrix(DT_DATETIME, col, row, col);
        double buffer[mIndex];
        long long lBuffer[mIndex];
        int index = 0;

        while (index + mIndex <= row * col)
        {
            double *p = data->getDoubleBuffer(index, mIndex, buffer);
            long long *q = ret->getLongBuffer(index, mIndex, lBuffer);
            for (int i = 0; i < mIndex; ++i)
            {
                q[i] = ConvertToDTdatetime(p[i]);
            }
            ((VectorSP)ret)->setLong(index, mIndex, q);
            index += mIndex;
        }
        int leacy = row * col - index;
        double *p = data->getDoubleBuffer(index, leacy, buffer);
        long long *q = ret->getLongBuffer(index, leacy, lBuffer);
        for (int i = 0; i < leacy; ++i)
        {
            q[i] = ConvertToDTdatetime(p[i]);
        }
        ((VectorSP)ret)->setLong(index, leacy, q);
        return ret;
    }
}

ConstantSP writeMat(Heap *heap, vector<ConstantSP> &args){
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "File must be a string scalar");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "VarName must be a string scalar");
    if (args[1]->isNull())
        throw IllegalArgumentException(__FUNCTION__, "VarName is empty");
    if (!std::isalpha(args[1]->getString()[0]))
        throw IllegalArgumentException(__FUNCTION__, "VarName in MATLAB (and thus also in MAT-files) must start with a letter");
    if(args[2]->getForm() != DF_MATRIX){
        throw IllegalArgumentException(__FUNCTION__, "Data must be a matrix");
    }
    string file = args[0]->getString();
    string varName = args[1]->getString();
    VectorSP data = args[2];

    /*mapMutex.lock();
    auto mutexLock = mutexMap[file];
    mapMutex.unlock();*/

    LockGuard<Mutex> lk(&mutexLock);

    MATFile *pMF = matOpen(file.c_str(), "w");
    if (pMF == NULL)
        throw RuntimeException("Can't write to " + file);
    int columns = data->columns();
    int rows = data->rows();
    switch (data->getType()) {
        case DT_FLOAT:{
            mxArray * tmp = mxCreateNumericMatrix(rows, columns, mxSINGLE_CLASS, mxREAL);
            if(tmp == nullptr){
                matClose(pMF);
                throw RuntimeException("Can't write to " + file + ", Because can't create data");
            }
            vector<float> dstBuffer(columns * rows);
            dstBuffer.resize(columns * rows);
            vector<float> buffer(rows);
            for(int i = 0; i < columns; ++i){
                const float *ptr = data->getFloatConst(i * rows, rows, buffer.data());
                for(int j = 0; j < rows; ++j){
                    dstBuffer[i * rows + j] = ptr[j];
                }
            }
            memcpy((void *)(mxGetPr(tmp)), (void *)dstBuffer.data(),sizeof (float ) * columns * rows);
            matPutVariable(pMF, varName.c_str(), tmp);
            mxDestroyArray(tmp);
            break;
        }
        case DT_DOUBLE:{
            mxArray * tmp = mxCreateNumericMatrix(rows, columns, mxDOUBLE_CLASS, mxREAL);
            if(tmp == nullptr){
                matClose(pMF);
                throw RuntimeException("Can't write to " + file + ", Because can't create data");
            }
            vector<double> dstBuffer(columns * rows);
            dstBuffer.resize(columns * rows);
            vector<double> buffer(rows);
            for(int i = 0; i < columns; ++i){
                const double *ptr = data->getDoubleConst(i * rows, rows, buffer.data());
                for(int j = 0; j < rows; ++j){
                    dstBuffer[i * rows + j] = ptr[j];
                }
            }
            memcpy((void *)(mxGetPr(tmp)), (void *)dstBuffer.data(),sizeof (double) * columns * rows);
            matPutVariable(pMF, varName.c_str(), tmp);
            mxDestroyArray(tmp);
            break;
        }
        case DT_BOOL:{
            mxArray * tmp = mxCreateNumericMatrix(rows, columns, mxLOGICAL_CLASS, mxREAL);
            if(tmp == nullptr){
                matClose(pMF);
                throw RuntimeException("Can't write to " + file + ", Because can't create data");
            }
            vector<char> dstBuffer(columns * rows);
            dstBuffer.resize(columns * rows);
            vector<char> buffer(rows);
            for(int i = 0; i < columns; ++i){
                const char *ptr = data->getBoolConst(i * rows, rows, buffer.data());
                for(int j = 0; j < rows; ++j){
                    dstBuffer[i * rows + j] = ptr[j];
                }
            }
            memcpy((void *)(mxGetPr(tmp)), (void *)dstBuffer.data(),sizeof (char) * columns * rows);
            matPutVariable(pMF, varName.c_str(), tmp);
            mxDestroyArray(tmp);
            break;
        }
        case DT_INT:{
            mxArray * tmp = mxCreateNumericMatrix(rows, columns, mxINT32_CLASS, mxREAL);
            if(tmp == nullptr){
                matClose(pMF);
                throw RuntimeException("Can't write to " + file + ", Because can't create data");
            }
            vector<int> dstBuffer(columns * rows);
            dstBuffer.resize(columns * rows);
            vector<int> buffer(rows);
            for(int i = 0; i < columns; ++i){
                const int *ptr = data->getIntConst(i * rows, rows, buffer.data());
                for(int j = 0; j < rows; ++j){
                    dstBuffer[i * rows + j] = ptr[j];
                }
            }
            memcpy((void *)(mxGetPr(tmp)), (void *)dstBuffer.data(),sizeof (int) * columns * rows);
            matPutVariable(pMF, varName.c_str(), tmp);
            mxDestroyArray(tmp);
            break;
        }
        case DT_LONG:{
            mxArray * tmp = mxCreateNumericMatrix(rows, columns, mxINT64_CLASS, mxREAL);
            if(tmp == nullptr){
                matClose(pMF);
                throw RuntimeException("Can't write to " + file + ", Because can't create data");
            }
            vector<long long> dstBuffer(columns * rows);
            dstBuffer.resize(columns * rows);
            vector<long long> buffer(rows);
            for(int i = 0; i < columns; ++i){
                const long long *ptr = data->getLongConst(i * rows, rows, buffer.data());
                for(int j = 0; j < rows; ++j){
                    dstBuffer[i * rows + j] = ptr[j];
                }
            }
            memcpy((void *)(mxGetPr(tmp)), (void *)dstBuffer.data(),sizeof (long long) * columns * rows);
            matPutVariable(pMF, varName.c_str(), tmp);
            mxDestroyArray(tmp);
            break;
        }
        case DT_CHAR:{
            mxArray * tmp = mxCreateNumericMatrix(rows, columns, mxINT8_CLASS, mxREAL);
            if(tmp == nullptr){
                matClose(pMF);
                throw RuntimeException("Can't write to " + file + ", Because can't create data");
            }
            vector<char> dstBuffer(columns * rows);
            dstBuffer.resize(columns * rows);
            vector<char> buffer(rows);
            for(int i = 0; i < columns; ++i){
                const char *ptr = data->getCharConst(i * rows, rows, buffer.data());
                for(int j = 0; j < rows; ++j){
                    dstBuffer[i * rows + j] = ptr[j];
                }
            }
            memcpy((void *)(mxGetPr(tmp)), (void *)dstBuffer.data(),sizeof (char) * columns * rows);
            matPutVariable(pMF, varName.c_str(), tmp);
            mxDestroyArray(tmp);
            break;
        }
        case DT_SHORT:{
            mxArray * tmp = mxCreateNumericMatrix(rows, columns, mxINT16_CLASS, mxREAL);
            if(tmp == nullptr){
                matClose(pMF);
                throw RuntimeException("Can't write to " + file + ", Because can't create data");
            }
            vector<short> dstBuffer(columns * rows);
            dstBuffer.resize(columns * rows);
            vector<short> buffer(rows);
            for(int i = 0; i < columns; ++i){
                const short *ptr = data->getShortConst(i * rows, rows, buffer.data());
                for(int j = 0; j < rows; ++j){
                    dstBuffer[i * rows + j] = ptr[j];
                }
            }
            memcpy((void *)(mxGetPr(tmp)), (void *)dstBuffer.data(),sizeof (short ) * columns * rows);
            matPutVariable(pMF, varName.c_str(), tmp);
            mxDestroyArray(tmp);
            break;
        }
        default:{
            throw RuntimeException("The DolphinDB data type " + std::to_string(data->getType()) + " is not supported");
        }
    }
    matClose(pMF);
    return new Bool(true);
}