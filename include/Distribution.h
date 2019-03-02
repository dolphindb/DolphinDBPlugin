/*
 * Distribution.h
 *
 *  Created on: Dec 3, 2016
 *      Author: dzhou
 */

#ifndef DISTRIBUTION_H_
#define DISTRIBUTION_H_

#include <random>

#include "CoreConcept.h"
#include "Util.h"

class CumDistrFunction{
public:
	static double student(double df, double x);
	static double f(double numeratorDF, double denominatorDF, double x);
	static double beta(double alpha, double beta, double x);
	static double gamma(double shape, double scale, double x);
	static double chiSquare(double df, double x);
	static double normal(double mean, double stdev, double x);
	static double exp(double mean, double x);
	static double uniform(double lower, double upper, double x);
	static double weibull(double alpha, double beta, double x);
	static double zipf(int num, double exponent, double x);
	static double logistic(double mu, double s, double x);
	static double binomial(int trials, double p, int x);
	static double poisson(double mean, int x);
	static double kolmogorov(double x);

private:
    static double generalizedHarmonic(int n, double m);
};

class InvCumDistrFunction{
public:
	static double student(double df, double p);
	static double f(double numeratorDF, double denominatorDF, double p);
	static double beta(double alpha, double beta, double p);
	static double gamma(double shape, double scale, double p);
	static double chiSquare(double df, double p);
	static double normal(double mean, double stdev, double p);
	static double exp(double mean, double p);
	static double uniform(double lower, double upper, double p);
	static double weibull(double alpha, double beta, double p);
	static double zipf(int num, double exponent, double p);
	static double logistic(double mu, double s, double p);
	static double binomial(int trials, double prob, double p);
	static double poisson(double mean, double p);
	static double kolmogorov(double p);
};

class RandomSample {
public:
	static void student(double df, double *buf, int n);
	static void f(double numeratorDF, double denominatorDF, double *buf, int n);
	static void beta(double alpha, double beta, double *buf, int n);
	static void gamma(double shape, double scale, double *buf, int n);
	static void chiSquare(double df, double *buf, int n);
	static void normal(double mean, double stdev, double *buf, int n);
	static void exp(double mean, double *buf, int n);
	static void uniform(double lower, double upper, double *buf, int n);
	static void weibull(double alpha, double beta, double *buf, int n);
	static void zipf(int num, double exponent, double *buf, int n);
	static void logistic(double mu, double s, double *buf, int n);
	static void binomial(int trials, double prob, double *buf, int n);
	static void poisson(double mean, double *buf, int n);
};

#endif /* DISTRIBUTION_H_ */
