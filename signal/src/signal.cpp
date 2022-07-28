/*
 * signal.cpp
 *
 * Created on: Dec 1.2020
 *     Author: zkluo
 *
 */

#include "signal.h"
#include "math.h"
#include "Util.h"
#include "fftw3.h"
#include <vector>
#include <string>
#include <omp.h>
#include <ScalarImp.h>

#define PI 3.1415926
bool fftwInit = false;
static void dwt_get(int, int, vector<double>&, vector<double>&, vector<double>&);
static void idwt_get(int, int, vector<double> &, vector<double> &, vector<double> &, omp_lock_t &);
static string argsCheck1D(vector<ConstantSP> &args);
static string argsCheck2D(vector<ConstantSP> &args);
static ConstantSP fft1D(VectorSP vec, int n, double scale, bool overwrite, bool inverse);
static ConstantSP fft2D(VectorSP matrix, int shapeRow, int shapeCol, double scale, bool overwrite, bool inverse);

//离散余弦变换(DCT-II)
ConstantSP dct(const ConstantSP &a, const ConstantSP &b)
{
    if (!(a->getForm()==DF_VECTOR && a->isNumber() && (a->getCategory() == INTEGRAL || a->getCategory() == FLOATING) && a->size() > 0))
        throw IllegalArgumentException("dct", "The argument should be a nonempty integrial or floating vector.");
    if (a->hasNull())
        throw IllegalArgumentException("dct", "The argument should not contain NULL values");
    int size = a->size();
    vector<double> xn(size, 0); //存储输入的离散信号序列x(n)
    vector<double> xk(size, 0); //存储计算出的离散余弦变换序列X(k)
    a->getDouble(0, size, &xn[0]);
#pragma omp parallel for schedule(static) num_threads(omp_get_num_procs())
    for (int k = 0; k < size; k++)
    {
        double data = 0;
        double ak = k == 0 ? sqrt(1.0 / size) : sqrt(2.0 / size);
        double base_cos = cos(PI * 2 * k / (2 * size));
        double base_sin = sin(PI * 2 * k / (2 * size));
        double last_cos, last_sin, cur_cos, cur_sin;
        for (int j = 0; j < size; j++)
        {
            cur_cos = j == 0 ? cos(PI * k / (2 * size)) : last_cos * base_cos - last_sin * base_sin; //cos(kj)=cos((k-1)j+j)=cos((k-1)j)*cos(j)-sin((k-1)j)*sin(j)
            cur_sin = j == 0 ? sin(PI * k / (2 * size)) : last_sin * base_cos + last_cos * base_sin; //sin(kj)=sin((k-1)j+j)=sin((k-1)j)cosj+cos((k-1)j)*sinj
            last_cos = cur_cos;
            last_sin = cur_sin;
            data += xn[j] * cur_cos;
        }
        xk[k] = ak * data;
    }
    VectorSP res = Util::createVector(DT_DOUBLE, size);
    res->setDouble(0, size, &xk[0]);
    return res;
}
ConstantSP dctMap(Heap *heap, vector<ConstantSP> &args)
{
    TableSP table = args[0];
    int size = args[1]->getInt();
    vector<double> xn(table->rows(), 0);
    vector<double> xk(size, 0);
    vector<int> index_j(table->rows(), 0);
    table->getColumn(0)->getInt(0, table->rows(), &index_j[0]);
    table->getColumn(1)->getDouble(0, table->rows(), &xn[0]);
    omp_lock_t lock;
    omp_init_lock(&lock);
#pragma omp parallel for schedule(static) num_threads(omp_get_num_procs())
    for (int idx = 0; idx < index_j.size(); idx++)
    {
        for (int k = 0; k < size; k++)
        {
            double ak = k == 0 ? sqrt(1.0 / size) : sqrt(2.0 / size);
            omp_set_lock(&lock);
            xk[k] += xn[idx] * cos(PI * k * (2 * index_j[idx] + 1) / (2 * size)) * ak;
            omp_unset_lock(&lock);
        }
    }
    ConstantSP result = Util::createVector(DT_DOUBLE, size);
    result->setDouble(0, size, &xk[0]);
    return result;
}
ConstantSP dctNumMap(Heap *heap, vector<ConstantSP> &args)
{
    TableSP t = args[0];
    int size = t->rows();
    ConstantSP res = Util::createConstant(DT_INT);
    res->setInt(size);
    return res;
}
ConstantSP dctReduce(const ConstantSP &mapRes1, const ConstantSP &mapRes2)
{
    vector<double> xk_1(mapRes1->size(), 0);
    vector<double> xk_2(mapRes2->size(), 0);
    mapRes1->getDouble(0, mapRes1->size(), &xk_1[0]);
    mapRes2->getDouble(0, mapRes2->size(), &xk_2[0]);
#pragma omp parallel for schedule(static) num_threads(omp_get_num_procs())
    for (int i = 0; i < xk_1.size(); i++)
        xk_1[i] += xk_2[i];
    ConstantSP result = Util::createVector(DT_DOUBLE, xk_1.size());
    result->setDouble(0, xk_1.size(), &xk_1[0]);
    return result;
}
ConstantSP dctNumReduce(const ConstantSP &mapRes1, const ConstantSP &mapRes2)
{
    int x1 = mapRes1->getInt();
    int x2 = mapRes2->getInt();
    int size = x1 + x2;
    ConstantSP res = Util::createConstant(DT_INT);
    res->setInt(size);
    return res;
}
ConstantSP dctParallel(Heap *heap, vector<ConstantSP> &args)
{
    ConstantSP ds = args[0];

    FunctionDefSP num_mapfunc = heap->currentSession()->getFunctionDef("signal::dctNumMap");
    FunctionDefSP num_reducefunc = heap->currentSession()->getFunctionDef("signal::dctNumReduce");
    FunctionDefSP mr = heap->currentSession()->getFunctionDef("mr");
    vector<ConstantSP> num_myargs = {ds, num_mapfunc, num_reducefunc};
    ConstantSP size = mr->call(heap, num_myargs);

    FunctionDefSP mapfunc = heap->currentSession()->getFunctionDef("signal::dctMap");
    vector<ConstantSP> mapwithsizearg = {new Void(), size};
    FunctionDefSP mapwithsize = Util::createPartialFunction(mapfunc, mapwithsizearg);
    FunctionDefSP reducefunc = heap->currentSession()->getFunctionDef("signal::dctReduce");
    vector<ConstantSP> myargs = {ds, mapwithsize, reducefunc};
    return mr->call(heap, myargs);
}
//离散正弦变换(DST-I)
ConstantSP dst(const ConstantSP &a, const ConstantSP &b)
{
    if (!(a->getForm()==DF_VECTOR && a->isNumber() && (a->getCategory() == INTEGRAL || a->getCategory() == FLOATING) && a->size() > 0))
        throw IllegalArgumentException("dst", "The argument should be a nonempty integrial or floating vector.");
    if (a->hasNull())
        throw IllegalArgumentException("dst", "The argument should not contain NULL values");
    int size = a->size();
    vector<double> xn(size, 0); //存储输入的离散信号序列x(n)
    vector<double> xk(size, 0); //存储输出的离散正弦变换序列X(k)
    a->getDouble(0, size, &xn[0]);
#pragma omp parallel for schedule(static) num_threads(omp_get_num_procs())
    for (int k = 0; k < size; k++)
    {
        double ak = 2;
        //double ak=sqrt(2.0/(size+1));
        double data = 0;
        double base_cos = cos(PI * (k + 1) / (size + 1));
        double base_sin = sin(PI * (k + 1) / (size + 1));
        double last_cos, last_sin, cur_cos, cur_sin;
        for (int j = 0; j < size; j++)
        {
            cur_cos = j == 0 ? base_cos : last_cos * base_cos - last_sin * base_sin; //cos(kj)=cos((k-1)j+j)=cos((k-1)j)*cos(j)-sin((k-1)j)*sin(j)
            cur_sin = j == 0 ? base_sin : last_sin * base_cos + last_cos * base_sin; //sin(kj)=sin((k-1)j+j)=sin((k-1)j)cosj+cos((k-1)j)*sinj
            last_cos = cur_cos;
            last_sin = cur_sin;
            data += xn[j] * cur_sin;
        }
        xk[k] = ak * data;
    }
    VectorSP res = Util::createVector(DT_DOUBLE, size);
    res->setDouble(0, size, &xk[0]);
    return res;
}

