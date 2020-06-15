
#include "svm_common.h"
#include "svm_option.h"

using std::cout;
using std::cerr;
using std::endl;

SVMOption::SVMOption(){
    _c = SVM_OPTION_C;
    _eps = SVM_OPTION_EPSILON;
    _sig = SVM_OPTION_SIGMA;
    _is_linear_kernel = false;
    _fname_train[0] = '\0';
    _fname_valid[0] = '\0';
    _fname_model[0] = '\0';
    _flags = 0;
}

SVMOption::~SVMOption(){}

void SVMOption::print(){
    cerr << 
        "#========================================================\n"
        "# SVM Option: \n"
        "#========================================================\n"
        "# [c = " << _c << "]\n"
        "# [epsilon = " << _eps << "]\n"
        "# [sigma = " << _sig << "]\n"
        "# [is_linear_kernel = " << _is_linear_kernel << "]\n"
        "# [fname_train = " << _fname_train << "]\n"
        "# [fname_model = " << _fname_model << "]\n"
        "# [fname_valid = " << _fname_valid << "]\n"
        "#========================================================\n"
    << endl; 
}

int SVMOption::parse_command_line(int argc, char *argv[]){
    int option;
    const char *opt_string = "";
    struct option long_opts[] = {
        {"train",           1, NULL, 0},
        {"model",           1, NULL, 1},
        {"validate",        1, NULL, 2},
        {"linear_kernel",   0, NULL, 3},
        {"c",               1, NULL, 4},
        {"epsilon",         1, NULL, 5},
        {"sigma",           1, NULL, 6},
        {"help",            0, NULL, 7},
        {0, 0, 0, 0}
    };
 
    while ((option = getopt_long_only(argc, argv, opt_string, long_opts, NULL)) != -1){
        switch (option) {
            case 0:
                _flags |= FLAG_TRAIN;
                memcpy(_fname_train, optarg, strlen(optarg));
                _fname_train[strlen(optarg)] = '\0';
                break;
            case 1:
                _flags |= FLAG_MODEL;
                memcpy(_fname_model, optarg, strlen(optarg));
                _fname_model[strlen(optarg)] = '\0';
                break;
            case 2:
                _flags |= FLAG_VALID;
                memcpy(_fname_valid, optarg, strlen(optarg));
                _fname_valid[strlen(optarg)] = '\0';
                break;
            case 3:
                _flags |= FLAG_LINEAR_KERNEL;
                _is_linear_kernel = true;
                break;
            case 4:
                _flags |= FLAG_C;
                _c = atof(optarg);
                break;
            case 5:
                _flags |= FLAG_EPSILON;
                _eps = atof(optarg);
                break;
            case 6:
                _flags |= FLAG_SIGMA;
                _sig = atof(optarg);
                break;
            case 7:
                _flags |= FLAG_HELP;
                break;
        }
    }

    if (!(_flags & FLAG_TRAIN)) return FLAG_TRAIN;
    if (!(_flags & FLAG_VALID)) return FLAG_VALID;
    if (!(_flags & FLAG_MODEL)) return FLAG_MODEL;
    return 0;
}
