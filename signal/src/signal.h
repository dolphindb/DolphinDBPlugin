/*
 * signal.h
 *
 * Created on: Dec 1.2020
 *     Author: zkluo
 *
 */

#ifndef SIGNAL_H_
#define SIGNAL_H_
#include "CoreConcept.h"

extern "C" ConstantSP dct(const ConstantSP &a, const ConstantSP &b);  //离散余弦变换(DCT-II)
extern "C" ConstantSP dst(const ConstantSP &a, const ConstantSP &b);  //离散正弦变换(DST-I)
extern "C" ConstantSP dwt(const ConstantSP &a, const ConstantSP &b);  //一维离散小波变换(DWT)
extern "C" ConstantSP idwt(const ConstantSP &a, const ConstantSP &b); //一维离散小波逆变换(IDWT)
extern "C" ConstantSP dctMap(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP dctReduce(const ConstantSP &mapRes1, const ConstantSP &mapRes2);
extern "C" ConstantSP dctNumMap(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP dctNumReduce(const ConstantSP &mapRes1, const ConstantSP &mapRes2);
extern "C" ConstantSP dctParallel(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP fft(Heap *heap, vector<ConstantSP> &args); //1-D fast discrete Fourier Transform.
extern "C" ConstantSP fft1(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP ifft(Heap *heap, vector<ConstantSP> &args); //1-D inverse fast discrete Fourier Transform.
extern "C" ConstantSP ifft1(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP fft2(Heap *heap, vector<ConstantSP> &args); //2-D fast discrete Fourier Transform.
extern "C" ConstantSP fft21(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP ifft2(Heap *heap, vector<ConstantSP> &args); //2-D inverse fast discrete Fourier Transform.
extern "C" ConstantSP ifft21(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP secc(Heap *heap, vector<ConstantSP> &args); //Super-Efficient Cross-Correlation of seismic waveforms
#endif /* SIGNAL_H_ */
