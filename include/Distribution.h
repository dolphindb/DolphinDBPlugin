/*
 * Distribution.h
 *
 *  Created on: Dec 3, 2016
 *      Author: dzhou
 */

#ifndef DISTRIBUTION_H_
#define DISTRIBUTION_H_

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


#endif /* DISTRIBUTION_H_ */
