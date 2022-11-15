#include "svmPlugin.h"
#include <ScalarImp.h>
#include <Exceptions.h>
#include <sstream>
#include "svm.h"

ConstantSP fit(Heap *heap, vector<ConstantSP> &args){
    ConstantSP X = args[1], y = args[0];
    if(y->getForm() != DF_VECTOR || !y->isNumber())
        throw IllegalArgumentException(__FUNCTION__, "Y must be a numeric vector.");
    ConstantSP params = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
    if(args.size() == 3){
        params = args[2];
        if(params->getForm() != DF_DICTIONARY || ((DictionarySP)params)->getKeyType() != DT_STRING || ((DictionarySP)params)->getType() != DT_ANY)
            throw IllegalArgumentException(__FUNCTION__, "params must be a dictionary with STRING-ANY key-value type.");
    }
    if(X->getForm() == DF_MATRIX){
        if(!X->isNumber())
            throw IllegalArgumentException(__FUNCTION__, "X must be a matrix.");

    }else if(X->getForm() == DF_TABLE){
        const int n = X->columns();
        for(int i = 0;i < n;i++){
            if(!X->getColumn(i)->isNumber()){
                throw IllegalArgumentException(__FUNCTION__, "All columns of X must be of a number type.");
            }
        }
    }else if(X->getForm() == DF_VECTOR){
        if(!X->isNumber())
            throw IllegalArgumentException(__FUNCTION__, "X must be a numeric vector.");
    }else {
        throw IllegalArgumentException(__FUNCTION__, "X must be a numeric matrix, table or vector.");
    }
    return svm::fit(heap, y, X, params, X->getForm());
}

ConstantSP predict(Heap *heap, vector<ConstantSP> &args){
    ConstantSP model = args[0], X = args[1];
    if(model->getType() != DT_RESOURCE || model->getString() != "SVM model")
        throw IllegalArgumentException(__FUNCTION__, "model must be a svm-model.");
    if(X->getForm() == DF_MATRIX){
        if(!X->isNumber())
            throw IllegalArgumentException(__FUNCTION__, "X must be a numeric matrix.");
    }
    else if(X->getForm() == DF_TABLE){
        const int n = X->columns();
        for(int i = 0;i < n;i++){
            if(!X->getColumn(i)->isNumber()){
                throw IllegalArgumentException(__FUNCTION__, "All columns of X must be of a numeric type.");
            }
        }
    }else if(X->getForm() == DF_VECTOR){
        if(!X->isNumber())
            throw IllegalArgumentException(__FUNCTION__, "X must be a numeric vector");
    }else{
        throw IllegalArgumentException(__FUNCTION__, "X must be a numeric matrix, table or vector.");
    }
    return svm::predict(model, X, X->getForm());
}

ConstantSP score(Heap *heap, vector<ConstantSP> &args){
    ConstantSP model = args[0], X = args[2], y = args[1];
    if(model->getType() != DT_RESOURCE || model->getString() != "SVM model")
        throw IllegalArgumentException(__FUNCTION__, "model must be a svm-model.");

    if(y->getForm() != DF_VECTOR || !y->isNumber())
        throw IllegalArgumentException(__FUNCTION__, "Y must be a numeric vector.");

    if(X->getForm() == DF_MATRIX){
        if(!X->isNumber())
            throw IllegalArgumentException(__FUNCTION__, "X must be a matrix.");
    }else if(X->getForm() == DF_TABLE){
        const int n = X->columns();
        for(int i = 0;i < n;i++){
            if(!X->getColumn(i)->isNumber()){
                throw IllegalArgumentException(__FUNCTION__, "All columns of X must be of a numeric type.");
            }
        }
    }else if(X->getForm() == DF_VECTOR){
        if(!X->isNumber())
            throw IllegalArgumentException(__FUNCTION__, "X must be a numeric vector.");
    }
    else
        throw IllegalArgumentException(__FUNCTION__, "X must be a numeric matrix, table or vector.");
    return svm::score(heap, model,y, X);
}

ConstantSP saveModel(Heap *heap, vector<ConstantSP> &args){
    ConstantSP model = args[0], location = args[1];
    if(model->getType() != DT_RESOURCE || model->getString() != "SVM model")
        throw IllegalArgumentException(__FUNCTION__, "model must be a SVM-model.");
    if(location->getType() != DT_STRING || !location->isScalar())
        throw IllegalArgumentException(__FUNCTION__, "path must be a string.");
    return svm::saveModel(model, location);
}

ConstantSP loadModel(Heap *heap, vector<ConstantSP> &args){
    ConstantSP location = args[0];
    if(location->getType() != DT_STRING || !location->isScalar())
        throw IllegalArgumentException(__FUNCTION__, "path must be a string.");
    return svm::loadModel(heap, location);
}

