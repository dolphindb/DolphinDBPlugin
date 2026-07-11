#include "fftw.h"

#include "signal.h"
#include "CoreConcept.h"
#include "DolphinDBEverything.h"
#include "Exceptions.h"
#include "PluginLogger.h"
#include "Types.h"
#include "Util.h"
#include <chrono>
#include <tuple>
#include <vector>
#include <string>
#include <omp.h>
#include "OperatorImp.h"
#include <ScalarImp.h>

#if defined(_MSC_VER)
#pragma warning( push )
#elif defined(__clang__)
#pragma clang diagnostic push
#else // gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include "wavelib.h"

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#else // gcc
#pragma GCC diagnostic pop
#endif

#include <array>

using namespace ddb;

#define PI 3.1415926
static void dwt_get(int, int, vector<double>&, vector<double>&, vector<double>&);
static void idwt_get(int, int, vector<double> &, vector<double> &, vector<double> &, omp_lock_t &);

namespace {

// NOLINTBEGIN(cert-err58-cpp)
ConstantSP placeholder;
ConstantSP complex_zero;
// NOLINTEND(cert-err58-cpp)

fftw_plan_manager fftw;

void dct2(const fftw_memory<double> &buf)
{
    auto n = buf.size();
    fftw.execute(n, buf.get(), FFTW_REDFT10);
    double factor0 = 0.5 * sqrt(1.0 / n);
    double factorK = 0.5 * sqrt(2.0 / n);
    *buf[0] *= factor0;
#pragma omp parallel for schedule(static) if(n > OMP_THRESHOLD)
    for (size_t k = 1; k < n; k++) {
        *buf[k] *= factorK;
    }
}

void to_double(Heap *heap, ConstantSP &val)
{
    if (val->getType() != ddb::DT_DOUBLE && val->getType() != ddb::DT_COMPLEX) {
        val = OperatorImp::asDouble(heap, val, placeholder);
    }
    val = Util::asContiguous(val);
}

void to_complex(Heap *heap, ConstantSP &val)
{
    if (val->getType() != ddb::DT_COMPLEX) {
        val = OperatorImp::asComplex(heap, val, new Double(0.0));
    }
    val = Util::asContiguous(val);
}

fftw_complex* get_ptr_complex(const VectorSP &X)
{
    return reinterpret_cast<fftw_complex *>(X->getBinaryBuffer(0, X->size(), complex_size, nullptr)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
}

double* get_ptr_double(const VectorSP &X)
{
    return X->getDoubleBuffer(0, X->size(), nullptr);
}

std::string NULL_ERROR;

} // namespace

ddb::ConstantSP initialize(ddb::Heap* heap, argsT &args)
{
    std::ignore = heap;
    std::ignore = args;
    if (fftw_init_threads() == 0) {
        throw RuntimeException("Plugin dependency init failed: fftw");
    }
    LOG_INFO("FFTW init success.");
    complex_zero = new Complex(0.0, 0.0);
    placeholder = Util::createConstant(DT_VOID);
    NULL_ERROR = " should not contain NULL values";
    return placeholder;
}

ddb::ConstantSP fftwConfig(ddb::Heap* heap, argsT &args)
{
    std::ignore = heap;
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw RuntimeException("config must be a string scalar.");
    }
    std::string config = args[0]->getString();
    if (config == "measure") {
        fftw.set_plan_flag(FFTW_MEASURE);
    } else if (config == "estimate") {
        fftw.set_plan_flag(FFTW_ESTIMATE);
    } else if (config == "clear_plans") {
        fftw.clear_plans();
    } else {
        throw RuntimeException("invalid config option: " + config);
    }
    return placeholder;
}

//离散余弦变换(DCT-II)
ConstantSP dct(Heap *heap, const ConstantSP &a, const ConstantSP &b)
{
    std::ignore = b;
    std::ignore = heap;
    if (!(a->getForm()==DF_VECTOR && a->isNumber() && (a->getCategory() == INTEGRAL || a->getCategory() == FLOATING) && a->size() > 0))
        throw IllegalArgumentException("dct", "[X] should be a nonempty integrial or floating vector.");
    if (a->hasNull())
        throw IllegalArgumentException("dct", "[X]" + NULL_ERROR);
    int size = a->size();
    fftw_memory<double> buf(size);
    a->getDouble(0, size, buf.get());
    dct2(buf);
    VectorSP res = Util::createVector(DT_DOUBLE, size);
    res->setDouble(0, size, buf.get());
    return res;
}

