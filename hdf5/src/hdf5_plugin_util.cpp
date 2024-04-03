#include <hdf5_plugin_util.h>
#include <hdf5_plugin_imp.h>
#include "Types.h"

bool colsNumEqual(const TableSP& t, int nCols)
{
    return t->columns() == nCols;
}

void generateIncrementedColsName(std::vector<string> &cols, int size)
{
    cols.resize(size, "col_");
    for (size_t i = 0; i != cols.size(); i++)
        cols[i].append(std::to_string(i));
}

std::string typeIncompatibleErrorMsg(int idx, DATA_TYPE src, const VectorSP& destVec)
{
    DATA_TYPE dest = (destVec == nullptr) ? src : destVec->getType();
    return "incompatible type in column " + std::to_string(idx) + " " +
           Util::getDataTypeString(src) + "->" + Util::getDataTypeString(dest);
}

void checkHDF5Parameter(Heap *heap, vector<ConstantSP> &arguments, ConstantSP &filename, ConstantSP &destOrGroupName,
                        ConstantSP &schema, size_t &startRow, size_t &rowNum, const string &syntax) {
    filename = arguments[0];
    destOrGroupName = arguments[1];

    startRow = 0;
    rowNum = 0;
    schema = H5PluginImp::nullSP;

    if(filename->getType() != DT_STRING || filename->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "fileName must be a string.");

    if(destOrGroupName->getType() != DT_STRING || destOrGroupName->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "groupName must be a string.");

    if (arguments.size() >= 3) {
        if (arguments[2]->isNull())
            schema = H5PluginImp::nullSP;
        else if (!arguments[2]->isTable())
            throw IllegalArgumentException(__FUNCTION__, syntax + "schema must be a table containing column names and types.");
        else
            schema = arguments[2];
    }
    if (arguments.size() >= 4) {
        if (arguments[3]->isScalar() && arguments[3]->isNumber()) {
            if (arguments[3]->getLong() < 0)
                throw IllegalArgumentException(__FUNCTION__, syntax + "startRow must be positive.");
            startRow = arguments[3]->getLong();
        }
        else if (arguments[3]->isNull())
            startRow = 0;
        else
            throw IllegalArgumentException(__FUNCTION__, syntax + "startRow must be an integer scalar.");

    }
    if (arguments.size() >= 5) {
        if (arguments[4]->isScalar() && arguments[4]->isNumber()) {
            if (arguments[4]->getLong() < 0)
                throw IllegalArgumentException(__FUNCTION__, syntax + "rowNum must be positive.");
            rowNum = arguments[4]->getLong();
        }
        else if (arguments[4]->isNull())
            rowNum = 0;
        else
            throw IllegalArgumentException(__FUNCTION__, syntax + "rowNum must be an integer scalar.");

    }
}
