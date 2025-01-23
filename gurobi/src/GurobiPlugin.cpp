//
// Created by htxu on 12/1/2023.
//

#include "GurobiPlugin.h"

#include <ScalarImp.h>

#include "ddbplugin/Plugin.h"
#include "gurobi_c++.h"

/// Resource Related

// descriptions
const string GUROBI_PREFIX = "[Plugin::Gurobi]";
const string GUROBI_MODEL_DESC = "gurobi model";
const string GUROBI_LIN_EXPRESSION_DESC = "gurobi linear expression";
const string GUROBI_QUAD_EXPRESSION_DESC = "gurobi quadratic expression";

// maps
dolphindb::ResourceMap<GRBModel> GRB_MODEL_AMP(GUROBI_PREFIX, GUROBI_MODEL_DESC);
dolphindb::ResourceMap<GRBLinExpr> GRB_LIN_EXPRESSION_AMP(GUROBI_PREFIX, GUROBI_LIN_EXPRESSION_DESC);
dolphindb::ResourceMap<GRBQuadExpr> GRB_QUAD_EXPRESSION_AMP(GUROBI_PREFIX, GUROBI_QUAD_EXPRESSION_DESC);

// close functions
void modelOnClose(Heap *heap, vector<ConstantSP> &args) {}
void linExpressionOnClose(Heap *heap, vector<ConstantSP> &args) {}
void quadExpressionOnClose(Heap *heap, vector<ConstantSP> &args) {}

/// Helper Declarations

VectorSP getNumVector(const SmartPointer<Constant> &arg, const string &funcName, const string &usage,
                      const string &argName, int size = 0);
VectorSP getCharVector(const SmartPointer<Constant> &arg, const string &funcName, const string &usage,
                       const string &argName, int size = 0);
VectorSP getStringVector(const SmartPointer<Constant> &arg, const string &funcName, const string &usage,
                         const string &argName, int size = 0);
VectorSP getQuadMatrix(const SmartPointer<Constant> &arg, const string &funcName, const string &usage,
                       const string &argName, int size = 0);
char getCharScalar(SmartPointer<Constant> &arg, const string &funcName, const string &usage, const string &argName);
double getDoubleScalar(SmartPointer<Constant> &arg, const string &funcName, const string &usage, const string &argName);

/// Interfaces

ConstantSP gurobiModel(Heap *heap, vector<ConstantSP> &args) {
    string usage = GUROBI_PREFIX + " model([params]): ";

    DictionarySP params;
    if (args.size() == 1) {
        if (args[0].isNull() or args[0]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(__FUNCTION__, usage + "params should be a dictionary.");
        }
        params = args[0];
        if (params->getKeyType() != DT_STRING || params->getType() != DT_STRING) {
            throw IllegalArgumentException(
                __FUNCTION__, usage + "params should be a dictionary with string type of keys and values.");
        }
    }

    SmartPointer<GRBModel> model;
    try {
        auto env = GRBEnv(true);
        env.set(GRB_IntParam_OutputFlag, 0);
        if (!params.isNull()) {
            auto keys = params->keys();
            for (auto i = 0; i < keys->size(); ++i) {
                auto param = keys->getString(i);
                env.set(param, params->getMember(param)->getString());
            }
        }
        env.start();
        model = new GRBModel(env);
        model->update();
    } catch (GRBException &e) {
        throw RuntimeException(GUROBI_PREFIX + " Error code = " + std::to_string(e.getErrorCode()) + "\n" +
                               e.getMessage());
    }

    FunctionDefSP onClose(Util::createSystemProcedure("modelOnClose", modelOnClose, 1, 1));
    ConstantSP resource = Util::createResource(reinterpret_cast<long long>(model.get()), GUROBI_MODEL_DESC, onClose,
                                               heap->currentSession());
    GRB_MODEL_AMP.safeAdd(resource, model);
    return resource;
}