//离散正弦变换(DST-I)
ConstantSP dst(Heap *heap, const ConstantSP &a, const ConstantSP &b)
{
    std::ignore = b;
    std::ignore = heap;
    if (!(a->getForm()==DF_VECTOR && a->isNumber() && (a->getCategory() == INTEGRAL || a->getCategory() == FLOATING) && a->size() > 0)) {
        throw IllegalArgumentException("dst", "[X] should be a nonempty integrial or floating vector.");
    }
    if (a->hasNull()) {
        throw IllegalArgumentException("dst", "[X]" + NULL_ERROR);
    }
    int n = a->size();

    fftw_memory<double> buf(n);
    a->getDouble(0, n, buf.get());
    fftw.execute(n, buf.get(), FFTW_RODFT00);
    VectorSP res = Util::createVector(DT_DOUBLE, n);
    res->setDouble(0, n, buf.get());
    return res;
}

ConstantSP dctMap(Heap *heap, vector<ConstantSP> &args)
{
    std::ignore = heap;
    TableSP table = args[0];
    int size = args[1]->getInt();
    vector<double> xn(table->rows(), 0);
    vector<double> xk(size, 0);
    vector<int> index_j(table->rows(), 0);
    table->getColumn(0)->getInt(0, table->rows(), &index_j[0]);
    table->getColumn(1)->getDouble(0, table->rows(), &xn[0]);
    for (size_t idx = 0; idx < index_j.size(); idx++)
    {
        for (int k = 0; k < size; k++)
        {
            double ak = k == 0 ? sqrt(1.0 / size) : sqrt(2.0 / size);
            xk[k] += xn[idx] * cos(PI * k * (2 * index_j[idx] + 1) / (2 * size)) * ak;
        }
    }
    ConstantSP result = Util::createVector(DT_DOUBLE, size);
    result->setDouble(0, size, &xk[0]);
    return result;
}

ConstantSP dctNumMap(Heap *heap, vector<ConstantSP> &args)
{
    std::ignore = heap;
    TableSP t = args[0];
    int size = t->rows();
    ConstantSP res = Util::createConstant(DT_INT);
    res->setInt(size);
    return res;
}

ConstantSP dctReduce(Heap *heap, const ConstantSP &mapRes1, const ConstantSP &mapRes2)
{
    std::ignore = heap;
    vector<double> xk_1(mapRes1->size(), 0);
    vector<double> xk_2(mapRes2->size(), 0);
    mapRes1->getDouble(0, mapRes1->size(), &xk_1[0]);
    mapRes2->getDouble(0, mapRes2->size(), &xk_2[0]);
    for (size_t i = 0; i < xk_1.size(); i++)
        xk_1[i] += xk_2[i];
    ConstantSP result = Util::createVector(DT_DOUBLE, xk_1.size());
    result->setDouble(0, xk_1.size(), &xk_1[0]);
    return result;
}

ConstantSP dctNumReduce(Heap *heap, const ConstantSP &mapRes1, const ConstantSP &mapRes2)
{
    std::ignore = heap;
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

    FunctionDefSP num_mapfunc = Util::getFuncDefFromHeap(heap, "signal::dctNumMap");
    FunctionDefSP num_reducefunc = Util::getFuncDefFromHeap(heap, "signal::dctNumReduce");
    FunctionDefSP mr = Util::getFuncDefFromHeap(heap, "mr");
    vector<ConstantSP> num_myargs = {ds, num_mapfunc, num_reducefunc};
    ConstantSP size = mr->call(heap, num_myargs);

    FunctionDefSP mapfunc = Util::getFuncDefFromHeap(heap, "signal::dctMap");
    vector<ConstantSP> mapwithsizearg = {new Void(), size};
    FunctionDefSP mapwithsize = Util::createPartialFunction(mapfunc, mapwithsizearg);
    FunctionDefSP reducefunc = Util::getFuncDefFromHeap(heap, "signal::dctReduce");
    vector<ConstantSP> myargs = {ds, mapwithsize, reducefunc};
    return mr->call(heap, myargs);
}

