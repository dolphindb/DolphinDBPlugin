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
	static double student(double df) { return InvCumDistrFunction::student(df, standardUniform());}
	static double f(double numeratorDF, double denominatorDF) { return InvCumDistrFunction::f(numeratorDF, denominatorDF, standardUniform()); }
	static double beta(double alpha, double beta) { return InvCumDistrFunction::beta(alpha, beta, standardUniform()); }
	static double gamma(double shape, double scale) { return InvCumDistrFunction::gamma(shape, scale, standardUniform()); }
	static double chiSquare(double df) { return InvCumDistrFunction::chiSquare(df, standardUniform()); }
	static double normal(double mean, double stdev) { return InvCumDistrFunction::normal(mean, stdev, standardUniform()); }
	static double exp(double mean) { return InvCumDistrFunction::exp(mean, standardUniform()); }
	static double uniform(double lower, double upper) { return InvCumDistrFunction::uniform(lower, upper, standardUniform()); }
	static double weibull(double alpha, double beta) { return InvCumDistrFunction::weibull(alpha, beta, standardUniform()); }
	static double zipf(int num, double exponent);
	static double logistic(double mu, double s) { return InvCumDistrFunction::logistic(mu, s, standardUniform()); }
	static double binomial(int trials, double prob) { return InvCumDistrFunction::binomial(trials, prob, standardUniform()); }
	static double poisson(double mean) { return InvCumDistrFunction::poisson(mean, standardUniform()); }
	static double kolmogorov();
private:
	static inline double standardUniform() {
		std::uniform_real_distribution<> distr(0, 1);
		return distr(*Util::m1);
	}
};

#endif /* DISTRIBUTION_H_ */
