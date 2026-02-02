#ifndef TR1_RANDOM_COMPAT_H
#define TR1_RANDOM_COMPAT_H

// Compatibility header for tr1/random
// On Linux with GCC 11+, we use std:: directly instead of std::tr1::

#include <random>

// For Linux GCC 11+, we don't need to define std::tr1 namespace
// as the code should use std:: directly
namespace std {
namespace tr1 {
    // Just import everything from std:: to std::tr1:: for compatibility
    using std::mt19937;
    using std::mt19937_64;
} // namespace tr1
} // namespace std

#endif // TR1_RANDOM_COMPAT_H