ConstantSP dwtEx(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    //X
    if (!(args[0]->getForm()==DF_VECTOR && (args[0]->getCategory() == INTEGRAL || args[0]->getCategory() == FLOATING) && args[0]->size() > 0)) {
        throw IllegalArgumentException("dwtEx", "[X] should be a nonempty integrial or floating vector.");
    }
    if (args[0]->hasNull()) {
        throw IllegalArgumentException("dwtEx", "[X]" + NULL_ERROR);
    }
    int dataLen = args[0]->size();
    vector<double> xn(dataLen, 0);
    args[0]->getDouble(0, dataLen, &xn[0]);

    //wavelet
    std::string wavelet = "db1";
    if (args.size() > 1 && !args[1]->isNothing()) {
        if(args[1]->getForm() != DF_SCALAR || args[1]->getType() != DT_STRING) {
            throw IllegalArgumentException("dwtEx", "[wavelet] should be a string scalar.");
        }
        wavelet = args[1]->getString();
        static std::set<std::string> validWavelet{"db1", "db2", "db3", "db4", "db5", "db6", "db7",
            "db8", "db9", "db10", "db11", "db12", "db13", "db14", "db15"};
        if(validWavelet.count(wavelet) == 0) {
            throw IllegalArgumentException("dwtEx", std::string("Invalid value for [wavelet]") + wavelet);
        }
    }

    //level
    int level = 1;
    if (args.size() > 2 && !args[2]->isNothing()) {
        if(args[2]->getForm() != DF_SCALAR || args[2]->getCategory() != INTEGRAL) {
            throw IllegalArgumentException("dwtEx", "[level] should be a integral scalar.");
        }
        level = args[2]->getInt();
        if(level > 100 || level <= 0) {
            throw IllegalArgumentException("dwtEx", "[level] should be in [1, 100]");
        }
    }

    wave_object obj = wave_init(wavelet.c_str());
    int maxIter = log(static_cast<double>(dataLen) / (static_cast<double>(obj->filtlength) - 1.0)) / log(2.0);
    if(level > maxIter) {
        wave_free(obj);
        throw IllegalArgumentException("dwtEx", "All coefficients will experience boundary effects, you can use a longer signal or reduce the filter length or lower the level");
    }
    wt_object wt = wt_init(obj, "dwt", dataLen, level);
    //setDWTExtension(wt, "sym");
    //setWTConv(wt, "direct");
    dwt(wt, xn.data());

    ConstantSP ret = Util::createVector(DT_ANY, wt->lenlength - 1);
    int start = 0;
    for (int i = 0; i < wt->lenlength - 1; ++i) {
        int len = wt->length[i];
        VectorSP ele = Util::createVector(DT_DOUBLE, len);
        ele->setDouble(0, len, wt->output + start);
        ret->set(i, ele);
        start += len;
    }

    wave_free(obj);
    wt_free(wt);
    return ret;
}