ConstantSP gurobiAddVars(Heap *heap, vector<ConstantSP> &args) {
    string usage = GUROBI_PREFIX + " addVars(model, lb, ub, obj, type, varName): ";
    auto numVars = args[5]->size();  // use name as default size

    /// args
    // model
    auto model = GRB_MODEL_AMP.safeGet(args[0]);
    model->update();
    // lb
    auto lb = getNumVector(args[1], __FUNCTION__, usage, "lb", numVars);
    // ub
    auto ub = getNumVector(args[2], __FUNCTION__, usage, "ub", numVars);
    // obj
    VectorSP obj;
    if (!args[3]->isNull()) {
        obj = getNumVector(args[3], __FUNCTION__, usage, "obj", numVars);
    }
    // type
    VectorSP type;
    if (!args[4]->isNull()) {
        type = getCharVector(args[4], __FUNCTION__, usage, "type", numVars);
    }
    // varName
    auto varName = getStringVector(args[5], __FUNCTION__, usage, "varName", numVars);

    /// ddb vec to std vec
    vector<double> lbVec(numVars);
    vector<double> ubVec(numVars);
    vector<double> objVec;
    vector<char> typeVec;
    vector<string> varNameVec(numVars);
    for (auto i = 0; i < numVars; ++i) {
        lbVec[i] = lb->getDouble(i);
        ubVec[i] = ub->getDouble(i);
        varNameVec[i] = varName->getString(i);
    }
    if (!obj.isNull()) {
        objVec.reserve(numVars);
        for (auto i = 0; i < numVars; ++i) {
            objVec.emplace_back(obj->getDouble(i));
        }
    }
    if (!type.isNull()) {
        typeVec.reserve(numVars);
        for (auto i = 0; i < numVars; ++i) {
            typeVec.emplace_back(type->getChar(i));
        }
    }

    /// add vars
    try {
        model->addVars(lbVec.data(), ubVec.data(), objVec.data(), typeVec.data(), varNameVec.data(), numVars);
        model->update();
    } catch (GRBException &e) {
        throw RuntimeException(GUROBI_PREFIX + " Error code = " + std::to_string(e.getErrorCode()) + "\n" +
                               e.getMessage());
    }
    return varName;
}

ConstantSP gurobiLinExpr(Heap *heap, vector<ConstantSP> &args) {
    string usage = GUROBI_PREFIX + " linExpr(model, coefficient, varName): ";
    auto numVars = args[2]->size();

    /// args
    // model
    auto model = GRB_MODEL_AMP.safeGet(args[0]);
    model->update();
    // coefficient
    auto coefficient = getNumVector(args[1], __FUNCTION__, usage, "coefficient", numVars);
    // varName
    auto varName = getStringVector(args[2], __FUNCTION__, usage, "varName", numVars);

    /// create linear expression
    GRBLinExpr expr = 0;
    try {
        auto size = varName->size();
        for (auto i = 0; i < size; ++i) {
            auto c = coefficient->getDouble(i);
            auto var = model->getVarByName(varName->getString(i));

            if (c != 0) {
                expr += c * var;
            }
        }
    } catch (GRBException &e) {
        throw RuntimeException(GUROBI_PREFIX + " Error code = " + std::to_string(e.getErrorCode()) + "\n" +
                               e.getMessage());
    }

    SmartPointer<GRBLinExpr> linExpr = new GRBLinExpr(expr);
    FunctionDefSP onClose(Util::createSystemProcedure("linExpressionOnClose", linExpressionOnClose, 1, 1));
    ConstantSP resource = Util::createResource(reinterpret_cast<long long>(linExpr.get()), GUROBI_LIN_EXPRESSION_DESC,
                                               onClose, heap->currentSession());
    GRB_LIN_EXPRESSION_AMP.safeAdd(resource, linExpr);
    return resource;
}

ConstantSP gurobiQuadExpr(Heap *heap, vector<ConstantSP> &args) {
    string usage = GUROBI_PREFIX + " quadExpr(model, quadMatrix, varNames, [linExpr]): ";
    auto numVars = args[2]->size();

    /// args
    auto model = GRB_MODEL_AMP.safeGet(args[0]);
    model->update();
    // quadMatrix
    // optimization: other forms or types - table, vector, etc.
    auto quadMatrix = getQuadMatrix(args[1], __FUNCTION__, usage, "quadMatrix", numVars);
    // varName
    auto varName = getStringVector(args[2], __FUNCTION__, usage, "varName", numVars);
    // linExpr
    SmartPointer<GRBLinExpr> linExpr;
    if (args.size() == 4) {
        linExpr = GRB_LIN_EXPRESSION_AMP.safeGet(args[3]);
    }

    /// create quadratic expression
    GRBQuadExpr expr;
    try {
        if (linExpr.isNull()) {
            expr = GRBQuadExpr();
        } else {
            expr = GRBQuadExpr(*linExpr);
        }
        auto size = varName->size();
        for (auto i = 0; i < size; ++i) {
            VectorSP col = quadMatrix->getColumn(i);
            auto iVar = model->getVarByName(varName->getString(i));
            for (auto j = 0; j < size; j++) {
                auto coefficient = col->getDouble(j);
                if (coefficient != 0) {
                    auto jVar = model->getVarByName(varName->getString(j));
                    expr += coefficient * iVar * jVar;
                }
            }
        }
    } catch (GRBException &e) {
        throw RuntimeException(GUROBI_PREFIX + " Error code = " + std::to_string(e.getErrorCode()) + "\n" +
                               e.getMessage());
    }

    SmartPointer<GRBQuadExpr> quadExpr = new GRBQuadExpr(expr);
    FunctionDefSP onClose(Util::createSystemProcedure("quadExpressionOnClose", quadExpressionOnClose, 1, 1));
    ConstantSP resource = Util::createResource(reinterpret_cast<long long>(quadExpr.get()), GUROBI_QUAD_EXPRESSION_DESC,
                                               onClose, heap->currentSession());
    GRB_QUAD_EXPRESSION_AMP.safeAdd(resource, quadExpr);
    return resource;
}