namespace svm{
    void setParam(svm &, const ConstantSP &);
    void svmObjectClose(Heap *heap, vector<ConstantSP> &args);
    void print_null(const char *s) {}
    ConstantSP fit(Heap * &heap, const ConstantSP &y, const ConstantSP &X, const ConstantSP &para, DATA_FORM df){
        svm * psvmObject = new svm();
        setParam(*psvmObject, para);

        double bufX[Util::BUF_SIZE], bufy[Util::BUF_SIZE];
        const int n = y->size();
        int lenX = 0, startX = 0, preX = 0, leny = 0, starty = 0, prey = 0;
        const double * pX = nullptr, * py = nullptr;
        if(df == DF_MATRIX || df == DF_VECTOR){
            const int m = X->size() / n;
            for (int i=0; i<n; i++){
                vector<pair<int, double> > v;
                for(int j = 0;j < m;j++){
                    if(startX >= lenX) {
                        preX += lenX;
                        lenX = std::min(Util::BUF_SIZE, X->size() - preX);
                        pX = X->getDoubleConst(preX, lenX, bufX);
                        startX = 0;
                    }
                    v.push_back(std::make_pair(j, pX[startX++]));
                }
                if(starty >= leny) {
                    prey += leny;
                    leny = std::min(Util::BUF_SIZE, y->size() - prey);
                    py = y->getDoubleConst(prey, leny, bufy);
                    starty = 0;
                }
                psvmObject->add_train_data(py[starty++], v);
            }
        }else if(df == DF_TABLE){
            TableSP pt = X;
            std::vector<ConstantSP> cols;
            for(int i = 0;i < pt->columns();i++)
                cols.push_back(pt->getColumn(i));
            for (int i=0; i < pt->rows(); i++){
                vector<pair<int, double> > v;
                for(int j = 0;j < pt->columns();j++){
                    v.push_back(std::make_pair(j, cols[j]->get(i)->getDouble()));
                }
                if(starty >= leny) {
                    prey += leny;
                    leny = std::min(Util::BUF_SIZE, y->size() - prey);
                    py = y->getDoubleConst(prey, leny, bufy);
                    starty = 0;
                }
                psvmObject->add_train_data(py[starty++], v);
            }
        }

        psvmObject->train();

        FunctionDefSP onClose(Util::createSystemProcedure("SVM Object deconstruct", svmObjectClose, 1, 1));
        return Util::createResource(reinterpret_cast<long long>(psvmObject),"SVM model", onClose, heap->currentSession());
    }


    ConstantSP predict(const ConstantSP &model, const ConstantSP &X, DATA_FORM df){
        svm *psvmObject = reinterpret_cast<svm *>(model->getLong());
        if(psvmObject == nullptr){
            throw IllegalArgumentException(__FUNCTION__, "Not a illegal SVM object");
        }
        const int m = psvmObject->get_nr_class();
        ConstantSP y;
        if(df == DF_VECTOR || df == DF_MATRIX){
            const int n = X->size() / m;
            y = Util::createVector(DT_DOUBLE, n);
            double bufX[Util::BUF_SIZE];
            int lenX = 0, startX = 0, preX = 0;
            const double * pX = nullptr;
            for(int i = 0;i < n;i++){
                vector<pair<int, double> > v;
                for(int j = 0;j < m;j++){
                    if (startX >= lenX) {
                        preX += lenX;
                        lenX = std::min(Util::BUF_SIZE, X->size() - preX);
                        pX = X->getDoubleConst(preX, lenX, bufX);
                        startX = 0;
                    }
                    v.push_back(std::make_pair(j, pX[startX++]));
                }
                double res = psvmObject->predict(v);
                y->setDouble(i, res);
            }
        }else if(df == DF_TABLE){
            TableSP pt = X;
            const int n = pt->rows();
            y = Util::createVector(DT_DOUBLE, n);
            std::vector<ConstantSP> cols;
            for(int i = 0;i < pt->columns();i++)
                cols.push_back(pt->getColumn(i));

            for (int i=0; i < n; i++){
                vector<pair<int, double> > v;
                for(int j = 0;j < pt->columns();j++){
                    v.push_back(std::make_pair(j, cols[j]->get(i)->getDouble()));
                }
                double res = psvmObject->predict(v);
                y->setDouble(i, res);
            }
        }

        return y;
    }