//一维离散小波变换(DWT)
ConstantSP dwt1(Heap *heap, const ConstantSP &a, const ConstantSP &b)
{
    std::ignore = heap;
    if (!(a->getForm()==DF_VECTOR && a->isNumber() && (a->getCategory() == INTEGRAL || a->getCategory() == FLOATING) && a->size() > 0))
        throw IllegalArgumentException("dwt", "[X] should be a nonempty integrial or floating vector.");
    if (a->hasNull())
        throw IllegalArgumentException("dwt", "[X]" + NULL_ERROR);
    std::string wavelet = "db1";
    if(!b->isNothing()) {
        if (b->getType() != DT_STRING || b->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("dwt", "[wavelet] must be a string scalar");
        }
        wavelet = b->getString();
    }
    int dataLen = a->size(); //信号序列长度
    vector<double> FilterLD;
    vector<double> FilterHD;
    int filterLen; 
    if(wavelet == "db1") {
        FilterLD = {
            0.7071067811865475244008443621048490392848359376884740365883398,
            0.7071067811865475244008443621048490392848359376884740365883398}; //基于db1小波函数的滤波器低通序列
        FilterHD = {
                -0.7071067811865475244008443621048490392848359376884740365883398,
                0.7071067811865475244008443621048490392848359376884740365883398}; //基于db1小波函数的滤波器高通序列
        filterLen = 2;
    } else if(wavelet == "db4") {
        FilterLD = {
            -0.010597401785069032,
            0.0328830116668852,
            0.030841381835560764,
            -0.18703481171909309,
            -0.027983769416859854,
            0.6308807679298589,
            0.7148465705529157,
            0.2303778133088965}; //基于db4小波函数的滤波器低通序列
        FilterHD = {
            -0.2303778133088965,
            0.7148465705529157,
            -0.6308807679298589,
            -0.027983769416859854,
            0.18703481171909309,
            0.030841381835560764,
            -0.0328830116668852,
            -0.010597401785069032}; //基于db4小波函数的滤波器高通序列
        filterLen = 8;
    } else {
        throw IllegalArgumentException("dwt", R"([wavelet] must be 'db1' or 'db4')");
    }
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
ConstantSP idwt1(Heap *heap, const ConstantSP &a, const ConstantSP &b)
{
    std::ignore = heap;
    if (!(a->getForm()==DF_VECTOR && a->isNumber() && (a->getCategory() == INTEGRAL || a->getCategory() == FLOATING) && a->size() > 0))
        throw IllegalArgumentException("idwt", "[X] should be a nonempty integrial or floating vector.");
    if (!(b->getForm()==DF_VECTOR && b->isNumber() && (b->getCategory() == INTEGRAL || b->getCategory() == FLOATING) && b->size() > 0))
        throw IllegalArgumentException("idwt", "[Y] should be a nonempty integrial or floating vector.");
    if (a->size() != b->size())
        throw IllegalArgumentException("idwt", "two arguments should have the same size.");
    if(a->hasNull()) {
        throw IllegalArgumentException("idwt", "[X]" + NULL_ERROR);
    }
    if(b->hasNull()) {
        throw IllegalArgumentException("idwt", "[Y]" + NULL_ERROR);
    }
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

double get_scale(int n, int direction, const ConstantSP &s)
{
    std::string norm = "backward";
    if (!s->isScalar() || s->getType() != DT_STRING) {
        throw RuntimeException("[norm] should be a string scalar");
    }
    norm = s->getString();
    if (norm != "backward" && norm != "forward" && norm != "ortho") {
        throw RuntimeException("[norm] must be one of 'backward', 'forward', or 'ortho'.");
    }
    double scale{1.0};
    if (norm == "ortho") {
        scale /= sqrt(n);
    } else if ((direction == FFTW_FORWARD && norm == "forward") || (direction == FFTW_BACKWARD && norm == "backward")) {
        scale /= n;
    }
    return scale;
}

ConstantSP fft(vector<ConstantSP>& args, bool overwrite_x, int direction)
{
    if (!(args[0]->isVector() && (args[0]->getType() == DT_COMPLEX || args[0]->isNumber()) && args[0]->size() > 0)) {
        throw RuntimeException("[X] should be a nonempty vector");
    }
    VectorSP X = args[0];
    if (X->hasNull()) {
        throw RuntimeException("[X]" + NULL_ERROR);
    }
    auto in_size = X->size();
    auto out_size = in_size;
    if (args.size() > 1) {
        auto &n = args[1];
        if (!n->isScalar() || n->getType() != DT_INT || n->getInt() <= 0) {
            throw RuntimeException("[n] should be positive integer");
        }
        out_size = n->getInt();
    }
    if (X->getType() == DT_DOUBLE || in_size < out_size) {
        overwrite_x = false;
    }
    static ConstantSP default_scale = new String("backward");
    double scale = get_scale(out_size, direction, args.size() > 2 ? args[2] : default_scale);

    VectorSP res = Util::asContiguous(Util::createVector(DT_COMPLEX, out_size));
    if (X->getType() == DT_COMPLEX) {
        auto copy_size = std::min(in_size, out_size);
        res->fill(0, copy_size, X);
        res->fill(copy_size, out_size - copy_size, complex_zero);
    }
    auto *out = get_ptr_complex(res);
    if (X->getType() == DT_DOUBLE) {
        auto *in = get_ptr_double(X);
        if (in_size >= out_size) {
            fftw.execute(out_size, in, out);
        } else {
            fftw_memory<double> in_buf(out_size);
            memcpy(in_buf.get(), in, in_size * sizeof(double));
            memset(in_buf.get() + in_size, 0, (out_size - in_size) * sizeof(double));
            fftw.execute(out_size, in_buf.get(), out);
        }
        fft_scale_fill(out, out_size, scale);
    } else {
        fftw.execute(out_size, out, direction);
        fft_scale(out, out_size, scale);
        if (overwrite_x) {
            X->setBinary(0, out_size, complex_size, (unsigned char *)out);
        }
    }
    return res;
}

ConstantSP fft(Heap* heap, vector<ConstantSP>& args)
{
    to_double(heap, args[0]);
    return fft(args, false, FFTW_FORWARD);
}

ConstantSP fft1(Heap *heap, vector<ConstantSP> &args)
{
    to_double(heap, args[0]);
    return fft(args, true, FFTW_FORWARD);
}

ConstantSP ifft(Heap *heap, vector<ConstantSP> &args)
{
    to_complex(heap, args[0]);
    return fft(args, false, FFTW_BACKWARD);
}

ConstantSP ifft1(Heap *heap, vector<ConstantSP> &args)
{
    to_complex(heap, args[0]);
    return fft(args, true, FFTW_BACKWARD);
}

ConstantSP fft2(vector<ConstantSP> &args, bool overwrite_x, int direction)
{
    ConstantSP &a0 = args[0];
    if (!(a0->isMatrix() && (a0->getType() == DT_COMPLEX || a0->isNumber()) && a0->size() > 0)) {
        throw RuntimeException("[X] should be a nonempty matrix");
    }
    VectorSP X = args[0];
    if (X->hasNull()) {
        throw RuntimeException("[X]" + NULL_ERROR);
    }
    auto in_rows = X->rows();
    auto in_cols = X->columns();
    auto out_rows = in_rows;
    auto out_cols = in_cols;
    if (args.size() > 1) {
        if (!args[1]->isVector() || args[1]->getType() != DT_INT || args[1]->size() != 2 || args[1]->hasNull()) {
            throw RuntimeException("[s] should be a vector with 2 positive integer");
        }
        VectorSP shape = args[1];
        if (shape->getInt(0) <= 0 || shape->getInt(1) <= 0) {
            throw  RuntimeException("[s] should be a vector with 2 positive integer");
        }
        out_rows = shape->getInt(0);
        out_cols = shape->getInt(1);
    }
    auto out_size = out_rows * out_cols;
    if (X->getType() == DT_DOUBLE || in_rows < out_rows || in_cols < out_cols) {
        overwrite_x = false;
    }
    static ConstantSP default_scale = new String("backward");
    double scale = get_scale(out_size, direction, args.size() > 2 ? args[2] : default_scale);
    VectorSP res = Util::asContiguous(Util::createMatrix(DT_COMPLEX, out_cols, out_rows, out_cols));
    if (X->getType() == DT_COMPLEX) {
        auto copy_rows = std::min(in_rows, out_rows);
        auto copy_cols = std::min(in_cols, out_cols);
        for (int j = 0; j < copy_cols; ++j) {
            res->fill(j * out_rows, copy_rows, X, j * in_rows);
            res->fill((j * out_rows) + copy_rows, out_rows - copy_rows, complex_zero);
        }
        for (int j = copy_cols; j < out_cols; ++j) {
            res->fill(j * out_rows, out_rows, complex_zero);
        }
    }
    int buf_rows = std::min(in_rows, out_rows);
    int buf_cols = std::min(in_cols, out_cols);
    // Here we call fftw with a transposed input (swap cols and rows) as FFTW is row-major
    if (X->getType() == DT_DOUBLE) {
        fftw_memory<double> in(out_cols, out_rows);
        fftw_memory<fftw_complex> out(out_cols, out_rows);
        for (int j = 0; j < buf_cols; ++j) {
            X->getDouble(j * in_rows, buf_rows, in[j]);
        }
        fftw.execute_2d(in, out);
        fft_scale_fill(out, scale);
        for (int j = 0; j < out_cols; ++j) {
            res->setBinary(j * out_rows, out_rows, complex_size, (unsigned char *)out[j]);
        }
    } else {
        auto *out = get_ptr_complex(res);
        fftw.execute_2d(out_cols, out_rows, out, direction);
        fft_scale(out, out_size, scale);
        if (overwrite_x) {
            for (int j = 0; j < out_cols; ++j) {
                X->fill(j * in_rows, out_rows, res, j *out_rows);
            }
        }
    }
    return res;
}

ConstantSP fft2(Heap *heap, vector<ConstantSP> &args)
{
    to_double(heap, args[0]);
    return fft2(args, false, FFTW_FORWARD);
}

ConstantSP fft21(Heap *heap, vector<ConstantSP> &args)
{
    to_double(heap, args[0]);
    return fft2(args, true, FFTW_FORWARD);
}

ConstantSP ifft2(Heap *heap, vector<ConstantSP> &args)
{
    to_complex(heap, args[0]);
    return fft2(args, false, FFTW_BACKWARD);
}

ConstantSP ifft21(Heap *heap, vector<ConstantSP> &args)
{
    to_complex(heap, args[0]);
    return fft2(args, true, FFTW_BACKWARD);
}

ConstantSP secc(VectorSP &vsp, const VectorSP &msp, int k, std::vector<double> &mouts, std::vector<double> &weight)
{
    int lenData = vsp->size();
    int rows = msp->rows();
    int cols = msp->columns();
    int m = rows;
    //sumy2 = sqrt(sum(y.^2))
    vector<double> y(rows * cols, 0);
    msp->getDouble(0, rows * cols, y.data());
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
    vsp = Util::asContiguous(vsp);
    auto* v_ptr = vsp->getDoubleConst(0, lenData, nullptr);
    int step = k - m + 1;
    int sRows = (lenData + step - 1) / step;
    // alloc a bit more memory to avoid crashes caused by off-by-one error
    fftw_memory<double> s(sRows + 1, k);
    int s_row = 0;
    int s_col = 0;
    int vspIdx = 0;
    while (vspIdx < lenData && s_row < sRows) {
        s[s_row][s_col] = v_ptr[vspIdx++];
        ++s_col;
        if (s_col == k && vspIdx < lenData) {
            vspIdx -= (m - 1);
            s_col = 0;
            ++s_row;
        }
    }

    //sumx2_t=sqrt(movsum(s.^2,[m-1,0]));
    fftw_memory<double> sumx2(sRows + 1, k);
    for (int i = 0; i < sRows; i++) {
        double sum = 0;
        auto *s_i = s[i];
        for (int j = 0; j < m; j++) {
            sum += (s_i[j] * s_i[j]);
            sumx2[i][j] = sqrt(sum);
        }
        for (int j = m; j < k; j++) {
            sum += (s_i[j] * s_i[j] - s_i[j - m] * s_i[j - m]);
            sumx2[i][j] = sqrt(sum);
        }
    }

    //yz = y(end:-1:1,:);  reverse the templates
    //yz(m+1:k,:) = 0; padding with zero
    fftw_memory<double> yz(cols, k);
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            yz[i][j] = y[(i + 1) * rows - 1 - j];
        }
    }
    y.clear();

    //X=fft(s),Y=fft(y);
    int complex_len = k / 2 + 1;
    fftw_memory<fftw_complex> X(sRows, complex_len);
    fftw_memory<fftw_complex> Y(cols, complex_len);
    int fftLen = k;
    for (int i = 0; i < sRows; i++)
    {
        fftw.execute(fftLen, s[i], X[i]);
    }
    for (int i = 0; i < cols; i++)
    {
        fftw.execute(fftLen, yz[i], Y[i]);
    }

    VectorSP res = Util::createMatrix(DT_DOUBLE, cols, lenData - m + 1, cols);
    res = Util::asContiguous(res);
    fftw_memory<fftw_complex> Z(sRows, complex_len);
    fftw_memory<double> z(sRows, k);
    int colLen = lenData - m + 1;
    for (int c = 0; c < cols; c++)
    {
        int resIdxBase = c * colLen;
        double* colPtr = res->getDoubleBuffer(resIdxBase, colLen, nullptr);
        int m_out_c = (int)mouts[c];
        std::fill(colPtr, colPtr + std::min(m_out_c, colLen), 0.0);
        //Z = X.*Y(:,i);
        for (int i = 0; i < sRows; i++)
        {
            fftw_complex* Xi = X[i];
            fftw_complex* Yc = Y[c];
            fftw_complex* Zi = Z[i];
            for (int j = 0; j < complex_len; j++)
            {
                double x_re = Xi[j][0];
                double x_im = Xi[j][1];
                double y_re = Yc[j][0];
                double y_im = Yc[j][1];
                Zi[j][0] = x_re * y_re - x_im * y_im;
                Zi[j][1] = x_re * y_im + x_im * y_re;
            }
        }

        //z=ifft(Z)
        for (int i = 0; i < sRows; i++)
        {
            fftw.execute(fftLen, Z[i], z[i]);
        }
        // ccha=z(m:k,:)./(sumx2_t(m:k,:)*sumy2(i)); %devide by the normalization factor
        // ccc_sum(:,i)=weights(j,i).*([zeros(1,moveouts(j,i)),ccha(m:l_data-moveouts(j,i))])
        double w = weight[c];
        double inv_k = 1.0 / k;
        double inv_sumy2 = 1.0 / sumy2[c];
        double total_weight = w * inv_k * inv_sumy2;
        int currentColIdx = m_out_c;
        int writeLimit = std::min(colLen, (int)(lenData - mouts[c]));
        for (int i = 0; i < sRows && currentColIdx < writeLimit; i++) {
            for (int j = m - 1; j < k && currentColIdx < writeLimit; j++) {
                constexpr double zero { 1e-15 };
                if (sumx2[i][j] > zero) {
                    double val = z[i][j] * total_weight / sumx2[i][j];
                    colPtr[currentColIdx++] = std::isnan(val) ? DBL_NMIN : val;
                } else {
                    colPtr[currentColIdx++] = DBL_NMIN;
                }
            }
        }
        if (currentColIdx < colLen) {
            std::fill(colPtr + currentColIdx, colPtr + colLen, 0.0);
        }
    }
    return res;
}

