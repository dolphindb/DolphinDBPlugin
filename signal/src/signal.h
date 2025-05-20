/*
 * signal.h
 *
 * Created on: Dec 1.2020
 *     Author: zkluo
 *
 */

#ifndef SIGNAL_H_
#define SIGNAL_H_
#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLoggerImp.h"

extern "C" {

ddb::ConstantSP dct(const ddb::ConstantSP &a, const ddb::ConstantSP &b);  //离散余弦变换(DCT-II)
ddb::ConstantSP dst(const ddb::ConstantSP &a, const ddb::ConstantSP &b);  //离散正弦变换(DST-I)
ddb::ConstantSP dwt(const ddb::ConstantSP &a, const ddb::ConstantSP &b);  //一维离散小波变换(DWT)
ddb::ConstantSP idwt(const ddb::ConstantSP &a, const ddb::ConstantSP &b); //一维离散小波逆变换(IDWT)
ddb::ConstantSP dctMap(ddb::Heap *heap, argsT &args);
ddb::ConstantSP dctReduce(const ddb::ConstantSP &mapRes1, const ddb::ConstantSP &mapRes2);
ddb::ConstantSP dctNumMap(ddb::Heap *heap, argsT &args);
ddb::ConstantSP dctNumReduce(const ddb::ConstantSP &mapRes1, const ddb::ConstantSP &mapRes2);
ddb::ConstantSP dctParallel(ddb::Heap *heap, argsT &args);
ddb::ConstantSP fft(ddb::Heap *heap, argsT &args); //1-D fast discrete Fourier Transform.
ddb::ConstantSP fft1(ddb::Heap *heap, argsT &args);
ddb::ConstantSP ifft(ddb::Heap *heap, argsT &args); //1-D inverse fast discrete Fourier Transform.
ddb::ConstantSP ifft1(ddb::Heap *heap, argsT &args);
ddb::ConstantSP fft2(ddb::Heap *heap, argsT &args); //2-D fast discrete Fourier Transform.
ddb::ConstantSP fft21(ddb::Heap *heap, argsT &args);
ddb::ConstantSP ifft2(ddb::Heap *heap, argsT &args); //2-D inverse fast discrete Fourier Transform.
ddb::ConstantSP ifft21(ddb::Heap *heap, argsT &args);
ddb::ConstantSP secc(ddb::Heap *heap, argsT &args); //Super-Efficient Cross-Correlation of seismic waveforms
ddb::ConstantSP absFuc(ddb::Heap *heap, argsT &args);
ddb::ConstantSP mul(ddb::Heap *heap, argsT &args);

}

#endif /* SIGNAL_H_ */