//一维离散小波变换(DWT)
ConstantSP dwt(const ConstantSP &a, const ConstantSP &b)
{
    if (!(a->getForm()==DF_VECTOR && a->isNumber() && (a->getCategory() == INTEGRAL || a->getCategory() == FLOATING) && a->size() > 0))
        throw IllegalArgumentException("dwt", "The argument should be a nonempty integrial or floating vector.");
    if (a->hasNull())
        throw IllegalArgumentException("dwt", "The argument should not contain NULL values");
    int dataLen = a->size(); //信号序列长度
    vector<double> FilterLD = {
        0.7071067811865475244008443621048490392848359376884740365883398,
        0.7071067811865475244008443621048490392848359376884740365883398}; //基于db1小波函数的滤波器低通序列
    vector<double> FilterHD = {
        -0.7071067811865475244008443621048490392848359376884740365883398,
        0.7071067811865475244008443621048490392848359376884740365883398}; //基于db1小波函数的滤波器通序列
    const int filterLen = 2;                                              //滤波器序列长度
    int decLen = (dataLen + filterLen - 1) / 2;                           //小波变换后的序列长度
    vector<double> xn(dataLen, 0);
    vector<double> cA(decLen, 0);
    vector<double> cD(decLen, 0);
    a->getDouble(0, dataLen, &xn[0]);
#pragma omp sections
    {
#pragma omp section
        {
            dwt_get(filterLen, dataLen, xn, FilterLD, cA);
        }
#pragma omp section
        {
            dwt_get(filterLen, dataLen, xn, FilterHD, cD);
        }
    }
    VectorSP res_cA = Util::createVector(DT_DOUBLE, decLen);
    VectorSP res_cD = Util::createVector(DT_DOUBLE, decLen);
    res_cA->setDouble(0, decLen, &cA[0]);
    res_cD->setDouble(0, decLen, &cD[0]);
    vector<string> colNames = {"cA", "cD"}; //cA:分解后的近似部分序列-低频部分  cD:分解后的细节部分序列-高频部分
    vector<ConstantSP> columns;
    columns.emplace_back(res_cA);
    columns.emplace_back(res_cD);
    TableSP t = Util::createTable(colNames, columns);
    return t;
}