ConstantSP secc(Heap *heap, vector<ConstantSP> &args)
{
    if (!args[0]->isVector() || !args[0]->isNumber() || args[0]->size() <= 0)
        throw IllegalArgumentException("secc", "[data] should be a nonempty vector");
    if (args[0]->hasNull()) {
        throw IllegalArgumentException("secc", "[data]" + NULL_ERROR);
    }
    VectorSP vsp = OperatorImp::asDouble(heap, args[0], placeholder);
    int lenData = vsp->size();
    if (!args[1]->isMatrix() || !args[1]->isNumber() || args[1]->size() <= 0)
        throw IllegalArgumentException("secc", "[template] should be a nonempty matrix");
    if (args[1]->hasNull()) {
        throw IllegalArgumentException("secc", "[template]" + NULL_ERROR);
    }
    VectorSP msp = OperatorImp::asDouble(heap, args[1], placeholder);
    int rows = msp->rows();
    int cols = msp->columns();
    if (lenData < rows)
        throw IllegalArgumentException("secc", "The length of data should not be less than the number of rows of templates");
    int m = rows;
    if (!args[2]->isScalar() || args[2]->getType() != DT_INT || args[2]->isNull())
        throw IllegalArgumentException("secc", "[k] should be a positive integer");
    if (args[2]->getInt() < 2 * m)
        throw IllegalArgumentException("secc", "For better performance, k is at least twice the number of rows of templates");
    int k = args[2]->getInt();
    vector<double> mouts(cols, 0);
    vector<double> weight(cols, 1);
    if (args.size() > 3)
    {
        if (!args[3]->isVector() || !args[3]->isNumber())
            throw IllegalArgumentException("secc", "[moveout] should be a nonempty vector");
        if (args[3]->size() != cols)
            throw IllegalArgumentException("secc", "The length of moveouts should be the same as the number of columns of templates");
        VectorSP moveout = args[3];
        if (moveout->hasNull()) {
            throw IllegalArgumentException("secc", "[moveout]" + NULL_ERROR);
        }
        moveout->getDouble(0, moveout->size(), &mouts[0]);
        double maxOuts = *std::max_element(mouts.begin(), mouts.end());
        for (size_t i = 0; i < mouts.size(); i++)
            mouts[i] = maxOuts - mouts[i];
    }
    if (args.size() > 4)
    {
        if (!args[4]->isVector() || !args[4]->isNumber() || args[4]->hasNull())
            throw IllegalArgumentException("secc", "[weight] should be a nonempty vector");
        if (args[4]->size() != cols)
            throw IllegalArgumentException("secc", "The length of weights should be the same as the number of columns of templates");
        VectorSP weights = args[4];
        if (weights->hasNull()) {
            throw IllegalArgumentException("secc", "[weight]" + NULL_ERROR);
        }
        weights->getDouble(0, cols, &weight[0]);
    }
    return secc(vsp, msp, k, mouts, weight);
}