    ConstantSP score(Heap * &heap, const ConstantSP &model, const ConstantSP &y, const ConstantSP &X){
        svm *psvmObject = reinterpret_cast<svm *>(model->getLong());
        if(psvmObject == nullptr){
            throw IllegalArgumentException(__FUNCTION__, "Not a illegal SVM object");
        }

        const int n = y->size();
        int svm_type = psvmObject->get_svm_type();
        int correct = 0;
        int total = 0;
        double error = 0;
        double sump = 0, sumt = 0, sumpp = 0, sumtt = 0, sumpt = 0;

        ConstantSP predict_labels = predict(model, X, X->getForm());
        double bufX[Util::BUF_SIZE],bufy[Util::BUF_SIZE];
        const double * pX = nullptr, * py = nullptr;
        int lenX = 0, startX = 0, preX = 0,leny = 0, starty = 0, prey = 0;
        for(int i = 0;i < n;i++){
            if(startX >= lenX) {
                preX += lenX;
                lenX = std::min(Util::BUF_SIZE, n - preX);
                pX = predict_labels->getDoubleConst(preX, lenX, bufX);
                startX = 0;
            }
            if(starty >= leny) {
                prey += leny;
                leny = std::min(Util::BUF_SIZE, n - prey);
                py = y->getDoubleConst(prey, leny, bufy);
                starty = 0;
            }
            double predict_label = pX[startX++], target_label = py[starty++];
            if(predict_label == target_label)
                ++correct;
            error += (predict_label-target_label)*(predict_label-target_label);
            sump += predict_label;
            sumt += target_label;
            sumpp += predict_label*predict_label;
            sumtt += target_label*target_label;
            sumpt += predict_label*target_label;
            ++total;
        }

        std::stringstream ss;
        if (svm_type==NU_SVR || svm_type==EPSILON_SVR) {
            DictionarySP res = Util::createDictionary(DT_STRING, nullptr, DT_DOUBLE, nullptr);
            res->set("MSE", new Double(error/total));
            res->set("R2", new Double(((total*sumpt-sump*sumt)*(total*sumpt-sump*sumt)) / ((total*sumpp-sump*sump)*(total*sumtt-sumt*sumt))));
            return res;
        }
        else {
            double accuracy = (double)correct/total;
            return new Double(accuracy);
        }
    }

    ConstantSP loadModel(Heap *heap, const ConstantSP &location){
        string spath = location->getString();

        svm * psvmObject = new svm();
        psvmObject->load(spath);

        FunctionDefSP onClose(Util::createSystemProcedure("SVM model onClose()", svmObjectClose, 1, 1));
        return Util::createResource(reinterpret_cast<long long>(psvmObject),"SVM model", onClose, heap->currentSession());
    }

    ConstantSP saveModel(const ConstantSP &model, const ConstantSP &location){
        string funcName = "saveModel";
        string spath = location->getString();
        svm * psvmObject = reinterpret_cast<svm *>(model->getLong());
        if(psvmObject == nullptr){
            throw IllegalArgumentException(funcName, "model is invalid");
        }
        psvmObject->dump(spath);
        return ConstantSP(new Bool(true));
    }

    void setParam(svm &svmObject, const ConstantSP &para){
        struct svm_parameter param;
        param.svm_type = C_SVC;
        param.kernel_type = RBF;
        param.degree = 3;
        param.gamma = -1; // 1/num_features
        param.coef0 = 0;
        param.nu = 0.5;
        param.cache_size = 100;
        param.C = 1;
        param.eps = 1e-3;
        param.p = 0.1;
        param.shrinking = 1;
        param.probability = 0;
        param.nr_weight = 0;
        param.weight_label = NULL;
        param.weight = NULL;
        ConstantSP keys = para->keys();
        for(int i = 0;i < keys->size();i++){
            ConstantSP key = keys->get(i);
            ConstantSP value = para->getMember(key);
            string skey = key->getString();
            if(skey == "type"){
                string svalue = value->getString();
                if(svalue == "NuSVC")
                    param.svm_type = NU_SVC;
                else if(svalue == "NuSVR")
                    param.svm_type = NU_SVR;
                else if(svalue == "OneClass")
                    param.svm_type = ONE_CLASS;
                else if(svalue == "SVC")
                    param.svm_type = C_SVC;
                else if(svalue == "SVR")
                    param.svm_type = EPSILON_SVR;
                else
                    throw IllegalArgumentException(__FUNCTION__, "the value of key 'type' is invalid");
            }
            else if(skey == "kernel"){
                string svalue = value->getString();
                if(svalue == "linear")
                    param.kernel_type = 0;
                else if(svalue == "poly")
                    param.kernel_type = 1;
                else if(svalue == "rbf")
                    param.kernel_type = 2;
                else if(svalue == "sigmoid")
                    param.kernel_type = 3;
                else if(svalue == "precomputed")
                    param.kernel_type = 4;
                else
                    throw IllegalArgumentException(__FUNCTION__, "the value of key 'kernel' is invalid");
            }
            else if(skey == "degree")
                param.degree = value->getInt();
            else if(skey == "gamma"){
                if(value->getString() == "scale")
                    param.gamma = -1;
                else
                    param.gamma = value->getDouble();
            }
            else if(skey == "coef0")
                param.coef0 = value->getDouble();
            else if(skey == "C")
                param.C = value->getDouble();
            else if(skey == "epsilon")
                param.eps = value->getDouble();
            else if(skey == "shrinking")
                param.shrinking = (value->getBool()) ? 1 : 0;
            else if(skey == "cache_size")
                param.cache_size = value->getDouble();
            else if(skey == "verbose") {
                if(value->getBool() == 0)
                    ::svm_set_print_string_function(print_null);
            }
            else if(skey == "nu")
                param.nu = value->getDouble();
            else
                throw IllegalArgumentException(__FUNCTION__, "unknown key " + skey);
        }
        svmObject.set_para(param);
    }

    void svmObjectClose(Heap *heap, vector<ConstantSP> &args){
        svm * svmObject = reinterpret_cast<svm *>(args[0]->getLong());
        if(svmObject != nullptr){
            delete svmObject;
            args[0]->setLong(0);
        }
    }
}