//一维离散小波逆变换(IDWT)
ConstantSP idwt(const ConstantSP &a, const ConstantSP &b)
{
    if (!(a->getForm()==DF_VECTOR && a->isNumber() && (a->getCategory() == INTEGRAL || a->getCategory() == FLOATING) && a->size() > 0))
        throw IllegalArgumentException("idwt", "The argument 1 should be a nonempty integrial or floating vector.");
    if (!(b->getForm()==DF_VECTOR && b->isNumber() && (b->getCategory() == INTEGRAL || b->getCategory() == FLOATING) && b->size() > 0))
        throw IllegalArgumentException("idwt", "The argument 2 should be a nonempty integrial or floating vector.");
    if (a->size() != b->size())
        throw IllegalArgumentException("idwt", "two arguments should have the same size.");
    if(a->hasNull())
        throw IllegalArgumentException("idwt", "The argument 1 should not contain NULL values");
    if(b->hasNull())
        throw IllegalArgumentException("idwt", "The argument 2 should not contain NULL values"); 
    vector<double> FilterLR = {
        0.7071067811865475244008443621048490392848359376884740365883398,
        0.7071067811865475244008443621048490392848359376884740365883398}; //基于db1小波函数的滤波器低通序列
    vector<double> FilterHR = {
        0.7071067811865475244008443621048490392848359376884740365883398,
        -0.7071067811865475244008443621048490392848359376884740365883398}; //基于db1小波函数的滤波器通序列
    int dataLen = a->size();
    const int filterLen = 2;
    int recLen = dataLen * 2;
    vector<double> cA(dataLen, 0);
    vector<double> cD(dataLen, 0);
    vector<double> recData(recLen, 0);
    a->getDouble(0, dataLen, &cA[0]);
    b->getDouble(0, dataLen, &cD[0]);
    omp_lock_t _lock;
    omp_init_lock(&_lock);
#pragma omp sections
    {
#pragma omp section
        {
            idwt_get(filterLen, dataLen, cA, FilterLR, recData, _lock);
        }
#pragma omp section
        {
            idwt_get(filterLen, dataLen, cD, FilterHR, recData, _lock);
        }
    }
    VectorSP res = Util::createVector(DT_DOUBLE, recLen);
    res->setDouble(0, recLen, &recData[0]);
    return res;
}
static void dwt_get(int decLen, int dataLen, vector<double> &input, vector<double> &Filter, vector<double> &output)
{
    int step = 2;
    int i = step - 1, idx = 0;

    for (; i < decLen && i < dataLen; i += step, ++idx)
    {
        double sum = 0;
        int j;
        for (j = 0; j <= i; j++)
            sum += Filter[j] * input[i - j];
        while (j < decLen)
        {
            int k;
            for (k = 0; k < dataLen && j < decLen; ++j, ++k)
                sum += Filter[j] * input[k];
            for (k = 0; k < decLen && j < decLen; ++k, ++j)
                sum += Filter[j] * input[dataLen - 1 - k];
        }
        output[idx] = sum;
    }
    for (; i < dataLen; i += step, ++idx)
    {
        double sum = 0;
        for (int j = 0; j < decLen; ++j)
            sum += input[i - j] * Filter[j];
        output[idx] = sum;
    }
    for (; i < decLen; i += step, ++idx)
    {
        double sum = 0;
        int j = 0;
        while (i - j >= dataLen)
        {
            int k;
            for (k = 0; k < dataLen && i - j >= dataLen; ++j, ++k)
                sum += Filter[i - dataLen - j] * input[dataLen - 1 - k];
            for (k = 0; k < dataLen && i - j >= dataLen; ++j, ++k)
                sum += Filter[i - dataLen - j] * input[k];
        }
        for (; j <= i; ++j)
            sum += Filter[j] * input[i - j];
        while (j < decLen)
        {
            int k;
            for (k = 0; k < dataLen && j < decLen; ++j, ++k)
                sum += Filter[j] * input[k];
            for (k = 0; k < dataLen && j < decLen; ++k, ++j)
                sum += Filter[j] * input[dataLen - 1 - k];
        }
        output[idx] = sum;
    }
    for (; i < dataLen + decLen - 1; i += step, ++idx)
    {
        double sum = 0;
        int j = 0;
        while (i - j >= dataLen)
        {
            int k;
            for (k = 0; k < dataLen && i - j >= dataLen; ++j, ++k)
                sum += Filter[i - dataLen - j] * input[dataLen - 1 - k];
            for (k = 0; k < dataLen && i - j >= dataLen; ++j, ++k)
                sum += Filter[i - dataLen - j] * input[k];
        }
        for (; j < decLen; ++j)
            sum += Filter[j] * input[i - j];
        output[idx] = sum;
    }
}
static void idwt_get(int recLen, int dataLen, vector<double> &input, vector<double> &Filter, vector<double> &output, omp_lock_t &_lock)
{
    int idx, i;
    for (idx = 0, i = recLen / 2 - 1; i < dataLen; ++i, idx += 2)
    {
        double sum_even = 0;
        double sum_odd = 0;
        for (int j = 0; j < recLen / 2; ++j)
        {
            sum_even += Filter[j * 2] * input[i - j];
            sum_odd += Filter[j * 2 + 1] * input[i - j];
        }
        omp_set_lock(&_lock);
        output[idx] += sum_even;
        output[idx + 1] += sum_odd;
        omp_unset_lock(&_lock);
    }
}