ConstantSP absFuc(Heap *heap, vector<ConstantSP> &args){
    std::ignore = heap;
    if((!args[0]->isVector() && !args[0]->isScalar()) || args[0]->getType() != DT_COMPLEX || args[0]->hasNull()){
        throw IllegalArgumentException("abs", "data must be a nonempty complex vector or a nonempty complex scalar.");
    }
    ConstantSP data = args[0];
    if (data->hasNull()) {
        throw IllegalArgumentException("abs", "[data]" + NULL_ERROR);
    }
    if(args[0]->isScalar()){
        double buffer[2];
        data->getBinary(0, 1, 16, (unsigned char *)buffer);
        return new Double(sqrt(buffer[0] * buffer[0] + buffer[1] * buffer[1]));
    }
    int vSize = data->size();
    std::vector<double> dataBuffer(Util::BUF_SIZE * 2);
    std::vector<double> retBuffer(Util::BUF_SIZE * 2);
    
    int index = 0;
    VectorSP ret = Util::createVector(DT_DOUBLE, vSize, vSize);
    while(index < vSize){
        int subSize = std::min(vSize - index, Util::BUF_SIZE);
        const unsigned char* dataPtr = data->getBinaryConst(index, subSize, 16, (unsigned char *)dataBuffer.data());
        for (int i = 0; i < subSize; i++)
        {
            double x = ((double*)dataPtr)[i * 2];
            double y =  ((double*)dataPtr)[i * 2 + 1];
            retBuffer[i] = sqrt(x * x + y * y);
        }
        ret->setDouble(index, subSize, retBuffer.data());
        index += subSize;
    }
    return ret;
}

