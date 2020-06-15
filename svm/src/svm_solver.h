#include <iostream>
#include <fstream>
#include <strstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <iomanip>

#include "svm_common.h"
#include "svm_option.h"

#ifndef SVM_SOLVER_H
#define SVM_SOLVER_H

#define TOLERANCE 1e-6

class SVMSolver{
public:
    SVMSolver(SVMOption *opt);
    ~SVMSolver();
    int train();
    double predict();

protected:

    float error_rate();

    int load_model(std::ifstream& is);
    int dump_model(std::ofstream& os);
    
    float kernel(int i1, int i2);
    float learned_func(int k);

    int examine_example(int i1);
    int take_step(int i1, int i2);

protected:

    // input options
    float _c;
    float _eps;
    float _sig;
    bool _is_linear_kernel;
    const char *_fname_train;
    const char *_fname_valid;
    const char *_fname_model;
    
    // internal options 
    int _n;
    int _n_sv;
    TVectorArray _x_array;
    TFloatArray _y_array;
    TVector _w;

    TFloatArray _alpha;
    TFloatArray _d;
    TFloatArray _error_cache;
    float _b;
    float _delta_b;
    
};

#endif