static string argsCheck1D(vector<ConstantSP> &args)
{
    if (args.size() < 1 || args.size() > 3)
        return "Need 1-3 arguments";
    if (!(args[0]->isVector() && (args[0]->getType() == DT_COMPLEX || args[0]->isNumber()) && args[0]->size() > 0 && !args[0]->hasNull()))
        return "The first argument should be a nonempty vector";
    if (args.size() > 1)
    {
        if (!args[1]->isScalar() || args[1]->getType() != DT_INT || args[1]->getInt() <= 0)
            return "The second argument should be positive integer";
    }
    if (args.size() > 2)
    {
        if (!args[2]->isScalar() || args[2]->getType() != DT_STRING || (args[2]->getString() != "forward" && args[2]->getString() != "backward" && args[2]->getString() != "ortho"))
            return "The third argument should be forward,backward or ortho";
    }
    return "";
}

static ConstantSP fft1D(VectorSP vec, int n, double scale, bool overwrite, bool inverse)
{
    if (!fftwInit) {
        if (!fftw_init_threads())
            throw RuntimeException("Failed to init fftw");
        fftwInit = true;
        fftw_plan_with_nthreads(omp_get_num_threads());
    }

    int vSize = vec->size();
    if (n == -1)
        n = vSize;
    fftw_complex* a = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * std::max(vSize, n));
    double* buf = nullptr;
    fftw_plan p;
    if (vec->isNumber())
    {
        buf = new double[std::max(vSize, n)];
        p = fftw_plan_dft_r2c_1d(n, buf, a, FFTW_ESTIMATE);
        vec->getDouble(0, vSize, buf);
    }
    else {
        if (inverse)
            p = fftw_plan_dft_1d(n, a, a, FFTW_FORWARD, FFTW_ESTIMATE);
        else
            p = fftw_plan_dft_1d(n, a, a, FFTW_BACKWARD, FFTW_ESTIMATE);
        vec->getBinary(0, vSize, 16, (unsigned char *)a);
    }
    if (n > vSize) {
        if (vec->isNumber()) {
            memset(buf + vSize, 0, (n - vSize) * sizeof(double));
        }
        else {
            for (int i = vSize; i < n; i++) {
                a[i][0] = 0;
                a[i][1] = 0;
            }
        }
    }

    fftw_execute(p);

    a[0][0] *= scale;
    a[0][1] *= scale;
    if (vec->isNumber()) {
        for (int i = 1; i <= n / 2; i++) {
            a[i][0] *= scale;
            a[i][1] *= scale;
            a[n - i][0] = a[i][0];
            a[n - i][1] = (-a[i][1]);
            if (inverse) {
                std::swap(a[i][0], a[n - i][0]);
                std::swap(a[i][1], a[n - i][1]);
            }
        }
        if (n % 2 == 0) {
            a[n / 2][1] = (-a[n / 2][1]);
        } 
    }
    else {
        for (int i = 1; i <= n / 2; i++) {
            a[i][0] *= scale;
            a[i][1] *= scale;
            a[n - i][0] *= scale;
            a[n - i][1] *= scale;
            std::swap(a[i][0], a[n - i][0]);
            std::swap(a[i][1], a[n - i][1]);
        }
        if (n % 2 == 0) {
            a[n / 2][0] /= scale;
            a[n / 2][1] /= scale;
        }    
    }

    if (overwrite && vec->getType() == DT_COMPLEX && vec->size() >= n)
        vec->setBinary(0, n, 16, (unsigned char *)a);
    VectorSP res = Util::createVector(DT_COMPLEX, n, n);
    res->setBinary(0, n, 16, (unsigned char *)a);
    fftw_free(a);
    if (buf)
        delete[] buf;
    return res;
}