ConstantSP gurobiAddConstr(Heap *heap, vector<ConstantSP> &args) {
    string usage = GUROBI_PREFIX + " addConstr(model, expr, sense, rhsVal): ";

    /// args
    // model
    auto model = GRB_MODEL_AMP.safeGet(args[0]);
    model->update();
    // expr
    auto expr = args[1];
    bool isQuad = false;
    if (expr->getString() == GUROBI_QUAD_EXPRESSION_DESC) {
        isQuad = true;
    }
    // sense
    auto sense = getCharScalar(args[2], __FUNCTION__, usage, "sense");
    // rhsVal
    auto rhsVal = getDoubleScalar(args[3], __FUNCTION__, usage, "rhsVal");

    /// add constraint
    try {
        if (isQuad) {
            model->addQConstr(*GRB_QUAD_EXPRESSION_AMP.safeGet(expr), sense, rhsVal);
        } else {
            model->addConstr(*GRB_LIN_EXPRESSION_AMP.safeGet(expr), sense, rhsVal);
        }
        model->update();

    } catch (GRBException &e) {
        throw RuntimeException(GUROBI_PREFIX + " Error code = " + std::to_string(e.getErrorCode()) + "\n" +
                               e.getMessage());
    }
    return new Void();
}

ConstantSP gurobiSetObjective(Heap *heap, vector<ConstantSP> &args) {
    string usage = GUROBI_PREFIX + " setObjective(model, expr, sense): ";

    /// args
    // model
    auto model = GRB_MODEL_AMP.safeGet(args[0]);
    model->update();
    // expr
    auto expr = args[1];
    bool isQuad = false;
    if (expr->getString() == GUROBI_QUAD_EXPRESSION_DESC) {
        isQuad = true;
    }
    // sense
    if (args[2].isNull() or args[2]->getForm() != DF_SCALAR or args[2]->getType() != DT_INT) {
        throw IllegalArgumentException(__FUNCTION__, usage + " sense should be an int.");
    }
    int sense = args[2]->getInt();
    if (sense != GRB_MINIMIZE && sense != GRB_MAXIMIZE) {
        throw IllegalArgumentException(__FUNCTION__, usage + " sense should be 1 or -1.");
    }

    try {
        if (isQuad) {
            model->setObjective(*GRB_QUAD_EXPRESSION_AMP.safeGet(expr), sense);
        } else {
            model->setObjective(*GRB_LIN_EXPRESSION_AMP.safeGet(expr), sense);
        }
        model->update();
    } catch (GRBException &e) {
        throw RuntimeException(GUROBI_PREFIX + " Error code = " + std::to_string(e.getErrorCode()) + "\n" +
                               e.getMessage());
    }
    return new Void();
}

ConstantSP gurobiOptimize(Heap *heap, vector<ConstantSP> &args) {
    auto model = GRB_MODEL_AMP.safeGet(args[0]);
    model->update();

    int status;
    try {
        model->optimize();
        model->update();
        status = model->get(GRB_IntAttr_Status);
    } catch (GRBException &e) {
        throw RuntimeException(GUROBI_PREFIX + " Error code = " + std::to_string(e.getErrorCode()) + "\n" +
                               e.getMessage());
    }
    return new Int(status);
}

