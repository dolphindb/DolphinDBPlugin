/*
  * OperatorImp.h
 *
 *  Created on: Oct 18, 2013
 *      Author: dzhou
 */

#ifndef OPERATORIMP_H_
#define OPERATORIMP_H_

#include "CoreConcept.h"
#include "DolphinClass.h"
#include "Exceptions.h"
#include "Types.h"

namespace ddb {

#include <functional>

namespace OperatorImp{

//functions for other databases
ConstantSP SWORDFISH_API oracleConcat(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API oracleRowNumber(Heap* heap, vector<ConstantSP>& arguments);

//no argument function
ConstantSP SWORDFISH_API benchmark(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API now(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API today(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API memory(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API getHomeDir(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API getNodeAlias(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API getNodeHost(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API getNodePort(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API getOS(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API getOSBit(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API version(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API getEnv(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API getRequiredAPIVersion(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API getLicenseExpiration(const ConstantSP& a, const ConstantSP& b);

ConstantSP SWORDFISH_API constantDesc(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API brief(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API snippet(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API syntax(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API memSize(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API compress(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API decompress(const ConstantSP& a, const ConstantSP& b);

ConstantSP SWORDFISH_API func(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API add(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API sub(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API multiply(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API divide(const ConstantSP& a, const ConstantSP& b);
/** Equivalent to: '/' in Python; and '\' (`ratio`) in DolphinDB. */
ConstantSP SWORDFISH_API dividePython(const ConstantSP& a, const ConstantSP& b);
/** Equivalent to: '//' in Python. */
ConstantSP SWORDFISH_API floorDivide(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ratio(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API mod(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API modPython(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API power(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API minIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API maxIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API lt(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API le(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API gt(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ge(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API between(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ltIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API leIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API gtIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API geIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API betweenIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ltNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API leNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API gtNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API geNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API betweenNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ne(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API equal(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API logicAnd(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API logicOr(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API logicOrIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API logicXor(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API bitAnd(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API bitOr(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API bitXor(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API leftShift(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API rightShift(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API symmetricDifference(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API hashBucket(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API distance(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ifNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ifValid(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API notIn(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API notBetween(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API notLike(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API nullIf(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API neAny(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API eqAll(const ConstantSP& a, const ConstantSP& b);

ConstantSP SWORDFISH_API at(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API at(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API eachAt(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API eachAt(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API mask(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API member(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API rand(const ConstantSP& a,const ConstantSP& b);
ConstantSP SWORDFISH_API polynomial(const ConstantSP& a,const ConstantSP& b);
ConstantSP SWORDFISH_API take(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API stretch(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API drop(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API til(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API seq(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API pair(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API in(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API find(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API binsrch(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asof(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cut(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cast(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API reshape(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API join(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API concatMatrix(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API subarray(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API subtuple(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API arrayVector(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API slicedTable(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API head(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API tail(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API indexedSeries(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API enlist(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API lowerBound(const ConstantSP& a, const ConstantSP& b);

ConstantSP SWORDFISH_API linearTimeTrend(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API kama(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API wilder(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ema(Heap *pHeap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sma(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API wma(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API dema(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API tema(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API trima(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API t3(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API gema(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ma(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sessionWindow(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API talibNull(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mcount(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mfirst(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mfirstNot(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mifirstNot(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mlast(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mlastNot(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API milastNot(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mmed(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mavg(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mmin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mmax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mimin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mimax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API miminLast(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API mimaxLast(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API msum(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API msum2(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mprod(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mstd(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mvar(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mstdp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mvarp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mskew(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mkurtosis(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mrank(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mcorr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mcovar(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mbeta(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mwsum(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mwavg(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mslr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mpercentile(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mmse(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mmad(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API matImin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API matImax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mmaxPositiveStreak(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmcount(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmfirst(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmlast(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmsum(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmsum2(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmmed(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmpercentile(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmavg(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmstd(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmvar(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmstdp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmvarp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmskew(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmkurtosis(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmprod(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmmin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmmax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmrank(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmcorr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmcovar(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmbeta(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmwsum(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmwavg(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmove(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmatImin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmatImax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ftmsum(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ftmmax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ftmmin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ftmprod(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ftmlast(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ftmwavg(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumskewTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumkurtosisTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumsumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumavgTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumvarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumvarpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumstdTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumstdpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumcorrTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumbetaTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumcovarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumwsumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mskewTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mkurtosisTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API msumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mavgTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mvarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mvarpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mstdTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mstdpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mcorrTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mbetaTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mcovarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mwsumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mpercentileTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmskewTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmkurtosisTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmsumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmavgTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmvarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmvarpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmstdTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmstdpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmcorrTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmbetaTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmcovarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmwsumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmovingTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API movingTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API movingTopNIndex(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API movingWindowIndex(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API movingWindowData(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmovingWindowIndex(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmovingWindowData(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ewmMean(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ewmVar(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ewmStd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ewmCorr(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ewmCov(Heap* heap, vector<ConstantSP>& arguments);

//vector manipulation
ConstantSP SWORDFISH_API rowNo(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API move(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API next(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API prev(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API lead(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API lag(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API nextState(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API prevState(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API reverse(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API shuffle(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isortUnique(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isort(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API sort(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API distinct(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API groups(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API keys(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API values(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API iterator(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API row(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API column(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API columnNames(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API rowNames(const ConstantSP& a, const ConstantSP& b);

//aggregate function
ConstantSP SWORDFISH_API isOrderedDict(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isIndexedMatrix(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isIndexedSeries(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isColumnarTuple(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isVoid(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isNothing(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API type(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API typestr(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API category(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API form(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API hasNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API size(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API capacity(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API count(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API countNanInf(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API contextCount(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API rows(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API columns(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API shape(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API tupleSum(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API sum(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API contextSum(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API sum2(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API sum3(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API sum4(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API contextSum2(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API mad(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API sem(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API avg(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API var(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API std(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API varp(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API stdp(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API skew(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API kurtosis(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API prod(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API max(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API min(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API imax(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API imin(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API imaxLast(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API iminLast(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API first(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API last(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API lastNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API firstNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ilastNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ifirstNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API med(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API mode(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API stat(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API maxPositiveStreak(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API allTrue(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API anyTrue(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API oddTrue(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API maxDrawdown(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API rms(const ConstantSP& a, const ConstantSP& b);

ConstantSP SWORDFISH_API isSorted(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API atIMax(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API atIMin(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API wavg(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API wsum(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API dot(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API euclidean(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API jaccard(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API tanimoto(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API correlation(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API spearmanr(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API kendall(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API mutualInformation(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API covariance(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API beta(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API searchK(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API difference(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API crossStat(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isMonotonic(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isMonotonicIncreasing(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isMonotonicDecreasing(const ConstantSP& a, const ConstantSP& b);

ConstantSP SWORDFISH_API eye(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API diag(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API naiveMulti(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API strassenMulti(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API transpose(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API inverse(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API pinverse(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API det(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API solve(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cholesky(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API lu(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API schur(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API triu(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API tril(const ConstantSP& a, const ConstantSP& b);

ConstantSP SWORDFISH_API writeBytes(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API readBytes(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API readLine(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API readLines(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API toJson(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API toStdJson(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API fromJson(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API fromStdJson(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API crc32(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API md5(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API wideTable(const ConstantSP& a, const ConstantSP& b);

//unary temporal functions
ConstantSP SWORDFISH_API year(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API month(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API date(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API hour(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API minute(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API second(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API time(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API datetime(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API datehour(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API timestamp(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API nanotime(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API nanotimestamp(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API weekday(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API dayOfWeek(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API localtime(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API gmtime(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API concatDateTime(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API dayOfYear(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API dayOfMonth(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API quarterOfYear(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API monthOfYear(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API weekOfYear(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API hourOfDay(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API minuteOfHour(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API secondOfMinute(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API millisecond(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API microsecond(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API nanosecond(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isMonthStart(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isMonthEnd(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isQuarterStart(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isQuarterEnd(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isYearStart(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isYearEnd(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isLeapYear(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API daysInMonth(const ConstantSP& a, const ConstantSP& b);

//data type conversion
ConstantSP SWORDFISH_API decodeShortGenomeSeq(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API encodeShortGenomeSeq(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API genShortGenomeSeq(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API symbolCode(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asBool(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asChar(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asShort(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asInt(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asLong(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asFloat(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asDouble(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asSymbol(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asString(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asBlob(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asUuid(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asIPAddr(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asInt128(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asComplex(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asPoint(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asDuration(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API makeDuration(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API highDouble(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API lowDouble(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API highLong(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API lowLong(const ConstantSP& a, const ConstantSP& b);

/**
 * @brief cast other object to Decimal
 * 
 * @param a object to be casted
 * @param b scale
 */
ConstantSP SWORDFISH_API asDecimal32(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asDecimal64(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asDecimal128(const ConstantSP& a, const ConstantSP& b);

ConstantSP SWORDFISH_API decimalMultiply(Heap *heap, vector<ConstantSP> &arguments);

//unary math functions
ConstantSP SWORDFISH_API signum(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API signbit(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API nullFlag(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isValid(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isNanInf(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API nullFill(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API nanInfFill(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API where(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API logicNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API bitNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API round(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API roundPandas(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API decimalFormat(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API temporalFormat(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API format(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API temporalParse(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ceil(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API floor(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API neg(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API abs(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API reciprocal(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API sin(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cos(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API tan(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asin(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API acos(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API atan(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API sinh(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cosh(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API tanh(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asinh(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API acosh(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API atanh(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API deg2rad(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API rad2deg(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API sqrt(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API exp(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API log(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API exp2(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API expm1(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API log2(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API log10(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API log1p(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cbrt(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API square(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API flatten(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API copy(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API asis(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API deepCopy(const ConstantSP& a, const ConstantSP& b);

ConstantSP SWORDFISH_API cumPositiveStreak(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumsum(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumsum2(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumsum3(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumsum4(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumprod(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cummin(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cummax(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumcount(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumavg(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumstd(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumvar(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumstdp(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumvarp(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumpercentile(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumrank(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cumdenseRank(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cummed(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumcorr(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumcovar(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumbeta(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumwsum(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumwavg(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumfirstNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumlastNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumifirstNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumilastNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API cumnunique(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API dynamicGroupCumsum(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API dynamicGroupCumcount(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cummdd(const ConstantSP& a, const ConstantSP& b);

ConstantSP SWORDFISH_API deltas(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ratios(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API bar(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API volumeBar(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API zscore(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API demean(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API percentChange(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API segment(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API topRange(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API lowRange(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isPeak(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isValley(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API sumbars(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API valueChanged(const ConstantSP& a, const ConstantSP& b);

//string functions
ConstantSP SWORDFISH_API isUpper(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isLower(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isTitle(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isSpace(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isAlpha(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isNumeric(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isAlNum(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API isDigit(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API capitalize(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API title(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API swapCase(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API like(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ilike(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API trim(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API strip(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API strlen(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API strlenu(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API wc(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API upper(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API lower(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API repeat(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API left(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API right(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API ltrim(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API rtrim(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API startsWith(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API endsWith(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API strpos(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API charAt(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API split(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API concat(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API hex(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API binary(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API initcap(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API base64Encode(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API base64Decode(const ConstantSP& a, const ConstantSP& b);
ConstantSP SWORDFISH_API strReplace(Heap * heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API substr(Heap * heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API substru(Heap * heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API lpad(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API rpad(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API regexReplace(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API regexFind(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API regexCount(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API regexFindStr(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API convertEncode(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API toUTF8(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API fromUTF8(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API makeKey(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API makeSortedKey(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API hmac(Heap* heap, vector<ConstantSP>& arguments);

//adverb functions
ConstantSP each(const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP loop(const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP eachPre(const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP eachPost(const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP eachRight(const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP eachLeft(const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP cross(const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP reduce(const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP accumulate(const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP byRow(const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP byColumn(const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);

ConstantSP eachFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP loopFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP eachPreFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP eachPostFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP eachRightFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP eachLeftFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP crossFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP reduceFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP accumulateFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP groupFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP contextFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP byRowFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);
ConstantSP byColumnFD(Heap* heap, const ConstantSP& a, const ConstantSP& b,const FunctionDefSP& optr, int assembleRule);

//system functions
ConstantSP SWORDFISH_API quantile(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API quantileSeries(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API percentile(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API percentileRank(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API firstHit(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ifirstHit(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API geoWithin(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rdp(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API zigzag(Heap *heap, vector<ConstantSP> &arguments);

ConstantSP SWORDFISH_API sqlCol(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sqlColAlias(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sqlTuple(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API expression(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API binaryExpr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API unifiedExpr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API makeCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API makeUnifiedCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API makeObjCall(Heap*, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API makeUnifiedObjCall(Heap*, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API isInstanceOf(Heap*, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API setAttr(Heap*, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sql(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sqlUpdate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sqlDelete(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API evaluate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API objectComponent(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API objectType(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API getChunkPath(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sqlDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API textFilesDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API textChunkDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cacheDSNow(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API clearDSCacheNow(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API saveDSToDB(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cacheDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API clearDSCache(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API transDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API scheduleDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mrDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API imrDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API pipeline(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API readTableFromFileSegment(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API readTextFromFileSegment(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API readTabletChunk(Heap* heap, vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API partial(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API piecewise(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API compose(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API assemble(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API unifiedCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API delayedFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API partitionFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API dynamicFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API pdynamicFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ptableCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API loopFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ploopFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API eachFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ceachFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API peachFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API crossFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API pcrossFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API eachLeftFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API eachRightFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API eachPreFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API eachPostFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API rowReduceFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API reduceFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API accumulateFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API groupFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API rowGroupFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API contextFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API segmentFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API pivotFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API movingFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API rollingFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API allFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API anyFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API fillopFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API byRowFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API byColumnFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API talibFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API compareFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tmovingFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API aggrTopNFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API windowFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API twindowFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API adverbFuncCall(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API pdfF(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API pdfChiSquare(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API pdfNormal(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API cdfStudent(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfF(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfBeta(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfGamma(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfChiSquare(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfNormal(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfExp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfUniform(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfWeibull(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfZipf(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfLogistic(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfBinomial(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfPoisson(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cdfKolmogorov(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API invStudent(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invF(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invBeta(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invGamma(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invChiSquare(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invNormal(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invExp(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invUniform(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invWeibull(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invZipf(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invLogistic(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invBinomial(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invPoisson(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API invKolmogorov(Heap *heap, vector<ConstantSP> &arguments);

ConstantSP SWORDFISH_API randStudent(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randF(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randBeta(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randGamma(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randChiSquare(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randNormal(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randMultivariateNormal(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randExp(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randUniform(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randWeibull(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randZipf(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randLogistic(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randBinomial(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randPoisson(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randKolmogorov(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API randDiscrete(Heap *heap, vector<ConstantSP> &arguments);

ConstantSP SWORDFISH_API bondDuration(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API bondConvexity(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API bondDirtyPrice(Heap *heap, vector<ConstantSP> &arguments);

ConstantSP SWORDFISH_API rowMin(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowMax(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowImin(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowImax(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowIminLast(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowImaxLast(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowAt(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowSum(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowSum2(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowCount(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowSize(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowStd(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowStdp(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowVar(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowVarp(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowSkew(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowKurtosis(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowAvg(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowAnd(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowOr(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowXor(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowProd(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowMed(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowRank(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowDenseRank(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowRatios(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowDeltas(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowPercentChange(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowZscore(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowDemean(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowCumsum(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowCumwsum(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowCumprod(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowCummax(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowCummin(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowMove(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowNext(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowPrev(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowCorr(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowCovar(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowBeta(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowWsum(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowWavg(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowTanimoto(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowJaccard(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowEuclidean(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowDot(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API rowAlign(Heap *heap, vector<ConstantSP> &arguments);

ConstantSP SWORDFISH_API coalesce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API panel(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API unionAll(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API conditionalFilter(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API trueRange(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API isortTop(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API partition(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sample(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API freq(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API normal(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API digitize(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API bucket(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API bucketCount(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cutPoints(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API wcovariance(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ns(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API unpivot(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ungroup(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API rank(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API denseRank(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API isortInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API joinInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API removeHead(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API removeTail(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API iif(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API iterate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API conditionalIterate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API stateIterate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API replace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ffill(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API bfill(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API lfill(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ffillInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API bfillInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API lfillInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API winsorize(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API winsorizeInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API clip(Heap *heap, vector<ConstantSP> &args);
ConstantSP SWORDFISH_API clipInplace(Heap *heap, vector<ConstantSP> &args);
ConstantSP SWORDFISH_API locate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API slice(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sliceByKey(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cell(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cells(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API array(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API bigarray(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API matrix(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API repmat(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tensor(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API set(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API dictionary(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API syncDictionary(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API table(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API keyedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API latestKeyedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API indexedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API latestIndexedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API streamTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API keyedStreamTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API latestKeyedStreamTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API mvccTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API cachedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API database(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API addValuePartitions(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API addRangePartitions(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tableInsert(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getTablet(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API svd(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API qr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API asfreq(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API resample(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API freqSeq(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API regroup(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API align(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API merge(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API eqFloat(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API oneHot(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API fixedLengthArrayVector(Heap* heap, vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API exists(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API existsDatabase(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API existsTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API existsPartition(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API listTables(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getTables(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API parseJsonTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API jsonExtract(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API extractTextSchema(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API loadText(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ploadText(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API loadTextEx(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API loadTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API renameTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API loadColumn(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API loadStreamTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API loadMvccTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API createPartitionedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API createTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API createIMOLTPTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API loadTableBySQL(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API bigSQL(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API loadRecord(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API coevent(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API covarMatrix(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API corrMatrix(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API objAddr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API objects(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API objByName(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API funcByName(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API defined(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API functions(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API files(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API schema(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API objCall(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API unifiedObjCall(Heap *heap, vector<ConstantSP> &arguments);

ConstantSP SWORDFISH_API refCount(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API xdb(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API pnodeRun(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API remoteRun(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API remoteUrgentRun(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API remoteRunWithCompression(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API remoteRunCompatible(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API rpc(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API socket(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API file(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API writeObject(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API readObject(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API write(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API writeLine(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API writeLines(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API read(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API readLinesInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API seek(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API writeRecord(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API readRecordInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API clearCache(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getDiskIOStat(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getRunningQueries(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getCompletedQueries(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getSessionMemoryStat(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getMemoryStat(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API transaction(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API checksum(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API toCharArray(Heap* heap, vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API transFreq(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API temporalSeq(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API convertTZ(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API temporalAdd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API temporalDeltas(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API temporalDiff(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API dailyAlignedBar(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API businessDay(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API monthBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API monthEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API businessMonthBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API businessMonthEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API quarterBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API quarterEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API businessQuarterBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API businessQuarterEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API yearBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API yearEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API businessYearBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API businessYearEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API semiMonthBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API semiMonthEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API weekOfMonth(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API lastWeekOfMonth(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API weekBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API weekEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API fy5253(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API fy5253Quarter(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API marketHoliday(Heap* heap, vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API dropna(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API fillInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API nullFillInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API shuffleInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API replaceInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sortInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API append(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API upsert(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API tableUpsert(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP SWORDFISH_API pop(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API erase(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API update(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API sortBy(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ajInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API ljInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API clear(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API rename(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API dictUpdate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API reorderColumns(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API replaceColumn(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API dropColumns(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API setIndexedMatrix(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API setIndexedSeries(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API setColumnarTuple(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API appendTuple(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API memberModify(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API eqObj(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API license(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API updateLicense(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API genLicenseAuthorization(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API readLicenseAuthorization(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API shell(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API loadPlugin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API loadPatch(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API chunkMeta(Heap* heap, vector<ConstantSP>& arguments);

ConstantSP SWORDFISH_API submitJob(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API submitJobEx(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API submitJobEx2(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getRecentJobs(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getJobStatus(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getJobReturn(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getJobMessage(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getConsoleJobs(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API evalTimer(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API getPartitionDataFromDS(Heap* heap, vector<ConstantSP>& arguments);

//reduce and running functions for map-reduce of aggregate functions
ConstantSP avgReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP stdReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP stdpReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP varReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP varpReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP corrReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP covarReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP skewReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP skewReduce2(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP kurtosisReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP kurtosisReduce2(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP stdRunning(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP stdpRunning(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP varRunning(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP varpRunning(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP corrRunning(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP covarRunning(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP skewRunning(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP skewRunning2(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP kurtosisRunning(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP kurtosisRunning2(Heap* heap,vector<ConstantSP>& arguments);

//system procedures
void sleep(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API print(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API writeLog(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API run(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API runScript(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API test(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API testCommitFailure(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API saveText(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API saveTable(Heap* heap,vector<ConstantSP>& arguments);
void savePartition(Heap* heap,vector<ConstantSP>& arguments);
void saveDualPartition(Heap* heap,vector<ConstantSP>& arguments);
void enableActivePartition(Heap* heap,vector<ConstantSP>& arguments);
void disableActivePartition(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API dropPartition(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API dropTable(Heap* heap,vector<ConstantSP>& arguments);
void saveDatabase(Heap* heap,vector<ConstantSP>& arguments);
void dropDatabase(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API addColumn(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API dropColumn(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API setColumnComment(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API  setTableComment(Heap* heap,vector<ConstantSP>& arguments);
void setTableSensitiveColumn(Heap* heap,vector<ConstantSP>& arguments);
void setRetentionPolicy(Heap* heap,vector<ConstantSP>& arguments);
void setRemoveSpecialChar(Heap* heap,vector<ConstantSP>& arguments);
void setDatabaseForClusterReplication(Heap* heap,vector<ConstantSP>& arguments);
void setDatabaseClusterReplicationExecutionSet(Heap* heap,vector<ConstantSP>& arguments);
void setDatabaseOwner(Heap* heap,vector<ConstantSP>& arguments);
void setAtomicLevel(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API setRandomSeed(Heap* heap,vector<ConstantSP>& arguments);
void setMvccColumnDefaultValue(Heap* heap,vector<ConstantSP>& arguments);
void setMvccColumnNullability(Heap* heap,vector<ConstantSP>& arguments);
void close(Heap* heap,vector<ConstantSP>& arguments);
void mkdir(Heap* heap,vector<ConstantSP>& arguments);
void rmdir(Heap* heap,vector<ConstantSP>& arguments);
void rm(Heap* heap,vector<ConstantSP>& arguments);
void setSystem(Heap* heap,vector<ConstantSP>& arguments);
void setSQLStandard(Heap* heap,vector<ConstantSP>& arguments);
void addFunctionalViewInternal(Heap* heap,vector<ConstantSP>& arguments);
void updateFunctionViewOnDatanode(Heap* heap,vector<ConstantSP>& arguments);
void dropFunctionalViewInternal(Heap* heap,vector<ConstantSP>& arguments);
void SWORDFISH_API undef(Heap* heap,vector<ConstantSP>& arguments);
void plot(Heap* heap,vector<ConstantSP>& arguments);
void plotHist(Heap* heap,vector<ConstantSP>& arguments);
void generateLicense(Heap* heap, vector<ConstantSP>& arguments);
void generatePluginSign(Heap* heap, vector<ConstantSP>& arguments);
void SWORDFISH_API clearAllCache(Heap* heap, vector<ConstantSP>& arguments);
void clearCachedDatabase(Heap* heap, vector<ConstantSP>& arguments);
void clearDatabaseDomain(Heap* heap, vector<ConstantSP>& arguments);
void clearCachedSegment(Heap* heap, vector<ConstantSP>& arguments);
void checkMemoryUsageWarning(Heap* heap, vector<ConstantSP>& arguments);
void startHeapSample(Heap* heap, vector<ConstantSP> &arguments);
void stopHeapSample(Heap* heap, vector<ConstantSP> &arguments);
void dumpHeapSample(Heap* heap, vector<ConstantSP> &arguments);
void fflush(Heap* heap, vector<ConstantSP>& arguments);
void notifyWriteTask(Heap* heap, vector<ConstantSP>& arguments);
void reportConcurrentTasks(Heap* heap, vector<ConstantSP>& arguments);
void checkCtrAliveForConcurrentWrite(Heap* heap, vector<ConstantSP>& arguments);
void clearAllConcurrentWriteTasks(Heap* heap, vector<ConstantSP>& arguments);

void closeTransaction(Heap* heap, vector<ConstantSP>& arguments);
void commitTransaction(Heap* heap, vector<ConstantSP>& arguments);
void rollbackTransaction(Heap* heap, vector<ConstantSP>& arguments);
void cancelJob(Heap* heap, vector<ConstantSP>& arguments);
void cancelConsoleJob(Heap* heap, vector<ConstantSP>& arguments);
void cancelConsoleJobTask(Heap* heap, vector<ConstantSP>& arguments);
void addChunkMetaOnMaster(Heap* heap, vector<ConstantSP>& arguments);
void deleteChunkMetaOnMaster(Heap* heap, vector<ConstantSP>& arguments);
void saveColumn(Heap* heap, vector<ConstantSP>& arguments);
void updateTabletChunk(Heap* heap, vector<ConstantSP>& arguments);
void loadModule(Heap* heap, vector<ConstantSP>& arguments);
void saveModule(Heap* heap, vector<ConstantSP>& arguments);
void encryptModule(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP SWORDFISH_API uploadModule(Heap* heap, vector<ConstantSP>& arguments);
void appendMsg(Heap* heap,vector<ConstantSP>& arguments);
void generateMachineFingerprint(Heap *heap, vector<ConstantSP> &args);
void composeMachineFingerprint(Heap *heap, vector<ConstantSP> &args);

ConstantSP doubleSortPrefixJoin(TableSP left, TableSP right, ConstantSP leftMatchingColNames, ConstantSP rightMatchingColNames, ConstantSP leftFilter,ConstantSP rightFilter);
ConstantSP doubleSortEqualJoin(TableSP left, TableSP right, ConstantSP leftMatchingColNames, ConstantSP rightMatchingColNames, ConstantSP leftFilter,ConstantSP rightFilter, bool sortJoinedTable);
ConstantSP rightSortEqualJoin(TableSP left, TableSP right, ConstantSP leftMatchingColNames, ConstantSP rightMatchingColNames, ConstantSP leftFilter,ConstantSP rightFilter);
ConstantSP hashEqualJoin(TableSP left, TableSP right, ConstantSP leftMatchingColNames, ConstantSP rightMatchingColNames, ConstantSP leftFilter, ConstantSP rightFilter);
ConstantSP doubleSortLeftJoin(TableSP left, TableSP right,ConstantSP leftMatchingColNames, ConstantSP rightMatchingColNames, ConstantSP leftMatch, ConstantSP rightMatch, ConstantSP leftFilter,ConstantSP rightFilter);
ConstantSP doubleSortLeftSemiJoin(TableSP left, TableSP right,ConstantSP leftMatchingColNames, ConstantSP rightMatchingColNames, ConstantSP leftMatch, ConstantSP rightMatch, ConstantSP leftFilter,ConstantSP rightFilter, bool sortJoinedTable);
ConstantSP rightSortLeftSemiJoin(TableSP left, TableSP right, ConstantSP leftMatchingColNames, ConstantSP rightMatchingColNames, ConstantSP leftMatch, ConstantSP rightMatch, ConstantSP leftFilter,ConstantSP rightFilter);
ConstantSP doubleSortAsofJoin(TableSP left, TableSP right, ConstantSP leftMatchingColNames, ConstantSP rightMatchingColNames, ConstantSP leftFilter,ConstantSP rightFilter, bool sortJoinedTable);
ConstantSP hashMultipleColumnLeftSemiJoin(TableSP left, TableSP right, ConstantSP leftMatchingColNames, ConstantSP rightMatchingColNames, ConstantSP leftMatch, ConstantSP rightMatch, ConstantSP leftFilter,ConstantSP rightFilter);
ConstantSP hashLeftSemiJoin(TableSP left, TableSP right, const string& leftMatchingColName, const string& rightMatchingColName, ConstantSP leftMatch, ConstantSP rightMatch, ConstantSP leftFilter,ConstantSP rightFilter, bool sortJoinedTable);
ConstantSP hashLeftJoin(TableSP left, TableSP right, vector<int>& leftKeys, vector<int>& rightKeys, ConstantSP leftMatch, ConstantSP leftFilter);
ConstantSP doubleSortFullJoin(TableSP left, TableSP right, ConstantSP leftMatchingColNames, ConstantSP rightMatchingColNames, ConstantSP leftMatch, ConstantSP rightMatch, ConstantSP filterLeftSP,ConstantSP filterRightSP);


bool isAllTrue(const ConstantSP& obj);
INDEX checkVectorSize(const ConstantSP& a, const ConstantSP& b);
INDEX checkVectorSize(const ConstantSP& a, const ConstantSP& b, ConstantSP& a1, ConstantSP& b1);
void checkArgumentSize(const FunctionDefSP& func, int actualArgCount);
DATA_TYPE checkDataType(const ConstantSP& a);
void checkTupleType(const ConstantSP& tuple, DATA_TYPE type);
ConstantSP getColumn(const TableSP& table, const string& colName, const ConstantSP& filter);
ConstantSP getColumn(const TableSP& table, const string& colName);
string stripColumnQualifier(const string& colName);
void checkJoinColumnType(const TableSP& left, const TableSP& right, const ConstantSP& leftMatchingColNames, const ConstantSP& rightMatchingColNames);
void checkTupleReduce(const ConstantSP& init, const vector<ConstantSP>& in, vector<ConstantSP>& out, DATA_TYPE& type);
FunctionDefSP normalizePartialFunction(Heap* heap, const FunctionDefSP& partialFunc, vector<ConstantSP>& oldArgs, vector<ConstantSP>& newArgs);
FunctionDefSP normalizePartialFunction(Heap* heap, const FunctionDefSP& partialFunc, vector<ConstantSP>& oldArgs, vector<ConstantSP>& newArgs, vector<int>& oldArgPositions);
bool generateChunkPaths(Heap* heap, const DomainSP& domain, const ConstantSP& filters, vector<string>& paths, const string& tableName, string& errMsg);
/*method: 0-auto, 1-naive, 2-strassen*/
ConstantSP multiMatrix(const ConstantSP& a, const ConstantSP& b, int method=0);
VectorSP transposeVector(const ConstantSP& vec, INDEX rows, INDEX columns);
/* test if two objects are identical. */
bool testObject(const ConstantSP& a, const ConstantSP& b, double precision);
bool testFloatingVector(const ConstantSP& a, const ConstantSP& b, double precision);
bool testComplexVector(const ConstantSP& a, const ConstantSP& b, double precision);
ConstantSP hashBucket(const ConstantSP& keyObj, DATA_TYPE targetRawType, int buckets);

inline bool isTupleOfArray(const ConstantSP& a){
	return a->isTuple() && !((Vector*)a.get())->isTableColumn() && a->size() > 0 && a->get(0)->isArray();
}

inline bool isMultiColumn(const ConstantSP& a){
	return a->isMatrix() || a->isTable() || (a->isDictionary() && ((Dictionary*)a.get())->isOrdered()) || isTupleOfArray(a);
}

inline bool isMultiColumn2(const ConstantSP& a){
	return a->isMatrix() || a->isTable() || (a->isDictionary() && ((Dictionary*)a.get())->isOrdered());
}

inline bool isMultiColumn(const ConstantSP& a, const ConstantSP& b){
	return a->isMatrix() || a->isTable() || b->isMatrix() || b->isTable() || isTupleOfArray(a) || isTupleOfArray(b);
}

inline bool isMultiColumn2(const ConstantSP& a, const ConstantSP& b){
	return a->isMatrix() || a->isTable() || b->isMatrix() || b->isTable();
}

inline bool isLabeledMultiColumn(const ConstantSP& a){
	return a->isIndexed() || a->isSeries();
}

inline bool isLabeledMultiColumn(const ConstantSP& a, const ConstantSP& b){
	return (a->isIndexed() || a->isSeries()) && (b->isIndexed() || b->isSeries());
}

inline bool isVectorDerived(const ConstantSP& a) {
	DATA_FORM df = a->getForm();
	DATA_TYPE type = a->getType();
	return df == DF_TABLE || df == DF_DICTIONARY || (df == DF_VECTOR && (type >= ARRAY_TYPE_BASE || type == DT_ANY));
}

inline bool isVectorDerived(const ConstantSP& a, const ConstantSP& b) {
	DATA_FORM df = a->getForm();
	DATA_TYPE type = a->getType();
	if (a->getType() == DT_IOTANY) {
		throw RuntimeException("IotAnyVector doesn't support binary operation");
	}

	if(df == DF_TABLE || df == DF_DICTIONARY || (df == DF_VECTOR && (type >= ARRAY_TYPE_BASE || type == DT_ANY)))
		return true;
	df = b->getForm();
	type = b->getType();
	if (b->getType() == DT_IOTANY) {
		throw RuntimeException("IotAnyVector doesn't support binary operation");
	}
	return df == DF_TABLE || df == DF_DICTIONARY || (df == DF_VECTOR && (type >= ARRAY_TYPE_BASE || type == DT_ANY));
}
typedef ConstantSP(*CompareFunc)(const ConstantSP&,const ConstantSP&, bool nullAsMinValue);
/**
 * The semanticCategory for computeUnary and computeBinary
 * 0: process all types of columns in a table
 * 1: process numeric columns in a table
 * 2: process temporal columns in a table
 * 3: process string columns in a table
 * 4: process numeric and temporal columns in a table
 *
 * By default, only process numeric columns in a table
 */
ConstantSP computeUnary(const ConstantSP& a, const ConstantSP& b, OptrFunc optr, int semanticCategory = 1, int objIndex = 0);
ConstantSP computeUnary(Heap* heap, vector<ConstantSP>& args, SysFunc optr, int semanticCategory = 1, int objIndex= -1);
ConstantSP computeBinary(const ConstantSP& a, const ConstantSP& b, OptrFunc optr, int semanticCategory = 1);
ConstantSP computeBinary(Heap* heap, vector<ConstantSP>& args, SysFunc optr, int semanticCategory = 1);
ConstantSP computeTernary(Heap* heap, const ConstantSP& a, const ConstantSP& b, const ConstantSP& c, SysFunc optr, int semanticCategory = 1);
ConstantSP computeTernaryInplace(Heap* heap, const ConstantSP& a, const ConstantSP& b, const ConstantSP& c, SysFunc optr, int semanticCategory = 1);
ConstantSP eachColumn(const ConstantSP& a, const ConstantSP& b, OptrFunc optr, int semanticCategory = 1, bool isAggregate = false, bool isPair = false, bool isVector = true);
ConstantSP eachColumn(const ConstantSP& a, const ConstantSP& b, OptrFunc optr, FastFunc fastFunc, int semanticCategory = 1, bool isAggregate = false, bool isPair = false, bool isVector = true);
ConstantSP eachColumn(Heap* heap, vector<ConstantSP>& args, SysFunc optr, int secondMatrixIndex, int semanticCategory = 1, bool isAggregate = false, bool isPair = false);
template<class T>
ConstantSP eachMatrix(Heap* heap, vector<ConstantSP>& args, T optr, FastFunc fastFunc, int secondMatrixIndex, bool aggregate);
ConstantSP eachTable(Heap* heap, vector<ConstantSP>& args, const FunctionDefSP& optr, int secondTableIndex, int semanticCategory=1);
ConstantSP eachColumn(Heap* heap, vector<ConstantSP>& args, SysFunc optr, int secondMatrixIndex, int thirdMatrixIndex, int semanticCategory);
ConstantSP eachColumnInPlace(Heap* heap, vector<ConstantSP>& args, SysFunc optr, int semanticCategory = 1, int objIndex = -1);

template<class T>
ConstantSP rowArrayVector(Heap* heap, vector<ConstantSP>& args, T optr, FastFunc fastFunc, bool aggregate);
template<class T>
ConstantSP rowArrayVector(Heap* heap, vector<ConstantSP>& args, T optr, FastFunc fastFunc, int secondArrayIndex, bool aggregate);
template<class T>
ConstantSP rowTuple(Heap* heap, vector<ConstantSP>& args, T optr, FastFunc fastFunc, int secondTupleIndex, bool aggregate);
template<class T>
ConstantSP rowWiseProcess(Heap* heap, vector<ConstantSP>& args, T optr, FastFunc fastFunc, bool aggregate, const string& name, const string& syntax);
/**
 * row-level calculation on matrix.
 * solutions: (1) transpose the matrix, (2) column-level calculation, (3) transpose the resulting matrix
 */
ConstantSP rowMatrix(Heap* heap, vector<ConstantSP>& args, SysFunc optr);
ConstantSP matrixJoin(const ConstantSP& a, const ConstantSP& b, bool indexedMatrix, bool useLeftLabel = true);
ConstantSP rollMultiColumns(Heap* heap, const ColumnContextSP& colContext, WindowJoinFunction& windowFunction, const vector<ConstantSP>& args,
		const string& funcName, bool binary = false, INDEX minPeriodArgIndex = -1);
ConstantSP rollVector(Heap *heap, WindowJoinFunction &windowFunction, const vector<ConstantSP> &args,
                      const string &funcName, const string &syntax, bool binary = false,
                      std::pair<long long, long long> excludedPeriod = {-1, -1},
                      const std::function<string(int/*ith*/)> &getArgName = nullptr);
ConstantSP forwardRollVector(Heap* heap, WindowJoinFunction& windowFunction, const vector<ConstantSP>& args, const string& funcName, const string& syntax, bool binary = false);
ConstantSP computeArrayVector(const ConstantSP& a, const ConstantSP& b, OptrFunc optr);
ConstantSP computeArrayVector(Heap* heap, vector<ConstantSP>& args, SysFunc optr);
VectorSP copyIndexVectorOfArrayVector(const VectorSP& index);
bool checkArrayVectorSize(const ConstantSP& a, const ConstantSP& b);

// Tracing
void cleanStaleTraces(Heap *heap, vector<ConstantSP> &arguments);
void setTraceMode(Heap* heap, vector<ConstantSP> &arguments);
ConstantSP viewTraceInfo(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP getTraces(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP getLocalTraceLog(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP gmd5(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowGmd5(Heap *heap, vector<ConstantSP> &arguments);

} // namespace OperatorImp

} // namespace ddb

#endif /* OPERATORIMP_H_ */