ConstantSP fft(Heap* heap, vector<ConstantSP>& args)
{
    string check = argsCheck1D(args);
    if (check != "")
        throw IllegalArgumentException("fft", check);
    ConstantSP vec = args[0];
    int n = -1;
    double scale = 1;
    bool overwrite = false;
    if (args.size() > 1)
        n = args[1]->getInt();
    if (args.size() > 2)
    {
        if (args[2]->getString() == "forward")
            scale = (double)1 / n;
        else if (args[2]->getString() == "ortho")
            scale = (double)1 / sqrt(n);
    }
    return fft1D(vec, n, scale, overwrite, false);
}

ConstantSP fft1(Heap *heap, vector<ConstantSP> &args)
{
    string check = argsCheck1D(args);
    if (check != "")
        throw IllegalArgumentException("fft!", check);
    ConstantSP vec = args[0];
    int n = -1;
    double scale = 1;
    bool overwrite = true;
    if (args.size() > 1)
        n = args[1]->getInt();
    if (args.size() > 2)
    {
        if (args[2]->getString() == "forward")
            scale = (double)1 / n;
        else if (args[2]->getString() == "ortho")
            scale = (double)1 / sqrt(n);
    }
    return fft1D(vec, n, scale, overwrite, false);
}

ConstantSP ifft(Heap *heap, vector<ConstantSP> &args)
{
    string check = argsCheck1D(args);
    if (check != "")
        throw IllegalArgumentException("ifft", check);
    ConstantSP vec = args[0];
    int n = -1;
    bool overwrite = false;
    if (args.size() > 1)
        n = args[1]->getInt();
    double scale = (double)1 / n;
    if (args.size() > 2)
    {
        if (args[2]->getString() == "forward")
            scale = 1;
        else if (args[2]->getString() == "ortho")
            scale = (double)1 / sqrt(n);
    }
    return fft1D(vec, n, scale, overwrite, true);
}

ConstantSP ifft1(Heap *heap, vector<ConstantSP> &args)
{
    string check = argsCheck1D(args);
    if (check != "")
        throw IllegalArgumentException("ifft!", check);
    ConstantSP vec = args[0];
    int n = -1;

    bool overwrite = true;
    if (args.size() > 1)
        n = args[1]->getInt();
    double scale = (double)1 / n;
    if (args.size() > 2)
    {
        if (args[2]->getString() == "forward")
            scale = 1;
        else if (args[2]->getString() == "ortho")
            scale = (double)1 / sqrt(n);
    }
    return fft1D(vec, n, scale, overwrite, true);
}

static string argsCheck2D(vector<ConstantSP> &args)
{
    if (args.size() < 1 || args.size() > 3)
        return "Need 1-3 arguments";
    if (!(args[0]->isMatrix() && (args[0]->getType() == DT_COMPLEX || args[0]->isNumber()) && args[0]->size() > 0 && !args[0]->hasNull()))
        return "The first argument should be a nonempty matrix";
    if (args.size() > 1)
    { //shape of matrix
        if (!args[1]->isVector() || args[1]->getType() != DT_INT || args[1]->size() != 2 || args[1]->hasNull())
            return "The second argument should be a vector with 2 positive integer";
        VectorSP shape = args[1];
        if (shape->getInt(0) <= 0 || shape->getInt(1) <= 0)
            return "The second argument should be a vector with 2 positive integer";
    }
    if (args.size() > 2)
    { //Normalization
        if (!args[2]->isScalar() || args[2]->getType() != DT_STRING || (args[2]->getString() != "forward" && args[2]->getString() != "backward" && args[2]->getString() != "ortho"))
            return "The third argument should be forward,backward or ortho";
    }
    return "";
}

