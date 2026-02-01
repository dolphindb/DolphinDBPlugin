#ifndef UTIL_WRAPPER_H
#define UTIL_WRAPPER_H

// Wrapper for Util.h to fix tr1/random compatibility issue
// This header must be included before Util.h

#include <random>

// Define tr1/random compatibility
namespace std {
namespace tr1 {

using std::linear_congruential_engine;
using std::mersenne_twister_engine;
using std::subtract_with_carry_engine;
using std::discard_block_engine;
using std::independent_bits_engine;
using std::shuffle_order_engine;

using std::minstd_rand0;
using std::minstd_rand;
using std::mt19937;
using std::mt19937_64;
using std::ranlux24_base;
using std::ranlux48_base;
using std::ranlux24;
using std::ranlux48;
using std::knuth_b;

using std::uniform_int_distribution;
using std::uniform_real_distribution;
using std::normal_distribution;
using std::bernoulli_distribution;
using std::binomial_distribution;
using std::geometric_distribution;
using std::negative_binomial_distribution;
using std::poisson_distribution;
using std::exponential_distribution;
using std::gamma_distribution;
using std::weibull_distribution;
using std::extreme_value_distribution;
using std::lognormal_distribution;
using std::chi_squared_distribution;
using std::cauchy_distribution;
using std::fisher_f_distribution;
using std::student_t_distribution;
using std::discrete_distribution;
using std::piecewise_constant_distribution;
using std::piecewise_linear_distribution;

} // namespace tr1
} // namespace std

// Now include the original Util.h
#include "Util.h"

#endif // UTIL_WRAPPER_H
