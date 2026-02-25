// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include "CoreConcept.h"

extern "C" {

ConstantSP initialize(Heap *heap, argsT &args);
ConstantSP fftwConfig(Heap* heap, argsT &args);
ConstantSP dct(Heap *heap, const ConstantSP &a, const ConstantSP &b);  //离散余弦变换(DCT-II)
ConstantSP dst(Heap *heap, const ConstantSP &a, const ConstantSP &b);  //离散正弦变换(DST-I)
ConstantSP dwt1(Heap *heap, const ConstantSP &a, const ConstantSP &b);  //一维离散小波变换(DWT)
ConstantSP dwtEx(Heap *heap, argsT &args);      //一维离散小波变换(DWT)
ConstantSP idwt1(Heap *heap, const ConstantSP &a, const ConstantSP &b); //一维离散小波逆变换(IDWT)
ConstantSP dctMap(Heap *heap, argsT &args);
ConstantSP dctReduce(Heap *heap, const ConstantSP &mapRes1, const ConstantSP &mapRes2);
ConstantSP dctNumMap(Heap *heap, argsT &args);
ConstantSP dctNumReduce(Heap *heap, const ConstantSP &mapRes1, const ConstantSP &mapRes2);
ConstantSP dctParallel(Heap *heap, argsT &args);
ConstantSP fft(Heap *heap, argsT &args); //1-D fast discrete Fourier Transform.
ConstantSP fft1(Heap *heap, argsT &args);
ConstantSP ifft(Heap *heap, argsT &args); //1-D inverse fast discrete Fourier Transform.
ConstantSP ifft1(Heap *heap, argsT &args);
ConstantSP fft2(Heap *heap, argsT &args); //2-D fast discrete Fourier Transform.
ConstantSP fft21(Heap *heap, argsT &args);
ConstantSP ifft2(Heap *heap, argsT &args); //2-D inverse fast discrete Fourier Transform.
ConstantSP ifft21(Heap *heap, argsT &args);
ConstantSP secc(Heap *heap, argsT &args); //Super-Efficient Cross-Correlation of seismic waveforms
ConstantSP absFuc(Heap *heap, argsT &args);
ConstantSP mul(Heap *heap, argsT &args);

}