ConstantSP gurobiGetResult(Heap *heap, vector<ConstantSP> &args) {
    auto model = GRB_MODEL_AMP.safeGet(args[0]);
    model->update();

    DictionarySP result = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr, true);
    try {
        auto numVars = model->get(GRB_IntAttr_NumVars);
        auto vars = model->getVars();
        for (auto i = 0; i < numVars; ++i) {
            auto &var = vars[i];
            auto varName = var.get(GRB_StringAttr_VarName);
            auto varVal = var.get(GRB_DoubleAttr_X);
            result->set(varName, new Double(varVal));
        }
    } catch (GRBException &e) {
        throw RuntimeException(GUROBI_PREFIX + " Error code = " + std::to_string(e.getErrorCode()) + "\n" +
                               e.getMessage());
    }
    return result;
}

ConstantSP gurobiGetObjective(Heap *heap, vector<ConstantSP> &args) {
    try {
        auto model = GRB_MODEL_AMP.safeGet(args[0]);
        model->update();
        return new Double(model->get(GRB_DoubleAttr_ObjVal));
    } catch (GRBException &e) {
        throw RuntimeException(GUROBI_PREFIX + " Error code = " + std::to_string(e.getErrorCode()) + "\n" +
                               e.getMessage());
    }
}

/// Helper Implementations

VectorSP getNumVector(const SmartPointer<Constant> &arg, const string &funcName, const string &usage,
                      const string &argName, int size) {
    if (arg.isNull() or arg->getForm() != DF_VECTOR or (arg->getType() != DT_INT and arg->getType() != DT_DOUBLE)) {
        throw IllegalArgumentException(funcName, usage + argName + " should be a vector of int or double.");
    }
    if (arg->size() != size) {
        throw IllegalArgumentException(funcName, usage + "size of " + argName + " is different from size of varName.");
    }
    if (arg->hasNull()) {
        throw IllegalArgumentException(funcName, usage + argName + " can't contain null value.");
    }
    return arg;
}

VectorSP getCharVector(const SmartPointer<Constant> &arg, const string &funcName, const string &usage,
                       const string &argName, int size) {
    if (arg.isNull() or arg->getForm() != DF_VECTOR or arg->getType() != DT_CHAR) {
        throw IllegalArgumentException(funcName, usage + argName + " should be a vector of char.");
    }
    if (arg->size() != size) {
        throw IllegalArgumentException(funcName, usage + "size of " + argName + " is different from size of varName.");
    }
    if (arg->hasNull()) {
        throw IllegalArgumentException(funcName, usage + argName + " can't contain null value.");
    }
    return arg;
}

VectorSP getStringVector(const SmartPointer<Constant> &arg, const string &funcName, const string &usage,
                         const string &argName, int size) {
    if (arg.isNull() or arg->getForm() != DF_VECTOR or arg->getType() != DT_STRING) {
        throw IllegalArgumentException(funcName, usage + argName + " should be a vector of string.");
    }
    if (arg->size() != size) {
        throw IllegalArgumentException(funcName, usage + "size of " + argName + " is different from size of varName.");
    }
    if (arg->hasNull()) {
        throw IllegalArgumentException(funcName, usage + argName + " can't contain null value.");
    }
    return arg;
}

VectorSP getQuadMatrix(const SmartPointer<Constant> &arg, const string &funcName, const string &usage,
                       const string &argName, int size) {
    // check form
    auto form = arg->getForm();
    if (arg.isNull() or form != DF_MATRIX) {
        throw IllegalArgumentException(funcName, usage + argName + " should be a matrix.");
    }
    // check size
    auto argSize = arg->columns();
    if (argSize != size) {
        throw IllegalArgumentException(funcName, usage + "size of " + argName + " is different from size of varName.");
    }
    // check columns
    for (auto i = 0; i < size; ++i) {
        getNumVector(arg->getColumn(i), funcName, usage, "member of " + argName, argSize);
    }

    if (arg->hasNull()) {
        throw IllegalArgumentException(funcName, usage + argName + " can't contain null value.");
    }
    return arg;
}

char getCharScalar(SmartPointer<Constant> &arg, const string &funcName, const string &usage, const string &argName) {
    if (arg.isNull() or arg->getForm() != DF_SCALAR or arg->getType() != DT_CHAR) {
        throw IllegalArgumentException(funcName, usage + argName + " should be a char.");
    }
    return arg->getChar();
}

double getDoubleScalar(SmartPointer<Constant> &arg, const string &funcName, const string &usage,
                       const string &argName) {
    if (arg.isNull() or arg->getForm() != DF_SCALAR or (arg->getType() != DT_DOUBLE and arg->getType() != DT_INT)) {
        throw IllegalArgumentException(funcName, usage + argName + " should be a double.");
    }
    return arg->getDouble();
}