double calculateComplexAngle(double real, double imag, bool deg = false) {
    double theta = std::atan2(imag, real);
    if (deg) {
        theta *= 180.0 / MC_PI;
    }
    return theta;
}

double calculateRealAngle(double real, bool deg = false) {
    if (real < 0) {
        return deg ? 180.0 : MC_PI;
    }
    return 0.0;
}

ConstantSP angle(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    // X
    DATA_FORM form = args[0]->getForm();
    bool isComplex = args[0]->getType() == DT_COMPLEX;
    bool isReal = args[0]->isNumber();
    if ((form != DF_VECTOR && form != DF_SCALAR && form != DF_MATRIX) || (!isComplex && !isReal) ||
        args[0]->size() == 0) {
        throw IllegalArgumentException("angle",
                                       "X must be a nonempty complex or real numeric scalar, vector or matrix.");
    }
    if (args[0]->hasNull()) {
        throw RuntimeException("[X]" + NULL_ERROR);
    }
    // deg
    bool deg = false;
    if (args.size() > 1 && !args[1]->isNothing() && !args[1]->isNull()) {
        if (args[1]->getForm() != DF_SCALAR || args[1]->getType() != DT_BOOL) {
            throw IllegalArgumentException("angle", "[deg] should be a boolean scalar.");
        }
        deg = args[1]->getBool();
    }
    ConstantSP data = args[0];
    if (data->isScalar()) {
        if (isComplex) {
            std::array<double, 2> buffer;
            data->getBinary(0, 1, 16, (unsigned char *)buffer.data());
            double ret = calculateComplexAngle(buffer[0], buffer[1], deg);
            return new Double(ret);
        }
        return new Double(calculateRealAngle(data->getDouble(), deg));
    }
    // isVector or isMatrix
    int vSize = data->size();
    std::vector<double> retVec;
    retVec.reserve(vSize);
    int index = 0;
    if (isComplex) {
        constexpr int bytesPerComplex = 2 * sizeof(double);
        std::vector<unsigned char> dataBuffer(Util::BUF_SIZE * bytesPerComplex);
        double real = 0;
        double imag = 0;
        while (index < vSize) {
            int subSize = std::min(vSize - index, Util::BUF_SIZE);
            const unsigned char *dataPtr = data->getBinaryConst(index, subSize, bytesPerComplex, dataBuffer.data());
            for (int i = 0; i < subSize; i++) {
                const unsigned char *p = dataPtr + i * bytesPerComplex;
                memcpy(&real, p, sizeof(double));
                memcpy(&imag, p + sizeof(double), sizeof(double));
                double ang = calculateComplexAngle(real, imag, deg);
                retVec.push_back(ang);
            }
            index += subSize;
        }
    } else {
        std::vector<double> dataBuffer(Util::BUF_SIZE);
        while (index < vSize) {
            int subSize = std::min(vSize - index, Util::BUF_SIZE);
            data->getDouble(index, subSize, dataBuffer.data());
            for (int i = 0; i < subSize; i++) {
                retVec.push_back(calculateRealAngle(dataBuffer[i], deg));
            }
            index += subSize;
        }
    }
    VectorSP res;
    if (data->isMatrix()) {
        res = Util::createDoubleMatrix(data->columns(), data->rows());
    } else {
        res = Util::createVector(DT_DOUBLE, vSize, vSize);
    }
    res->setDouble(0, static_cast<int>(retVec.size()), retVec.data());
    return res;
}