static ConstantSP fft2D(VectorSP matrix, int shapeRow, int shapeCol, double scale, bool overwrite, bool inverse)
{
    if (!fftwInit) {
        if (!fftw_init_threads())
            throw RuntimeException("Failed to init fftw");
        fftwInit = true;
        fftw_plan_with_nthreads(omp_get_num_threads());
    }
    int rows = matrix->rows();
    int cols = matrix->columns();
    int n = shapeRow * shapeCol;
    fftw_complex *a = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * n);
    fftw_plan p;
    if (inverse)
        p = fftw_plan_dft_2d(shapeRow, shapeCol, &a[0], &a[0], FFTW_FORWARD, FFTW_ESTIMATE);
    else
        p = fftw_plan_dft_2d(shapeRow, shapeCol, &a[0], &a[0], FFTW_BACKWARD, FFTW_ESTIMATE);
    memset(a, 0, sizeof(fftw_complex) * shapeCol * shapeRow);
    if (matrix->isNumber())
    {
        for (int i = 0; i < std::min(rows, shapeRow); i++)
        {
            for (int j = 0; j < std::min(cols, shapeCol); j++)
            {
                int idxa = i * shapeCol + j;
                int idxm = j * rows + i;
                a[idxa][0] = matrix->getDouble(idxm);
                a[idxa][1] = 0;
            }
        }
    }
    else
    {
        fftw_complex *temp = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * rows * cols);
        memset(temp, 0, sizeof(fftw_complex) * rows * cols);
        matrix->getBinary(0, rows * cols, 16, (unsigned char *)temp);
        for (int i = 0; i < std::min(rows, shapeRow); i++)
        {
            for (int j = 0; j < std::min(cols, shapeCol); j++)
            {
                int idxa = i * shapeCol + j;
                int idxt = j * rows + i;
                a[idxa][0] = temp[idxt][0];
                a[idxa][1] = temp[idxt][1];
            }
        }
        fftw_free(temp);
    }
    fftw_execute(p);
    VectorSP res = Util::createMatrix(DT_COMPLEX, shapeCol, shapeRow, shapeCol);
    fftw_complex *coli = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * shapeRow);
    coli[0][0] = a[0][0] * scale;
    coli[0][1] = a[0][1] * scale;
    for (int j = 1; j < shapeRow; j++)
    {
        coli[shapeRow - j][0] = a[j * shapeCol][0] * scale;
        coli[shapeRow - j][1] = a[j * shapeCol][1] * scale;
    }
    res->setBinary(0, shapeRow, sizeof(fftw_complex), (unsigned char *)coli);
    for (int i = 1; i < shapeCol; i++)
    {
        coli[0][0] = a[i][0] * scale;
        coli[0][1] = a[i][1] * scale;
        for (int j = 1; j < shapeRow; j++)
        {
            coli[shapeRow - j][0] = a[j * shapeCol + i][0] * scale;
            coli[shapeRow - j][1] = a[j * shapeCol + i][1] * scale;
        }
        res->setBinary(n - (i * shapeRow), shapeRow, sizeof(fftw_complex), (unsigned char *)coli);
    }
    if (overwrite && matrix->getType() == DT_COMPLEX && rows >= shapeRow && cols >= shapeCol)
    {
        for (int i = 0; i < shapeCol; i++)
        {
            res->getBinary(i * shapeRow, shapeRow, sizeof(fftw_complex), (unsigned char *)coli);
            matrix->setBinary(i * rows, shapeRow, sizeof(fftw_complex), (unsigned char *)coli);
        }
    }
    fftw_free(coli);
    fftw_free(a);
    return res;
}

ConstantSP fft2(Heap *heap, vector<ConstantSP> &args)
{
    string check = argsCheck2D(args);
    if (check != "")
        throw IllegalArgumentException("fft2", check);
    VectorSP matrix = args[0];
    int shapeRow = matrix->rows();
    int shapeCol = matrix->columns();
    double scale = 1;
    bool overwrite = false;
    if (args.size() > 1)
    {
        VectorSP shape = args[1];
        shapeRow = shape->getInt(0);
        shapeCol = shape->getInt(1);
    }
    if (args.size() > 2)
    {
        int n = shapeRow * shapeCol;
        if (args[2]->getString() == "forward")
            scale = (double)1 / n;
        else if (args[2]->getString() == "ortho")
            scale = (double)1 / sqrt(n);
    }
    return fft2D(matrix, shapeRow, shapeCol, scale, overwrite, false);
}

ConstantSP fft21(Heap *heap, vector<ConstantSP> &args)
{
    string check = argsCheck2D(args);
    if (check != "")
        throw IllegalArgumentException("fft2!", check);
    VectorSP matrix = args[0];
    int shapeRow = matrix->rows();
    int shapeCol = matrix->columns();
    double scale = 1;
    bool overwrite = true;
    if (args.size() > 1)
    {
        VectorSP shape = args[1];
        shapeRow = shape->getInt(0);
        shapeCol = shape->getInt(1);
    }
    if (args.size() > 2)
    {
        int n = shapeRow * shapeCol;
        if (args[2]->getString() == "forward")
            scale = (double)1 / n;
        else if (args[2]->getString() == "ortho")
            scale = (double)1 / sqrt(n);
    }
    return fft2D(matrix, shapeRow, shapeCol, scale, overwrite, false);
}

