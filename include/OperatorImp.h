/*
  * OperatorImp.h
 *
 *  Created on: Oct 18, 2013
 *      Author: dzhou
 */

#ifndef OPERATORIMP_H_
#define OPERATORIMP_H_

#include "CoreConcept.h"

#include <functional>

namespace OperatorImp{

enum DistanceType : int8_t { EUCLIDEAN = 0, SEUCLIDEAN, MINKOWSKI, COSINE, MAHALANOBIS };
string distanceTypeToString(DistanceType distanceType);
string distanceFucntionUsage(DistanceType distanceType);

//functions for other databases
ConstantSP oracleConcat(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP oracleRowNumber(Heap* heap, vector<ConstantSP>& arguments);

//no argument function
ConstantSP benchmark(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP now(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP now(const ConstantSP& a, const ConstantSP& b);
ConstantSP today(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP today(const ConstantSP& a, const ConstantSP& b);
ConstantSP memory(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP getHomeDir(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP getExecDir(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP getWorkDir(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP getNodeAlias(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP getNodeHost(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP getNodePort(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP getOS(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP getOSBit(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP version(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP version(const ConstantSP& a, const ConstantSP& b);
ConstantSP getEnv(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP getRequiredAPIVersion(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP getLicenseExpiration(Heap* heap, const ConstantSP& a, const ConstantSP& b);

ConstantSP constantDesc(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP brief(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP snippet(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP syntax(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP memSize(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP compress(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP compress(const ConstantSP& a, const ConstantSP& b);
ConstantSP decompress(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP decompress(const ConstantSP& a, const ConstantSP& b);

ConstantSP func(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP func(const ConstantSP& a, const ConstantSP& b);
ConstantSP add(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP add(const ConstantSP& a, const ConstantSP& b);
ConstantSP sub(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sub(const ConstantSP& a, const ConstantSP& b);
ConstantSP multiply(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP multiply(const ConstantSP& a, const ConstantSP& b);
ConstantSP divide(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP divide(const ConstantSP& a, const ConstantSP& b);
ConstantSP ratio(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ratio(const ConstantSP& a, const ConstantSP& b);
ConstantSP mod(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP mod(const ConstantSP& a, const ConstantSP& b);
ConstantSP power(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP power(const ConstantSP& a, const ConstantSP& b);
ConstantSP minIgnoreNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP minIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP maxIgnoreNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP maxIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP lt(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP lt(const ConstantSP& a, const ConstantSP& b);
ConstantSP le(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP le(const ConstantSP& a, const ConstantSP& b);
ConstantSP gt(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP gt(const ConstantSP& a, const ConstantSP& b);
ConstantSP ge(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ge(const ConstantSP& a, const ConstantSP& b);
ConstantSP between(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP between(const ConstantSP& a, const ConstantSP& b);
ConstantSP ltIgnoreNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ltIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP leIgnoreNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP leIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP gtIgnoreNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP gtIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP geIgnoreNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP geIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP betweenIgnoreNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP betweenIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP ltNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ltNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP leNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP leNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP gtNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP gtNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP geNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP geNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP betweenNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP betweenNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP ne(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ne(const ConstantSP& a, const ConstantSP& b);
ConstantSP equal(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP equal(const ConstantSP& a, const ConstantSP& b);
ConstantSP logicAnd(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP logicAnd(const ConstantSP& a, const ConstantSP& b);
ConstantSP logicOr(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP logicOr(const ConstantSP& a, const ConstantSP& b);
ConstantSP logicOrIgnoreNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP logicOrIgnoreNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP logicXor(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP logicXor(const ConstantSP& a, const ConstantSP& b);
ConstantSP bitAnd(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP bitAnd(const ConstantSP& a, const ConstantSP& b);
ConstantSP bitOr(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP bitOr(const ConstantSP& a, const ConstantSP& b);
ConstantSP bitXor(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP bitXor(const ConstantSP& a, const ConstantSP& b);
ConstantSP leftShift(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP leftShift(const ConstantSP& a, const ConstantSP& b);
ConstantSP rightShift(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP rightShift(const ConstantSP& a, const ConstantSP& b);
ConstantSP symmetricDifference(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP symmetricDifference(const ConstantSP& a, const ConstantSP& b);
ConstantSP hashBucket(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP hashBucket(const ConstantSP& a, const ConstantSP& b);
ConstantSP distance(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP distance(const ConstantSP& a, const ConstantSP& b);
ConstantSP ifNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ifNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP ifValid(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ifValid(const ConstantSP& a, const ConstantSP& b);
ConstantSP notIn(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP notIn(const ConstantSP& a, const ConstantSP& b);
ConstantSP notBetween(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP notBetween(const ConstantSP& a, const ConstantSP& b);
ConstantSP notLike(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP notLike(const ConstantSP& a, const ConstantSP& b);
ConstantSP nullIf(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP nullIf(const ConstantSP& a, const ConstantSP& b);
ConstantSP neAny(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP neAny(const ConstantSP& a, const ConstantSP& b);
ConstantSP eqAll(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP eqAll(const ConstantSP& a, const ConstantSP& b);

ConstantSP at(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP at(const ConstantSP& a, const ConstantSP& b);
ConstantSP eachAt(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP mask(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP mask(const ConstantSP& a, const ConstantSP& b);
ConstantSP member(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP rand(Heap* heap, const ConstantSP& a,const ConstantSP& b);
ConstantSP rand(const ConstantSP& a,const ConstantSP& b);
ConstantSP polynomial(Heap* heap, const ConstantSP& a,const ConstantSP& b);
ConstantSP take(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP take(const ConstantSP& a, const ConstantSP& b);
ConstantSP stretch(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP drop(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP til(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP seq(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP seq(const ConstantSP& a, const ConstantSP& b);
ConstantSP pair(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP pair(const ConstantSP& a, const ConstantSP& b);
ConstantSP in(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP in(const ConstantSP& a, const ConstantSP& b);
ConstantSP find(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP binsrch(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asof(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asof(const ConstantSP& a, const ConstantSP& b);
ConstantSP cut(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cast(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cast(const ConstantSP& a, const ConstantSP& b);
ConstantSP reshape(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP reshape(const ConstantSP& a, const ConstantSP& b);
ConstantSP join(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP join(const ConstantSP& a, const ConstantSP& b);
ConstantSP concatMatrix(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP concatMatrix(const ConstantSP& a, const ConstantSP& b);
ConstantSP subarray(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP subtuple(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP arrayVector(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP arrayVector(const ConstantSP& a, const ConstantSP& b);
ConstantSP slicedTable(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP slicedTable(const ConstantSP& a, const ConstantSP& b);
ConstantSP head(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP tail(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP indexedSeries(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP enlist(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP lowerBound(Heap* heap, const ConstantSP& a, const ConstantSP& b);

ConstantSP linearTimeTrend(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP kama(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP wilder(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ema(Heap *pHeap, vector<ConstantSP>& arguments);
ConstantSP sma(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP wma(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP dema(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP tema(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP trima(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP t3(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP gema(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ma(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP sessionWindow(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP talibNull(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mcount(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mfirst(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mfirstNot(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mifirstNot(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mlast(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mlastNot(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP milastNot(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mmed(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mavg(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mmin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mmax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mimin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mimax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP miminLast(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP mimaxLast(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP msum(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP msum2(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mprod(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mstd(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mvar(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mstdp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mvarp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mskew(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mkurtosis(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mrank(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mcorr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mcovar(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mcovarp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mbeta(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mwsum(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mwavg(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mslr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mpercentile(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mmse(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mmad(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP matImin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP matImax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mmaxPositiveStreak(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmcount(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmfirst(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmlast(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmsum(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmsum2(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmmed(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmpercentile(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmavg(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmstd(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmvar(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmstdp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmvarp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmskew(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmkurtosis(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmprod(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmmin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmmax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmrank(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmcorr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmcovar(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmcovarp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmbeta(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmwsum(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmwavg(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmove(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmatImin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmatImax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ftmsum(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ftmmax(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ftmmin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ftmprod(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ftmlast(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ftmwavg(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumskewTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumkurtosisTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumsumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumavgTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumvarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumvarpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumstdTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumstdpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumcorrTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumbetaTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumcovarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumcovarpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumwsumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mskewTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mkurtosisTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP msumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mavgTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mvarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mvarpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mstdTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mstdpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mcorrTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mbetaTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mcovarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mcovarpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mwsumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mpercentileTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmskewTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmkurtosisTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmsumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmavgTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmvarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmvarpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmstdTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmstdpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmcorrTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmbetaTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmcovarTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmcovarpTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmwsumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cumTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmovingTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP movingTopN(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP movingTopNIndex(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP movingWindowIndex(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP movingWindowData(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmovingWindowIndex(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmovingWindowData(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ewmMean(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP ewmVar(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP ewmStd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP ewmCorr(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP ewmCov(Heap* heap, vector<ConstantSP>& arguments);

//vector manipulation
ConstantSP rowNo(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP move(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP move(const ConstantSP& a, const ConstantSP& b);
ConstantSP next(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP next(const ConstantSP& a, const ConstantSP& b);
ConstantSP prev(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP prev(const ConstantSP& a, const ConstantSP& b);
ConstantSP lead(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP lag(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP nextState(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP prevState(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP reverse(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP reverse(const ConstantSP& a, const ConstantSP& b);
ConstantSP shuffle(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isortUnique(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isort(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isort(const ConstantSP& a, const ConstantSP& b);
ConstantSP sort(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sort(const ConstantSP& a, const ConstantSP& b);
ConstantSP distinct(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP distinct(const ConstantSP& a, const ConstantSP& b);
ConstantSP groups(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP keys(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP values(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP row(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP column(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP columnNames(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP rowNames(Heap* heap, const ConstantSP& a, const ConstantSP& b);

//aggregate function
ConstantSP isOrderedDict(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isIndexedMatrix(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isIndexedSeries(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isColumnarTuple(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isColumnarTuple(const ConstantSP& a, const ConstantSP& b);
ConstantSP isVoid(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isNothing(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP type(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP typestr(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP category(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP form(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP hasNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP hasNull( const ConstantSP& a, const ConstantSP& b);
ConstantSP size(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP capacity(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP count(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP count(const ConstantSP& a, const ConstantSP& b);
ConstantSP countNanInf(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP countNanInf(const ConstantSP& a, const ConstantSP& b);
ConstantSP contextCount(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP rows(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP columns(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP shape(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP tupleSum(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sum(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sum(const ConstantSP& a, const ConstantSP& b);
ConstantSP contextSum(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sum2(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sum2(const ConstantSP& a, const ConstantSP& b);
ConstantSP sum3(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sum4(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP contextSum2(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP mad(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sem(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP avg(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP avg(const ConstantSP& a, const ConstantSP& b);
ConstantSP var(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP std(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP varp(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP varp(const ConstantSP& a, const ConstantSP& b);
ConstantSP stdp(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP skew(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP kurtosis(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP prod(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP max(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP max(const ConstantSP& a, const ConstantSP& b);
ConstantSP min(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP min(const ConstantSP& a, const ConstantSP& b);
ConstantSP imax(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP imax(const ConstantSP& a, const ConstantSP& b);
ConstantSP imin(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP imin(const ConstantSP& a, const ConstantSP& b);
ConstantSP imaxLast(Heap* heap, const ConstantSP &a, const ConstantSP &b);
ConstantSP imaxLast(const ConstantSP &a, const ConstantSP &b);
ConstantSP iminLast(Heap* heap, const ConstantSP &a, const ConstantSP &b);
ConstantSP iminLast(const ConstantSP &a, const ConstantSP &b);
ConstantSP first(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP last(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP lastNot(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP firstNot(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ilastNot(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ifirstNot(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ifirstNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP med(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP med(const ConstantSP& a, const ConstantSP& b);
ConstantSP mode(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP stat(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP maxPositiveStreak(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP allTrue(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP anyTrue(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP anyTrue(const ConstantSP& a, const ConstantSP& b);
ConstantSP oddTrue(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP maxDrawdown(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP rms(Heap* heap, const ConstantSP& a, const ConstantSP& b);

ConstantSP isSorted(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP atIMax(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP atIMin(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP wavg(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP wavg(const ConstantSP& a, const ConstantSP& b);
ConstantSP wsum(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP wsum(const ConstantSP& a, const ConstantSP& b);
ConstantSP dot(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP dot(const ConstantSP& a, const ConstantSP& b);
ConstantSP euclidean(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cosine(Heap *heap, const ConstantSP &a, const ConstantSP &b);
ConstantSP mahalanobis(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP seuclidean(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP minkowski(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP jaccard(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP tanimoto(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP correlation(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP spearmanr(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP kendall(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP mutualInformation(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP covariance(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP covarp(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP beta(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP searchK(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP difference(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP crossStat(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP crossStat(const ConstantSP& a, const ConstantSP& b);
ConstantSP isMonotonic(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isMonotonicIncreasing(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isMonotonicDecreasing(Heap* heap, const ConstantSP& a, const ConstantSP& b);

ConstantSP eye(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP eye(const ConstantSP& a, const ConstantSP& b);
ConstantSP diag(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP diag(const ConstantSP& a, const ConstantSP& b);
ConstantSP naiveMulti(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP strassenMulti(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP transpose(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP transpose(const ConstantSP& a, const ConstantSP& b);
ConstantSP inverse(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP inverse(const ConstantSP& a, const ConstantSP& b);
ConstantSP pinverse(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP pinverse(const ConstantSP& a, const ConstantSP& b);
ConstantSP det(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP det(const ConstantSP& a, const ConstantSP& b);
ConstantSP solve(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP solve(const ConstantSP& a, const ConstantSP& b);
ConstantSP cholesky(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cholesky(const ConstantSP& a, const ConstantSP& b);
ConstantSP lu(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP lu(const ConstantSP& a, const ConstantSP& b);
ConstantSP schur(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP schur(const ConstantSP& a, const ConstantSP& b);
ConstantSP triu(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP tril(Heap* heap, const ConstantSP& a, const ConstantSP& b);

ConstantSP writeBytes(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP readBytes(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP readLine(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP readLines(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP toJson(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP toStdJson(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP toStdJson(const ConstantSP& a, const ConstantSP& b);
ConstantSP fromJson(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP fromStdJson(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP crc32(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP md5(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP wideTable(Heap* heap, const ConstantSP& a, const ConstantSP& b);

//unary temporal functions
ConstantSP year(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP year(const ConstantSP& a, const ConstantSP& b);
ConstantSP month(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP month(const ConstantSP& a, const ConstantSP& b);
ConstantSP date(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP date(const ConstantSP& a, const ConstantSP& b);
ConstantSP hour(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP hour(const ConstantSP& a, const ConstantSP& b);
ConstantSP minute(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP minute(const ConstantSP& a, const ConstantSP& b);
ConstantSP second(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP second(const ConstantSP& a, const ConstantSP& b);
ConstantSP time(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP time(const ConstantSP& a, const ConstantSP& b);
ConstantSP datetime(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP datetime(const ConstantSP& a, const ConstantSP& b);
ConstantSP datehour(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP datehour(const ConstantSP& a, const ConstantSP& b);
ConstantSP timestamp(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP timestamp(const ConstantSP& a, const ConstantSP& b);
ConstantSP nanotime(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP nanotime(const ConstantSP& a, const ConstantSP& b);
ConstantSP nanotimestamp(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP nanotimestamp(const ConstantSP& a, const ConstantSP& b);
ConstantSP weekday(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP weekday(const ConstantSP& a, const ConstantSP& b);
ConstantSP dayOfWeek(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP dayOfWeek(const ConstantSP& a, const ConstantSP& b);
ConstantSP localtime(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP localtime(const ConstantSP& a, const ConstantSP& b);
ConstantSP gmtime(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP gmtime(const ConstantSP& a, const ConstantSP& b);
ConstantSP concatDateTime(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP concatDateTime(const ConstantSP& a, const ConstantSP& b);
ConstantSP dayOfYear(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP dayOfYear(const ConstantSP& a, const ConstantSP& b);
ConstantSP dayOfMonth(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP dayOfMonth(const ConstantSP& a, const ConstantSP& b);
ConstantSP quarterOfYear(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP quarterOfYear(const ConstantSP& a, const ConstantSP& b);
ConstantSP monthOfYear(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP monthOfYear(const ConstantSP& a, const ConstantSP& b);
ConstantSP weekOfYear(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP weekOfYear(const ConstantSP& a, const ConstantSP& b);
ConstantSP hourOfDay(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP hourOfDay(const ConstantSP& a, const ConstantSP& b);
ConstantSP minuteOfHour(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP minuteOfHour(const ConstantSP& a, const ConstantSP& b);
ConstantSP secondOfMinute(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP secondOfMinute(const ConstantSP& a, const ConstantSP& b);
ConstantSP millisecond(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP microsecond(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP nanosecond(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isMonthStart(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isMonthEnd(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isQuarterStart(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isQuarterEnd(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isYearStart(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isYearEnd(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isLeapYear(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP daysInMonth(Heap* heap, const ConstantSP& a, const ConstantSP& b);

//data type conversion
ConstantSP decodeShortGenomeSeq(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP encodeShortGenomeSeq(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP genShortGenomeSeq(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asBool(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asBool(const ConstantSP& a, const ConstantSP& b);
ConstantSP asChar(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asChar(const ConstantSP& a, const ConstantSP& b);
ConstantSP asShort(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asShort(const ConstantSP& a, const ConstantSP& b);
ConstantSP asInt(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asInt(const ConstantSP& a, const ConstantSP& b);
ConstantSP asLong(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asLong(const ConstantSP& a, const ConstantSP& b);
ConstantSP asFloat(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asFloat(const ConstantSP& a, const ConstantSP& b);
ConstantSP asDouble(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asDouble(const ConstantSP& a, const ConstantSP& b);
ConstantSP symbolCode(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asSymbol(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asSymbol(const ConstantSP& a, const ConstantSP& b);
ConstantSP asString(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asString(const ConstantSP& a, const ConstantSP& b);
ConstantSP asBlob(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asBlob(const ConstantSP& a, const ConstantSP& b);
ConstantSP asUuid(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asUuid(const ConstantSP& a, const ConstantSP& b);
ConstantSP asIPAddr(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asIPAddr(const ConstantSP& a, const ConstantSP& b);
ConstantSP asInt128(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asInt128(const ConstantSP& a, const ConstantSP& b);
ConstantSP asComplex(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asComplex(const ConstantSP& a, const ConstantSP& b);
ConstantSP asPoint(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asPoint(const ConstantSP& a, const ConstantSP& b);
ConstantSP asDuration(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asDuration(const ConstantSP& a, const ConstantSP& b);
ConstantSP asDecimal32(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asDecimal32(const ConstantSP& a, const ConstantSP& b);
ConstantSP asDecimal64(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asDecimal64(const ConstantSP& a, const ConstantSP& b);
ConstantSP asDecimal128(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asDecimal128(const ConstantSP& a, const ConstantSP& b);
ConstantSP makeDuration(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP highDouble(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP lowDouble(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP highLong(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP lowLong(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP decimalMultiply(Heap *heap, vector<ConstantSP> &arguments);

//unary math functions
ConstantSP signum(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP signbit(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP nullFlag(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isNull(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isNull(const ConstantSP& a, const ConstantSP& b);
ConstantSP isValid(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isValid(const ConstantSP& a, const ConstantSP& b);
ConstantSP isNanInf(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP nullFill(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP nullFill(const ConstantSP& a, const ConstantSP& b);
ConstantSP nanInfFill(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP nanInfFill(const ConstantSP& a, const ConstantSP& b);
ConstantSP where(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP where(const ConstantSP& a, const ConstantSP& b);
ConstantSP logicNot(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP logicNot(const ConstantSP& a, const ConstantSP& b);
ConstantSP bitNot(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP round(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP decimalFormat(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP decimalFormat(const ConstantSP& a, const ConstantSP& b);
ConstantSP temporalFormat(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP temporalFormat(const ConstantSP& a, const ConstantSP& b);
ConstantSP format(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP format(const ConstantSP& a, const ConstantSP& b);
ConstantSP temporalParse(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP temporalParse(const ConstantSP& a, const ConstantSP& b);
ConstantSP ceil(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP floor(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP neg(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP neg(const ConstantSP& a, const ConstantSP& b);
ConstantSP abs(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP abs(const ConstantSP& a, const ConstantSP& b);
ConstantSP reciprocal(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sin(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cos(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP tan(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP asin(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP acos(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP atan(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sinh(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cosh(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP tanh(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP tanh(const ConstantSP& a, const ConstantSP& b);
ConstantSP asinh(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP acosh(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP atanh(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP deg2rad(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP rad2deg(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sqrt(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sqrt(const ConstantSP& a, const ConstantSP& b);
ConstantSP exp(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP exp(const ConstantSP& a, const ConstantSP& b);
ConstantSP log(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP log(const ConstantSP& a, const ConstantSP& b);
ConstantSP exp2(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP expm1(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP log2(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP log10(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP log1p(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cbrt(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP square(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP square(const ConstantSP& a, const ConstantSP& b);
ConstantSP flatten(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP flatten(const ConstantSP& a, const ConstantSP& b);
ConstantSP copy(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP copy(const ConstantSP& a, const ConstantSP& b);
ConstantSP asis(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP deepCopy(Heap* heap, const ConstantSP& a, const ConstantSP& b);

ConstantSP cumPositiveStreak(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumsum(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumsum(const ConstantSP& a, const ConstantSP& b);
ConstantSP cumsum2(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumsum3(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumsum4(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumprod(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumprod(const ConstantSP& a, const ConstantSP& b);
ConstantSP cummin(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cummax(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumcount(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumavg(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumstd(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumvar(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumstdp(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumvarp(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumpercentile(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP cumrank(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP cumdenseRank(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP cummed(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumcorr(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumcovar(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumcovarp(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumbeta(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumwsum(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumwavg(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumfirstNot(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumlastNot(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumifirstNot(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumilastNot(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP cumnunique(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP dynamicGroupCumsum(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP dynamicGroupCumcount(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP cummdd(Heap* heap, const ConstantSP& a, const ConstantSP& b);

ConstantSP deltas(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP deltas(const ConstantSP& a, const ConstantSP& b);
ConstantSP ratios(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP bar(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP volumeBar(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP zscore(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP demean(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP percentChange(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP segment(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP topRange(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP lowRange(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isPeak(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isValley(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP sumbars(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP valueChanged(Heap* heap, const ConstantSP& a, const ConstantSP& b);

//string functions
ConstantSP isUpper(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isLower(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isTitle(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isSpace(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isAlpha(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isNumeric(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isAlNum(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP isDigit(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP capitalize(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP title(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP swapCase(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP like(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP like(const ConstantSP& a, const ConstantSP& b);
ConstantSP ilike(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP trim(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP strip(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP strlen(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP strlenu(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP wc(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP upper(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP lower(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP repeat(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP left(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP right(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP ltrim(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP rtrim(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP startsWith(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP endsWith(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP strpos(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP charAt(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP split(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP concat(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP concat(const ConstantSP& a, const ConstantSP& b);
ConstantSP hex(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP binary(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP initcap(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP base64Encode(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP base64Decode(Heap* heap, const ConstantSP& a, const ConstantSP& b);
ConstantSP strReplace(Heap * heap,vector<ConstantSP>& arguments);
ConstantSP substr(Heap * heap,vector<ConstantSP>& arguments);
ConstantSP substru(Heap * heap,vector<ConstantSP>& arguments);
ConstantSP lpad(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP rpad(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP regexReplace(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP regexFind(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP regexCount(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP regexFindStr(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP convertEncode(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP toUTF8(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP fromUTF8(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP makeKey(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP makeSortedKey(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP hmac(Heap* heap, vector<ConstantSP>& arguments);

//adverb functions
ConstantSP each(Heap* heap, const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP loop(Heap* heap, const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP eachPre(Heap* heap, const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP eachPost(Heap* heap, const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP eachRight(Heap* heap, const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP eachLeft(Heap* heap, const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP cross(Heap* heap, const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP reduce(Heap* heap, const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP accumulate(Heap* heap, const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP byRow(Heap* heap, const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);
ConstantSP byColumn(Heap* heap, const ConstantSP& a, const ConstantSP& b, const string& optrName, OptrFunc optr, FastFunc fastImp, int assembleRule);

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
ConstantSP quantile(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP quantileSeries(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP percentile(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP percentileRank(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP firstHit(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ifirstHit(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP geoWithin(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rdp(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP zigzag(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP gmd5(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowGmd5(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP qcut(Heap *heap, vector<ConstantSP> &arguments);

ConstantSP sqlCol(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP sqlColAlias(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP sqlTuple(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP expression(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP binaryExpr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP unifiedExpr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP makeCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP makeUnifiedCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP sql(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP sqlUpdate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP sqlDelete(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP evaluate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP objectComponent(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP objectType(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP getChunkPath(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP sqlDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP textFilesDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP textChunkDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cacheDSNow(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP clearDSCacheNow(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP saveDSToDB(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cacheDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP clearDSCache(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP transDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP scheduleDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mrDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP imrDS(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP pipeline(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP readTableFromFileSegment(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP readTextFromFileSegment(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP readTabletChunk(Heap* heap, vector<ConstantSP>& arguments);

ConstantSP partial(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP piecewise(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP assemble(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP unifiedCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP delayedFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP partitionFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP dynamicFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP pdynamicFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP nothrowFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ptableCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP loopFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ploopFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP eachFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ceachFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP peachFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP crossFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP pcrossFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP eachLeftFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP eachRightFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP eachPreFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP eachPostFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP rowReduceFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP reduceFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP accumulateFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP groupFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP rowGroupFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP contextFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP segmentFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP pivotFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP movingFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP movingValidFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP rollingFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP allFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP anyFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP fillopFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP byRowFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP byColumnFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP talibFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP compareFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tmovingFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP aggrTopNFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP windowFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP twindowFuncCall(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP adverbFuncCall(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP pdfF(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP pdfChiSquare(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP pdfNormal(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP cdfStudent(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfF(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfBeta(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfGamma(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfChiSquare(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfNormal(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfExp(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfUniform(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfWeibull(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfZipf(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfLogistic(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfBinomial(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfPoisson(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cdfKolmogorov(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP invStudent(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invF(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invBeta(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invGamma(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invChiSquare(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invNormal(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invExp(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invUniform(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invWeibull(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invZipf(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invLogistic(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invBinomial(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invPoisson(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP invKolmogorov(Heap *heap, vector<ConstantSP> &arguments);

ConstantSP randStudent(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randF(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randBeta(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randGamma(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randChiSquare(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randNormal(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randMultivariateNormal(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randExp(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randUniform(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randWeibull(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randZipf(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randLogistic(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randBinomial(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randPoisson(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randKolmogorov(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP randDiscrete(Heap *heap, vector<ConstantSP> &arguments);

ConstantSP bondDuration(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP bondConvexity(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP bondDirtyPrice(Heap *heap, vector<ConstantSP> &arguments);

ConstantSP rowMin(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowMax(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowImin(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowImax(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowIminLast(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowImaxLast(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowAt(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowSum(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowSum2(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowCount(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowSize(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowStd(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowStdp(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowVar(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowVarp(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowSkew(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowKurtosis(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowAvg(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowAnd(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowOr(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowXor(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowProd(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowMed(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowRank(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowDenseRank(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowRatios(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowDeltas(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowPercentChange(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowZscore(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowDemean(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowCumsum(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowCumwsum(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowCumprod(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowCummax(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowCummin(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowMove(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowNext(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowPrev(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowCorr(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowCovar(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowCovarp(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowBeta(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowWsum(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowWavg(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowTanimoto(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowJaccard(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowEuclidean(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowDot(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowAlign(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowFilterAndSort(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowMergeAndSort(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowMinkowski(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowSeuclidean(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowCosine(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP rowMahalanobis(Heap *heap, vector<ConstantSP> &arguments);

ConstantSP coalesce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP panel(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP unionAll(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP conditionalFilter(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP trueRange(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP isortTop(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP partition(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP sample(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP freq(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP normal(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP digitize(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP bucket(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP bucketCount(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cutPoints(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP wcovariance(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ns(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP unpivot(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ungroup(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP rank(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP denseRank(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP isortInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP joinInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP removeHead(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP removeTail(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP iif(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP iterate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP conditionalIterate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP stateIterate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP replace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ffill(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP bfill(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP lfill(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ffillInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP bfillInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP lfillInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP winsorize(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP winsorizeInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP clip(Heap *heap, vector<ConstantSP> &args);
ConstantSP clipInplace(Heap *heap, vector<ConstantSP> &args);
ConstantSP locate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP slice(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP sliceByKey(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cell(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cells(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP array(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP bigarray(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP matrix(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP repmat(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP set(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP dictionary(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP syncDictionary(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP table(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP keyedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP latestKeyedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP indexedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP latestIndexedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP streamTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP keyedStreamTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP latestKeyedStreamTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP mvccTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP cachedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP database(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP addValuePartitions(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP addRangePartitions(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tableInsert(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP getTablet(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP svd(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP qr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP asfreq(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP resample(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP freqSeq(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP regroup(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP align(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP merge(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP eqFloat(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP oneHot(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP fixedLengthArrayVector(Heap* heap, vector<ConstantSP>& arguments);

ConstantSP exists(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP existsDatabase(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP existsTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP existsPartition(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP listTables(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP getTables(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP parseJsonTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP jsonExtract(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP extractTextSchema(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP loadText(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ploadText(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP loadTextEx(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP loadTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP renameTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP loadColumn(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP loadStreamTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP loadMvccTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP createPartitionedTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP createTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP createIMOLTPTable(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP loadTableBySQL(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP bigSQL(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP loadRecord(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP coevent(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP covarMatrix(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP corrMatrix(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP objAddr(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP objects(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP objByName(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP funcByName(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP defined(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP functions(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP files(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP schema(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP orcaObject(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP refCount(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP xdb(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP pnodeRun(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP remoteRun(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP remoteUrgentRun(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP remoteRunWithCompression(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP remoteRunCompatible(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP rpc(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP socket(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP file(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP writeObject(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP readObject(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP write(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP writeLine(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP writeLines(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP read(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP readLinesInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP seek(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP writeRecord(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP readRecordInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP clearCache(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP getDiskIOStat(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP getRunningQueries(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP getCompletedQueries(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP getSessionMemoryStat(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP getMemoryStat(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP transaction(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP checksum(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP toCharArray(Heap* heap, vector<ConstantSP>& arguments);

ConstantSP transFreq(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP temporalSeq(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP convertTZ(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP temporalAdd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP temporalDeltas(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP temporalDiff(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP dailyAlignedBar(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP businessDay(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP monthBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP monthEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP businessMonthBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP businessMonthEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP quarterBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP quarterEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP businessQuarterBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP businessQuarterEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP semiannualBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP semiannualEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP yearBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP yearEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP businessYearBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP businessYearEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP semiMonthBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP semiMonthEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP weekOfMonth(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP lastWeekOfMonth(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP weekBegin(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP weekEnd(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP fy5253(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP fy5253Quarter(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP marketHoliday(Heap* heap, vector<ConstantSP>& arguments);

ConstantSP dropna(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP fillInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP nullFillInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP shuffleInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP replaceInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP sortInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP append(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP upsert(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP tableUpsert(Heap *heap, vector<ConstantSP> &arguments);
ConstantSP pop(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP erase(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP update(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP sortBy(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ajInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP ljInPlace(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP clear(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP rename(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP dictUpdate(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP reorderColumns(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP replaceColumn(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP dropColumns(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP setIndexedMatrix(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP setIndexedSeries(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP setColumnarTuple(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP appendTuple(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP memberModify(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP removeInplace(Heap* heap,vector<ConstantSP>& arguments);

ConstantSP eqObj(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP license(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP updateLicense(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP genLicenseAuthorization(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP readLicenseAuthorization(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP shell(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP loadPlugin(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP loadPatch(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP chunkMeta(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP submitJob(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP submitJobEx(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP submitJobEx2(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP getRecentJobs(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP getJobStatus(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP getJobReturn(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP getJobMessage(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP getConsoleJobs(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP evalTimer(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP getPartitionDataFromDS(Heap* heap, vector<ConstantSP>& arguments);
ConstantSP optimizeUDF(Heap* heap, vector<ConstantSP>& arguments);

//reduce and running functions for map-reduce of aggregate functions
ConstantSP avgReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP stdReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP stdpReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP varReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP varpReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP corrReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP covarReduce(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP covarpReduce(Heap* heap,vector<ConstantSP>& arguments);
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
ConstantSP covarpRunning(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP skewRunning(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP skewRunning2(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP kurtosisRunning(Heap* heap,vector<ConstantSP>& arguments);
ConstantSP kurtosisRunning2(Heap* heap,vector<ConstantSP>& arguments);

//system procedures
void sleep(Heap* heap,vector<ConstantSP>& arguments);
void print(Heap* heap,vector<ConstantSP>& arguments);
void writeLog(Heap* heap,vector<ConstantSP>& arguments);
void run(Heap* heap,vector<ConstantSP>& arguments);
void runScript(Heap* heap,vector<ConstantSP>& arguments);
void test(Heap* heap,vector<ConstantSP>& arguments);
void testCommitFailure(Heap* heap,vector<ConstantSP>& arguments);
void saveText(Heap* heap,vector<ConstantSP>& arguments);
void saveTable(Heap* heap,vector<ConstantSP>& arguments);
void savePartition(Heap* heap,vector<ConstantSP>& arguments);
void saveDualPartition(Heap* heap,vector<ConstantSP>& arguments);
void enableActivePartition(Heap* heap,vector<ConstantSP>& arguments);
void disableActivePartition(Heap* heap,vector<ConstantSP>& arguments);
void dropPartition(Heap* heap,vector<ConstantSP>& arguments);
void dropTable(Heap* heap,vector<ConstantSP>& arguments);
void saveDatabase(Heap* heap,vector<ConstantSP>& arguments);
void dropDatabase(Heap* heap,vector<ConstantSP>& arguments);
void addColumn(Heap* heap,vector<ConstantSP>& arguments);
void dropColumn(Heap* heap,vector<ConstantSP>& arguments);
void setColumnComment(Heap* heap,vector<ConstantSP>& arguments);
void setTableSensitiveColumn(Heap* heap,vector<ConstantSP>& arguments);
void setTableComment(Heap* heap,vector<ConstantSP>& arguments);
void setRetentionPolicy(Heap* heap,vector<ConstantSP>& arguments);
void setRemoveSpecialChar(Heap* heap,vector<ConstantSP>& arguments);
void setDatabaseForClusterReplication(Heap* heap,vector<ConstantSP>& arguments);
void setDatabaseClusterReplicationExecutionSet(Heap* heap,vector<ConstantSP>& arguments);
void setDatabaseOwner(Heap* heap,vector<ConstantSP>& arguments);
void setAtomicLevel(Heap* heap,vector<ConstantSP>& arguments);
void setRandomSeed(Heap* heap,vector<ConstantSP>& arguments);
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
void undef(Heap* heap,vector<ConstantSP>& arguments);
void plot(Heap* heap,vector<ConstantSP>& arguments);
void plotHist(Heap* heap,vector<ConstantSP>& arguments);
void generateLicense(Heap* heap, vector<ConstantSP>& arguments);
void generatePluginSign(Heap* heap, vector<ConstantSP>& arguments);
void clearAllCache(Heap* heap, vector<ConstantSP>& arguments);
void clearCachedDatabase(Heap* heap, vector<ConstantSP>& arguments);
void clearDatabaseDomain(Heap* heap, vector<ConstantSP>& arguments);
void clearCachedSegment(Heap* heap, vector<ConstantSP>& arguments);
void checkMemoryUsageWarning(Heap* heap, vector<ConstantSP>& arguments);
void startHeapSample(Heap* heap, vector<ConstantSP> &arguments);
void stopHeapSample(Heap* heap, vector<ConstantSP> &arguments);
void dumpHeapSample(Heap* heap, vector<ConstantSP> &arguments);
void fflush(Heap* heap, vector<ConstantSP>& arguments);

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
ConstantSP uploadModule(Heap* heap, vector<ConstantSP>& arguments);
void encryptModule(Heap* heap, vector<ConstantSP>& arguments);
void appendMsg(Heap* heap,vector<ConstantSP>& arguments);
void generateMachineFingerprint(Heap *heap, vector<ConstantSP> &args);
void composeMachineFingerprint(Heap *heap, vector<ConstantSP> &args);
void enableDynamicScriptOptimization(Heap *heap, vector<ConstantSP> &args);
void disableDynamicScriptOptimization(Heap *heap, vector<ConstantSP> &args);

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
INDEX checkVectorSize(const ConstantSP& a, const ConstantSP& b, DATA_FORM& form);
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

bool processVectorFuncOverTupleByRow(Heap* heap);

inline bool isTupleOfArray(const ConstantSP& a){
	return a->isTuple() && !((Vector*)a.get())->isTableColumn() && a->size() > 0 && a->get(0)->isArray();
}

inline bool isInMemoryTableWithKey(const ConstantSP& a){
	if(!a->isTable())
		return false;
	Table* tbl = (Table*)a.get();
	return tbl->getTableType() == BASICTBL && tbl->getKeyColumnCount() > 0;
}

inline bool isMultiColumn(const ConstantSP& a){
	return a->isMatrix() || a->isTable() || a->isDictionary() || isTupleOfArray(a);
}

inline bool isMultiColumn2(const ConstantSP& a){
	return a->isMatrix() || a->isTable() || a->isDictionary();
}

inline bool isMultiColumn3(Heap* heap, const ConstantSP& a){
	return a->isMatrix() || a->isTable() || a->isDictionary() || (!processVectorFuncOverTupleByRow(heap) && isTupleOfArray(a));
}

inline bool isMultiColumn4(const ConstantSP& a){
	return a->isTable() || a->isDictionary();
}

inline bool isInMemoryTableWithKey(const ConstantSP& a, const ConstantSP& b){
	if(!a->isTable() || !b->isTable())
		return false;
	Table* tblA = (Table*)a.get();
	Table* tblB = (Table*)b.get();
	return tblA->getTableType() == BASICTBL && tblA->getKeyColumnCount() > 0 &&
			tblB->getTableType() == BASICTBL && tblB->getKeyColumnCount() > 0;
}

inline bool isMultiColumn(const ConstantSP& a, const ConstantSP& b){
	return a->isMatrix() || a->isTable() || a->isDictionary() || isTupleOfArray(a) ||
			b->isMatrix() || b->isTable() || a->isDictionary() || isTupleOfArray(b);
}

inline bool isMultiColumn2(const ConstantSP& a, const ConstantSP& b){
	return a->isMatrix() || a->isTable() || a->isDictionary() ||
			b->isMatrix() || b->isTable() || b->isDictionary();
}

inline bool isMultiColumn3(Heap* heap, const ConstantSP& a, const ConstantSP& b){
	return a->isMatrix() || a->isTable() || a->isDictionary() ||
			b->isMatrix() || b->isTable() || b->isDictionary() ||
			(!processVectorFuncOverTupleByRow(heap) && (isTupleOfArray(a) || isTupleOfArray(b)));
}

inline bool isMultiColumn4(const ConstantSP& a, const ConstantSP& b){
	return a->isTable() || a->isDictionary() || b->isTable() || b->isDictionary();
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

inline bool isSpecialVector(DATA_CATEGORY cat){
	return cat == MIXED || cat == ARRAY;
}

inline bool isSpecialVector(const ConstantSP& a, const ConstantSP& b){
	DATA_CATEGORY catA = a->getCategory();
	DATA_CATEGORY catB = b->getCategory();
	return catA == MIXED || catA == ARRAY || catB == MIXED || catB == ARRAY;
}

typedef ConstantSP(*CompareFunc)(Heap* heap, const ConstantSP&,const ConstantSP&, bool nullAsMinValue);
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
ConstantSP computeUnary(Heap* heap, const ConstantSP& a, const ConstantSP& b, OptrFunc optr, int semanticCategory = 1, int objIndex = 0);
ConstantSP computeUnary(Heap* heap, vector<ConstantSP>& args, SysFunc optr, int semanticCategory = 1, int objIndex= -1);
ConstantSP computeBinary(Heap* heap, const ConstantSP& a, const ConstantSP& b, OptrFunc optr, int semanticCategory = 1, const ConstantSP& nullFillForDict = nullptr);
ConstantSP computeBinary(Heap* heap, vector<ConstantSP>& args, SysFunc optr, int semanticCategory = 1);
ConstantSP computeTernary(Heap* heap, const ConstantSP& a, const ConstantSP& b, const ConstantSP& c, SysFunc optr, int semanticCategory = 1);
ConstantSP computeTernaryInplace(Heap* heap, const ConstantSP& a, const ConstantSP& b, const ConstantSP& c, SysFunc optr, int semanticCategory = 1);
ConstantSP eachColumn(Heap* heap, const ConstantSP& a, const ConstantSP& b, OptrFunc optr, int semanticCategory = 1, bool isAggregate = false, bool isPair = false, bool isVector = true, const ConstantSP& nullFillForDict = nullptr);
ConstantSP eachColumn(Heap* heap, const ConstantSP& a, const ConstantSP& b, OptrFunc optr, FastFunc fastFunc, int semanticCategory = 1, bool isAggregate = false, bool isPair = false, bool isVector = true, const ConstantSP& nullFillForDict = nullptr);
ConstantSP eachColumn(Heap* heap, vector<ConstantSP>& args, SysFunc optr, int secondMatrixIndex, int semanticCategory = 1, bool isAggregate = false, bool isPair = false, const ConstantSP& nullFillForDict = nullptr);
template<class T>
ConstantSP eachMatrix(Heap* heap, vector<ConstantSP>& args, T optr, FastFunc fastFunc, int secondMatrixIndex, bool aggregate);
ConstantSP eachTable(Heap* heap, vector<ConstantSP>& args, const FunctionDefSP& optr, int secondTableIndex, int semanticCategory=1);
ConstantSP eachColumn(Heap* heap, vector<ConstantSP>& args, SysFunc optr, int secondMatrixIndex, int thirdMatrixIndex, int semanticCategory);
ConstantSP eachColumnInPlace(Heap* heap, vector<ConstantSP>& args, SysFunc optr, int semanticCategory = 1, int objIndex = -1);
bool computeBinaryInplace(Heap* heap, const ConstantSP& a, const ConstantSP& b, ConstantSP& result, InplaceOptr optr, OptrFunc func);
bool computeTupleInplace(Heap* heap, const ConstantSP& a, const ConstantSP& b, ConstantSP& result, INDEX inputLen, InplaceOptr optr, OptrFunc func);
bool computeArrayVectorInplace(Heap* heap, const ConstantSP& a, const ConstantSP& b, ConstantSP& result, INDEX inputLen, InplaceOptr optr, OptrFunc func);

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
ConstantSP computeArrayVector(Heap* heap, const ConstantSP& a, const ConstantSP& b, OptrFunc optr);
ConstantSP computeArrayVector(Heap* heap, vector<ConstantSP>& args, SysFunc optr);
VectorSP copyIndexVectorOfArrayVector(const VectorSP& index);
bool checkArrayVectorSize(const ConstantSP& a, const ConstantSP& b);
string readLicenseFromFile(const string& licenseFile, const string& funcName);
}

#endif /* OPERATORIMP_H_ */