ConstantSP mul(Heap *heap, vector<ConstantSP> &args){
    std::ignore = heap;
    if((!args[0]->isVector() && !args[0]->isScalar()) || args[0]->getType() != DT_COMPLEX){
        throw IllegalArgumentException("mul", "data must be a nonempty complex vector or a nonempty complex scalar.");
    }
    if(!args[1]->isNumber() || args[1]->getForm() != DF_SCALAR || args[1]->isNull())
        throw IllegalArgumentException("mul", "num should be a non-empty numeric scalar");
    ConstantSP data = args[0];
    if (data->hasNull()) {
        throw IllegalArgumentException("mul", "[data]" + NULL_ERROR);
    }
    double num = args[1]->getDouble();
    if(data->isScalar()){
        std::array<double, 2> buffer;
        data->getBinary(0, 1, 16, (unsigned char *)buffer.data());
        return new Complex(buffer[0] * num, buffer[1] * num);
    }
    int vSize = data->size();
    VectorSP res = Util::createVector(DT_COMPLEX, vSize, vSize);
    std::vector<double> dataBuffer(Util::BUF_SIZE * 2);
    std::vector<double> retBuffer(Util::BUF_SIZE * 2);
    int index = 0;
    while(index < vSize){
        int subSize = std::min(vSize - index, Util::BUF_SIZE);
        const unsigned char* dataPtr = data->getBinaryConst(index, subSize, 16, (unsigned char *)dataBuffer.data());
        for (int i = 0; i < subSize; i++)
        {
            double x = ((double*)dataPtr)[i * 2];
            double y = ((double*)dataPtr)[i * 2 + 1];
            retBuffer[i * 2] = x * num;
            retBuffer[i * 2 + 1] = y * num;
        }
        res->setBinary(index, subSize, 16, (unsigned char *)retBuffer.data());
        index += subSize;
    }
    return res;
}