ConstantSP ifft2(Heap *heap, vector<ConstantSP> &args)
{
    string check = argsCheck2D(args);
    if (check != "")
        throw IllegalArgumentException("ifft2", check);
    VectorSP matrix = args[0];
    int shapeRow = matrix->rows();
    int shapeCol = matrix->columns();
    bool overwrite = false;
    if (args.size() > 1)
    {
        VectorSP shape = args[1];
        shapeRow = shape->getInt(0);
        shapeCol = shape->getInt(1);
    }
    int n = shapeRow * shapeCol;
    double scale = (double)1 / n;
    if (args.size() > 2)
    {
        if (args[2]->getString() == "forward")
            scale = 1;
        else if (args[2]->getString() == "ortho")
            scale = (double)1 / sqrt(n);
    }
    return fft2D(matrix, shapeRow, shapeCol, scale, overwrite, true);
}

ConstantSP ifft21(Heap *heap, vector<ConstantSP> &args)
{
    string check = argsCheck2D(args);
    if (check != "")
        throw IllegalArgumentException("ifft2!", check);
    VectorSP matrix = args[0];
    int shapeRow = matrix->rows();
    int shapeCol = matrix->columns();
    bool overwrite = true;
    if (args.size() > 1)
    {
        VectorSP shape = args[1];
        shapeRow = shape->getInt(0);
        shapeCol = shape->getInt(1);
    }
    int n = shapeRow * shapeCol;
    double scale = (double)1 / n;
    if (args.size() > 2)
    {
        if (args[2]->getString() == "forward")
            scale = 1;
        else if (args[2]->getString() == "ortho")
            scale = (double)1 / sqrt(n);
    }
    return fft2D(matrix, shapeRow, shapeCol, scale, overwrite, true);
}

