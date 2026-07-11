// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLogger.h"

extern "C" {

ddb::ConstantSP initialize(ddb::Heap *heap, argsT &args);
ddb::ConstantSP fftwConfig(ddb::Heap* heap, argsT &args);
ddb::ConstantSP dct(ddb::Heap *heap, const ddb::ConstantSP &a, const ddb::ConstantSP &b);  //离散余弦变换(DCT-II)
ddb::ConstantSP dst(ddb::Heap *heap, const ddb::ConstantSP &a, const ddb::ConstantSP &b);  //离散正弦变换(DST-I)
ddb::ConstantSP dwt1(ddb::Heap *heap, const ddb::ConstantSP &a, const ddb::ConstantSP &b);  //一维离散小波变换(DWT)
ddb::ConstantSP dwtEx(ddb::Heap *heap, argsT &args);      //一维离散小波变换(DWT)
ddb::ConstantSP idwt1(ddb::Heap *heap, const ddb::ConstantSP &a, const ddb::ConstantSP &b); //一维离散小波逆变换(IDWT)
ddb::ConstantSP dctMap(ddb::Heap *heap, argsT &args);
ddb::ConstantSP dctReduce(ddb::Heap *heap, const ddb::ConstantSP &mapRes1, const ddb::ConstantSP &mapRes2);
ddb::ConstantSP dctNumMap(ddb::Heap *heap, argsT &args);
ddb::ConstantSP dctNumReduce(ddb::Heap *heap, const ddb::ConstantSP &mapRes1, const ddb::ConstantSP &mapRes2);
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
ddb::ConstantSP angle(ddb::Heap *heap, argsT &args);

}
