//
// Created by htxu on 12/14/2023.
//

#ifndef PLUGINNSQ_PLUGINUTIL_H
#define PLUGINNSQ_PLUGINUTIL_H


#include <CoreConcept.h>

using namespace ddb;
/**
 * Util functions across plugins
 */
namespace pluginUtil {

    /// Arg Parsers

    inline char getCharScalar(const SmartPointer<Constant> &arg, const string &argName, const string &funcName,
                              const string &usage) {

        if (arg.isNull() or arg->getForm() != DF_SCALAR or arg->getType() != DT_CHAR) {
            throw IllegalArgumentException(funcName, usage + argName + " should be a char.");
        }
        return arg->getChar();
    }

    inline double getNumScalar(const SmartPointer<Constant> &arg, const string &argName, const string &funcName,
                               const string &usage) {

        if (arg.isNull() or arg->getForm() != DF_SCALAR or (arg->getType() != DT_DOUBLE and arg->getType() != DT_INT)) {
            throw IllegalArgumentException(funcName, usage + argName + " should be a double.");
        }
        return arg->getDouble();
    }

    inline int getIntScalar(const SmartPointer<Constant> &arg, const string &argName, const string &funcName,
                            const string &usage) {

        if (arg.isNull() or arg->getForm() != DF_SCALAR or arg->getType() != DT_INT) {
            throw IllegalArgumentException(funcName, usage + argName + " should be an int.");
        }
        return arg->getInt();
    }

    inline string getStringScalar(const SmartPointer<Constant> &arg, const string &argName, const string &funcName,
                                  const string &usage) {

        if (arg.isNull() or arg->getForm() != DF_SCALAR or arg->getType() != DT_STRING) {
            throw IllegalArgumentException(funcName, usage + argName + " should be a string.");
        }
        return arg->getString();
    }

    inline DictionarySP getDictionary(const SmartPointer<Constant> &arg, const string &argName, const string &funcName,
                                      const string &usage) {

        if (arg.isNull() or arg->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(funcName, usage + argName + " should be a dictionary.");
        }
        return arg;
    }

    inline TableSP getSharedRealtimeTable(const SmartPointer<Constant> &arg, const string &argName,
                                          const string &funcName, const string &usage) {

        if (arg->getForm() != DF_TABLE) {
            throw IllegalArgumentException(funcName, usage + argName + " should be a table.");
        }
        TableSP table = arg;
        if (table->getTableType() != REALTIMETBL || !table->isSharedTable()) {
            throw IllegalArgumentException(funcName, usage + argName + " should be a shared stream table.");
        }
        return table;
    }

    inline DictionarySP getDictWithIntKeyAndSharedRealtimeTableValue(const SmartPointer<Constant> &arg,
                                                                     const string &argName, const string &funcName,
                                                                     const string &usage) {

        if (arg.isNull() or arg->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(funcName, usage + argName + " should be a dictionary.");
        }
        DictionarySP dict = arg;
        if (arg->keys()->getType() != DT_INT) {
            throw IllegalArgumentException(funcName, usage + argName + "'s keys should be int.");
        }
        VectorSP tables = arg->values();
        for (auto i = 0; i < tables->size(); i++) {
            getSharedRealtimeTable(tables->get(i), "any value of " + argName, funcName, usage);
        }

        return arg;
    }

    inline VectorSP getNumVector(const SmartPointer<Constant> &arg, const string &argName, const string &funcName,
                                 const string &usage, int size = 0) {

        if (arg.isNull() or arg->getForm() != DF_VECTOR or (arg->getType() != DT_INT and arg->getType() != DT_DOUBLE)) {
            throw IllegalArgumentException(funcName, usage + argName + " should be a vector of int or double.");
        }
        if (size and arg->size() != size) {
            throw IllegalArgumentException(funcName, usage + "size of " + argName + " is different from size of varName.");
        }

        return arg;
    }

    inline VectorSP getCharVector(const SmartPointer<Constant> &arg, const string &argName, const string &funcName,
                                  const string &usage, int size = 0) {

        if (arg.isNull() or arg->getForm() != DF_VECTOR or arg->getType() != DT_CHAR) {
            throw IllegalArgumentException(funcName, usage + argName + " should be a vector of char.");
        }
        if (size and arg->size() != size) {
            throw IllegalArgumentException(funcName, usage + "size of " + argName + " is different from size of varName.");
        }

        return arg;
    }

    inline VectorSP getStringVector(const SmartPointer<Constant> &arg, const string &argName, const string &funcName,
                    const string &usage, int size = 0) {

        if (arg.isNull() or arg->getForm() != DF_VECTOR or arg->getType() != DT_STRING) {
            throw IllegalArgumentException(funcName, usage + argName + " should be a vector of string.");
        }
        if (size and arg->size() != size) {
            throw IllegalArgumentException(funcName, usage + "size of " + argName + " is different from size of varName.");
        }

        return arg;
    }

    inline VectorSP getQuadMatrix(const SmartPointer<Constant> &arg, const string &argName, const string &funcName,
                                  const string &usage, int size = 0) {

        if (arg.isNull() or arg->getForm() != DF_VECTOR or arg->getType() != DT_ANY) {
            throw IllegalArgumentException(funcName, usage + argName + " should be a vector of string.");
        }
        if (size and arg->size() != size) {
            throw IllegalArgumentException(funcName, usage + "size of " + argName + " is different from size of varName.");
        }
        for (auto i = 0; i < size; i++) {
            getNumVector(arg->get(i), "member of " + argName, funcName, usage, arg->size());
        }

        return arg;
    }

    /// Single-append Helpers for vector<ConstantSP>

    // Iterator of vector<ConstantSP>
//    typedef __gnu_cxx::__normal_iterator<ConstantSP *, vector<ConstantSP>> ConstantVecIterator;

    typedef vector<ConstantSP>::iterator ConstantVecIterator;

    inline VectorSP getVec(const ConstantVecIterator &colIter) {
        return *colIter;
    }
    inline void appendString(const ConstantVecIterator &colIter, const string &data) {
        getVec(colIter)->appendString(&data, 1);
    }
    inline void appendInt(const ConstantVecIterator &colIter, int data) {
        getVec(colIter)->appendInt(&data, 1);
    }
    inline void appendLong(const ConstantVecIterator &colIter, long long data) {
        getVec(colIter)->appendLong(&data, 1);
    }
    inline void appendDouble(const ConstantVecIterator &colIter, double data) {
        getVec(colIter)->appendDouble(&data, 1);
    }
    inline void appendChar(const ConstantVecIterator &colIter, char data) {
        getVec(colIter)->appendChar(&data, 1);
    }

} // namespace pluginUtil


#endif //PLUGINNSQ_PLUGINUTIL_H