ConstantSP secc(Heap *heap, vector<ConstantSP> &args)
{
    if (args.size() > 5 || args.size() < 3)
        throw IllegalArgumentException("secc", "Need 3-5 arguments");
    if (!args[0]->isVector() || !args[0]->isNumber() || args[0]->size() <= 0)
        throw IllegalArgumentException("secc", "The first argument should be a nonempty vector");
    if (args[0]->hasNull())
        throw IllegalArgumentException("secc", "The first argument should be a vector without NULL elements");
    VectorSP vsp = args[0];
    int lenData = vsp->size();
    if (!args[1]->isMatrix() || !args[1]->isNumber() || args[1]->size() <= 0)
        throw IllegalArgumentException("secc", "The second argument should be a nonempty matrix");
    if (args[1]->hasNull())
        throw IllegalArgumentException("secc", "The second argument should be a matrix without NULL elements");
    VectorSP msp = args[1];
    int rows = msp->rows();
    int cols = msp->columns();
    if (lenData < rows)
        throw IllegalArgumentException("secc", "The length of data should not be less than the number of rows of templates");
    int m = rows;
    if (!args[2]->isScalar() || args[2]->getType() != DT_INT || args[2]->isNull())
        throw IllegalArgumentException("secc", "The third argument should be a positive integer");
    if (args[2]->getInt() < 2 * m)
        throw IllegalArgumentException("secc", "For better performance, k is at least twice the number of rows of templates");
    int k = args[2]->getInt();
    vector<double> mouts(cols, 0);
    vector<double> weight(cols, 1);
    if (args.size() > 3)
    {
        if (!args[3]->isVector() || !args[3]->isNumber() || args[3]->hasNull())
            throw IllegalArgumentException("secc", "The fourth argument should be a nonempty vector");
        if (args[3]->size() != cols)
            throw IllegalArgumentException("secc", "The length of moveouts should be the same as the number of columns of templates");
        VectorSP moveouts = args[3];
        moveouts->getDouble(0, moveouts->size(), &mouts[0]);
        double maxOuts = *std::max_element(mouts.begin(), mouts.end());
        for (int i = 0; i < mouts.size(); i++)
            mouts[i] = maxOuts - mouts[i];
    }
    if (args.size() > 4)
    {
        if (!args[4]->isVector() || !args[4]->isNumber() || args[4]->hasNull())
            throw IllegalArgumentException("secc", "The fifth argument should be a nonempty vector");
        if (args[4]->size() != cols)
            throw IllegalArgumentException("secc", "The length of weights should be the same as the number of columns of templates");
        VectorSP weights = args[4];
        weights->getDouble(0, cols, &weight[0]);
    }
    //sumy2 = sqrt(sum(y.^2))
    vector<double> y(rows * cols, 0);
    msp->getDouble(0, rows * cols, &y[0]);
    vector<double> sumy2(cols, 0);
    for (int i = 0; i < cols; i++)
    {
        double sum = 0;
        for (int j = 0; j < rows; j++)
        {
            double t = y[i * rows + j];
            sum += (t * t);
        }
        sumy2[i] = sqrt(sum);
    }

    //s=buffer(data(:,jj,j),k,m-1)
    int sRows = lenData / (k - m + 1) + 1;
    vector<double> s(sRows * k, 0);
    int sIdx = m - 1;
    int vspIdx = 0;
    for (; vspIdx < vsp->size();)
    {
        s[sIdx++] = vsp->getDouble(vspIdx++);
        if (sIdx % k == 0)
            vspIdx -= (m - 1);
    }

    //sumx2_t=sqrt(movsum(s.^2,[m-1,0]));
    vector<double> sumx2(sRows * k, 0);
    for (int i = 0; i < sRows; i++)
    {
        double sum = 0;
        sIdx = i * k;
        for (int j = 0; j < m; j++)
        {
            sum += (s[sIdx] * s[sIdx]);
            sumx2[sIdx++] = sqrt(sum);
        }
        for (int j = m; j < k; j++)
        {
            sum += (s[sIdx] * s[sIdx] - s[sIdx - m] * s[sIdx - m]);
            sumx2[sIdx++] = sqrt(sum);
        }
    }

    //yz = y(end:-1:1,:);  reverse the templates
    //yz(m+1:k,:) = 0; padding with zero
    vector<double> yz(cols * k, 0);
    for (int i = 0; i < cols; i++)
    {
        for (int j = 0; j < rows; j++)
        {
            if (j < rows / 2)
                std::swap(y[i * rows + j], y[(i + 1) * rows - 1 - j]);
            yz[i * k + j] = y[i * rows + j];
        }
    }
    y.clear();

    //X=fft(s),Y=fft(y);
    fftw_plan psy;
    fftw_complex *X = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * sRows * k);
    fftw_complex *Y = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * cols * k);
    vector<double> in(k);
    psy = fftw_plan_dft_r2c_1d(k, &in[0], &X[0], FFTW_BACKWARD);
    in.clear();
    for (int i = 0; i < sRows; i++)
    {
        psy = fftw_plan_dft_r2c_1d(k, &s[i * k], &X[i * k], FFTW_BACKWARD);
        fftw_execute(psy);
    }
    for (int i = 0; i < cols; i++)
    {
        psy = fftw_plan_dft_r2c_1d(k, &yz[i * k], &Y[i * k], FFTW_BACKWARD);
        fftw_execute(psy);
    }

    VectorSP res = Util::createMatrix(DT_DOUBLE, cols, lenData - m + 1, cols);
    for (int c = 0; c < cols; c++)
    {
        //Z = X.*Y(:,i);
        fftw_complex *Z = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * sRows * k);
        for (int i = 0; i < sRows; i++)
        {
            for (int j = 0; j < k; j++)
            {
                Z[i * k + j][0] = X[i * k + j][0] * Y[c * k + j][0] - X[i * k + j][1] * Y[c * k + j][1];
                Z[i * k + j][1] = X[i * k + j][1] * Y[c * k + j][0] + X[i * k + j][0] * Y[c * k + j][1];
            }
        }

        //z=ifft(Z)
        vector<double> z(sRows * k);
        fftw_plan pz;
        fftw_complex *infc = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * k);
        pz = fftw_plan_dft_c2r_1d(k, infc, &z[0], FFTW_BACKWARD);
        fftw_free(infc);
        for (int i = 0; i < sRows; i++)
        {
            pz = fftw_plan_dft_c2r_1d(k, &Z[i * k], &z[i * k], FFTW_BACKWARD);
            fftw_execute(pz);
        }
        for (int i = 0; i < sRows * k; i++)
            z[i] /= k;

        // ccha=z(m:k,:)./(sumx2_t(m:k,:)*sumy2(i)); %devide by the normalization factor
        vector<double> ccha((k - m + 1) * sRows, 0);
        int cchaIdx = 0;
        for (int i = 0; i < sRows; i++)
        {
            for (int j = m - 1; j < k; j++)
            {
                int idx = i * k + j;
                ccha[cchaIdx++] = z[idx] / (sumx2[idx] * sumy2[c]);
            }
        }

        // ccc_sum(:,i)=weights(j,i).*([zeros(1,moveouts(j,i)),ccha(m:l_data-moveouts(j,i))])
        int cIdx = mouts[c];
        vector<double> column(lenData - m + 1, 0);
        for (int i = m - 1; i < lenData - mouts[c]; i++)
            column[cIdx++] = weight[c] * ccha[i];

        int resIdx = c * (lenData - m + 1);
        for (int i = 0; i < lenData - m + 1; i++)
        {
            if (std::isnan(column[i]))
                res->setNull(resIdx++);
            else
                res->setDouble(resIdx++, column[i]);
        }
        fftw_free(Z);
    }
    fftw_free(X);
    fftw_free(Y);
    return res;
}