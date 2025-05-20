#include "jsonUtil.h"
#include "CoreConcept.h"
#include "EncoderDecoder.h"
#include "Exceptions.h"
#include "Logger.h"
#include "ScalarImp.h"
#include "Types.h"
#include <climits>
#include <exception>
#include <ratio>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include <ddbplugin/PluginLogger.h>

using namespace ddb;
using namespace rapidjson;
using std::max;
using std::min;

int ARRAY_VECTOR_TYPE_BASE = 64;

ConstantSP parseJsonWrapper(Heap* heap, vector<ConstantSP>& arguments)  {
    vector<ConstantSP> newArgs{};
    unsigned int index = 0;
    for(;index < 3; ++index) {
        newArgs.push_back(arguments[index]);
    }
    for(unsigned int i = index+1; i < arguments.size(); ++i) {
        newArgs.push_back(arguments[i]);
    }
    return parseJson(heap, newArgs);
}

ConstantSP parseNestedJsonWrapper(Heap* heap, vector<ConstantSP>& arguments) {
    vector<ConstantSP> newArgs{};
    unsigned int index = 0;
    for(;index < 3; ++index) {
        newArgs.push_back(arguments[index]);
    }
    for(unsigned int i = index+1; i < arguments.size(); ++i) {
        newArgs.push_back(arguments[i]);
    }
    return parseNestedJson(heap, newArgs);
}

void splitJson(const std::string& src, std::vector<std::string>& jsonVec)
{
    int startIndex = -1;
    int braceNum = 0;
    for (unsigned i = 0; i < src.size(); ++i) {
        if (src[i] == '{') {
            if (braceNum == 0) {
                startIndex = i;
            }
            ++braceNum;
        } else if (src[i] == '}') {
            --braceNum;
            if (braceNum == 0) {
                jsonVec.push_back(src.substr(startIndex, i - startIndex + 1));
            }
        }
    }
}

/**
 * parse dict json strings, return an table
 */
ConstantSP parseJson(Heap* heap, vector<ConstantSP>& arguments)
{
    bool isMultiJson = arguments[2]->getBool();

    VectorSP vec;
    switch (arguments[3]->getForm()) {
    case DF_TABLE:
        if (arguments[3]->columns() != 1 || arguments[3]->getColumn(0)->getType() != DT_STRING) {
            throw RuntimeException(ENCODERDECODER_PREFIX + " obj must be one of: table with only one STRING column, STRING vector, STRING scalar.");
        }
        vec = arguments[3]->getColumn(0);
        break;
    case DF_VECTOR:
        if (arguments[3]->getType() != DT_STRING) {
            throw RuntimeException(ENCODERDECODER_PREFIX + " obj must be one of: table with only one STRING column, STRING vector, STRING scalar.");
        }
        vec = arguments[3];
        break;
    case DF_SCALAR:
        if (arguments[3]->getType() != DT_STRING) {
            throw RuntimeException(ENCODERDECODER_PREFIX + " obj must be one of: table with only one STRING column, STRING vector, STRING scalar.");
        }
        vec = Util::createVector(DT_STRING, 0, 1);
        vec->append(arguments[3]);
        break;
    default:
        throw RuntimeException(ENCODERDECODER_PREFIX + " obj must be one of: table with only one STRING column, STRING vector, STRING scalar.");
        break;
    }

    vector<string> originCol, convertCol;
    VectorSP types;

    int rows = vec->size();
    if (isMultiJson) {
        std::vector<std::string> splitJsonVec;
        for (int k = 0; k < rows; ++k) {
            splitJson(vec->getString(k), splitJsonVec);
        }
        vec = Util::createVector(DT_STRING, 0, splitJsonVec.size());
        vec->appendString(splitJsonVec.data(), splitJsonVec.size());
        rows = vec->size();
    }

    //get colName
    types = arguments[1];
    int colSize = types->size();

    for (int i = 0; i < colSize; i++) {
        originCol.emplace_back(arguments[0]->getString(i));
        convertCol.emplace_back(arguments[0]->getString(i));
    }

    //create output vectors
    int maxIndex = 1024;
    if (types->size() != colSize)
        throw RuntimeException(ENCODERDECODER_PREFIX + " types size must be equal than origin colNames. ");
    vector<ConstantSP> cols;
    vector<vector<char>> dataBuffer;
    vector<vector<string>> dataStringBuffer;
    vector<vector<INDEX>> arrayDataIndex;
    vector<vector<char>> arrayDataBuffer;
    vector<bool> hasNulls;
    cols.resize(colSize);
    dataStringBuffer.resize(colSize);
    dataBuffer.resize(colSize);
    arrayDataIndex.resize(colSize);
    hasNulls.resize(colSize);
    vector<DATA_TYPE> colTypes;
    for (int i = 0; i < colSize; ++i) {
        DATA_TYPE type = (DATA_TYPE)types->getInt(i);
        if ((int)type < ARRAY_VECTOR_TYPE_BASE) {
            cols[i] = Util::createVector(type, 0, rows);
            switch (type) {
            case DT_BOOL:
                dataBuffer[i].resize(rows * sizeof(bool));
                break;
            case DT_CHAR:
                dataBuffer[i].resize(rows * sizeof(char));
                break;
            case DT_INT:
                dataBuffer[i].resize(rows * sizeof(int));
                break;
            case DT_LONG:
                dataBuffer[i].resize(rows * sizeof(long long));
                break;
            case DT_FLOAT:
                dataBuffer[i].resize(rows * sizeof(float));
                break;
            case DT_DOUBLE:
                dataBuffer[i].resize(rows * sizeof(double));
                break;
            case DT_BLOB:
            case DT_STRING:
                dataStringBuffer[i].resize(rows * sizeof(char*));
                break;
            case DT_SYMBOL:
                dataStringBuffer[i].resize(rows * sizeof(char*));
                break;
            default:
                throw RuntimeException(ENCODERDECODER_PREFIX + " The dolphindb type " + Util::getDataTypeString(type) + " is not supported");
            }
        } else {
            //cols[i] = Util::createArrayVector((DATA_TYPE)type, 0);
            switch (type - ARRAY_VECTOR_TYPE_BASE) {
            case DT_BOOL:
                dataBuffer[i].resize(rows * sizeof(bool));
                break;
            case DT_INT:
                dataBuffer[i].resize(rows * sizeof(int));
                break;
            case DT_LONG:
                dataBuffer[i].resize(rows * sizeof(long long));
                break;
            case DT_FLOAT:
                dataBuffer[i].resize(rows * sizeof(float));
                break;
            case DT_DOUBLE:
                dataBuffer[i].resize(rows * sizeof(double));
                break;
            default:
                throw RuntimeException(ENCODERDECODER_PREFIX + " The dolphindb type " + Util::getDataTypeString(type) + " don't support array vector");
            }
        }
        colTypes.push_back(type);
    }

    vector<string> originData;
    originData.resize(rows);
    char* buffer[maxIndex];
    int times = rows / maxIndex + 1;
    long long dataOffset = 0;
    for (int timeIndex = 0; timeIndex < times; ++timeIndex) {
        int subSize = min(maxIndex, rows - maxIndex * timeIndex);
        char** ptr = vec->getStringConst(maxIndex * timeIndex, subSize, buffer);
        for (int subRowIndex = 0; subRowIndex < subSize; ++subRowIndex) {
            originData[subRowIndex] = ptr[subRowIndex];
        }
        for (int subRowIndex = 0; subRowIndex < subSize; ++subRowIndex) {
            rapidjson::Document parsedObj;
            rapidjson::Document parsedObjNumberAsStr;
            try {
                parsedObj.Parse(originData[subRowIndex].c_str());
                parsedObjNumberAsStr.Parse<rapidjson::kParseNumbersAsStringsFlag>(originData[subRowIndex].c_str());
            } catch (MemoryException& me) {
                throw me;
            } catch (std::bad_alloc& me) {
                throw me;
            } catch (exception& ex) {
                PLUGIN_LOG_ERR(ENCODERDECODER_PREFIX + string(ex.what()));
                continue;
            }
            rapidjson::Value nullValue;
            nullValue.SetNull();
            for (int colIndex = 0; colIndex < colSize; ++colIndex) {

                rapidjson::Value& col = nullValue;
                bool hasMember = false;
                try {
                    hasMember = parsedObj.HasMember(originCol[colIndex].c_str());
                } catch (MemoryException& me) {
                    throw me;
                } catch (std::bad_alloc& me) {
                    throw me;
                } catch (...) {
                }
                if(hasMember) {
                    col = parsedObj[originCol[colIndex].c_str()];
                } else {
                    col.SetNull();
                }
                try {
                    if ((int)colTypes[colIndex] < ARRAY_VECTOR_TYPE_BASE) {
                        switch (colTypes[colIndex]) {
                        case DT_BOOL:
                            if (col.IsNull()) {
                                ((char*)dataBuffer[colIndex].data())[dataOffset] = CHAR_MIN;
                                hasNulls[colIndex] = true;
                            }
                            else if(col.IsBool())
                                ((bool*)dataBuffer[colIndex].data())[dataOffset] = (bool)col.GetBool();
                            else if(col.IsInt())
                                ((bool*)dataBuffer[colIndex].data())[dataOffset] = (bool)col.GetInt();
                            else if(col.IsInt64())
                                ((bool*)dataBuffer[colIndex].data())[dataOffset] = (bool)col.GetInt64();
                            else if(col.IsUint())
                                ((bool*)dataBuffer[colIndex].data())[dataOffset] = (bool)col.GetUint();
                            else if(col.IsUint64())
                                ((bool*)dataBuffer[colIndex].data())[dataOffset] = (bool)col.GetUint64();
                            else if(col.IsFloat())
                                ((bool*)dataBuffer[colIndex].data())[dataOffset] = (bool)col.GetFloat();
                            else if(col.IsDouble())
                                ((bool*)dataBuffer[colIndex].data())[dataOffset] = (bool)col.GetDouble();
                            else if(col.IsString()) {
                                string value = col.GetString();
                                ((bool*)dataBuffer[colIndex].data())[dataOffset] = value == "" ? false: true;
                            }
                            else
                                throw RuntimeException("Illegal conversion to type BOOL for col: " + originCol[colIndex]);
                            break;
                        case DT_CHAR:
                            if (col.IsNull()) {
                                ((char*)dataBuffer[colIndex].data())[dataOffset] = CHAR_MIN;
                                hasNulls[colIndex] = true;
                            } else if(col.IsBool()) {
                                ((char*)dataBuffer[colIndex].data())[dataOffset] = (char)col.GetBool();
                            }
                            else if(col.IsString()) {
                                string value = col.GetString();
                                if(value.size() == 0) {
                                    ((char*)dataBuffer[colIndex].data())[dataOffset] = CHAR_MIN;
                                } else if(value.size() == 1) {
                                    ((char*)dataBuffer[colIndex].data())[dataOffset] = value[0];
                                } else {
                                    throw RuntimeException("Illegal conversion from string \"" + value + "\" to type CHAR for col: " + originCol[colIndex]);
                                }
                            } else {
                                throw RuntimeException("Illegal conversion to type CHAR for col: " + originCol[colIndex]);
                            }
                            break;
                        case DT_INT:
                            if (col.IsNull()) {
                                ((int*)dataBuffer[colIndex].data())[dataOffset] = INT_MIN;
                                hasNulls[colIndex] = true;
                            }
                            else if(col.IsInt()) {
                                ((int*)dataBuffer[colIndex].data())[dataOffset] = (int)col.GetInt();
                            }
                            else if(col.IsInt64()) {
                                long long result = col.GetInt64();
                                if(result < INT_MIN) {
                                    ((int*)dataBuffer[colIndex].data())[dataOffset] = INT_MIN;
                                } else if(result > INT_MAX) {
                                    ((int*)dataBuffer[colIndex].data())[dataOffset] = INT_MAX;
                                } else {
                                    ((int*)dataBuffer[colIndex].data())[dataOffset] = (int)result;
                                }
                            }
                            else if(col.IsUint()) {
                                uint result = col.GetUint();
                                if(result > INT_MAX) {
                                    ((int*)dataBuffer[colIndex].data())[dataOffset] = INT_MAX;
                                } else {
                                    ((int*)dataBuffer[colIndex].data())[dataOffset] = (int)result;
                                }
                            }
                            else if(col.IsUint64()) {
                                unsigned long long result = col.GetUint64();
                                if(result > INT_MAX) {
                                    ((int*)dataBuffer[colIndex].data())[dataOffset] = INT_MAX;
                                } else {
                                    ((int*)dataBuffer[colIndex].data())[dataOffset] = (int)result;
                                }
                            }
                            else if(col.IsFloat()) {
                                float result = col.GetFloat();
                                if(result > double(INT_MAX)) {
                                    ((int *)dataBuffer[colIndex].data())[dataOffset] = INT_MAX;
                                } else if (result < double(INT_MIN)) {
                                    ((int *)dataBuffer[colIndex].data())[dataOffset] = INT_MIN;
                                }else {
                                    ((int *)dataBuffer[colIndex].data())[dataOffset] = (int)result;
                                }
                            }
                            else if(col.IsDouble()) {
                                double result = col.GetDouble();
                                if(result > double(INT_MAX)) {
                                    ((int *)dataBuffer[colIndex].data())[dataOffset] = INT_MAX;
                                } else if (result < double(INT_MIN)) {
                                    ((int *)dataBuffer[colIndex].data())[dataOffset] = INT_MIN;
                                }else {
                                    ((int *)dataBuffer[colIndex].data())[dataOffset] = (int)result;
                                }
                            }
                            else if(col.IsBool())
                                ((int*)dataBuffer[colIndex].data())[dataOffset] = (int)col.GetBool();
                            else if(col.IsString()) {
                                throw RuntimeException("Illegal conversion from string to type INT for col: " + originCol[colIndex]);
                            }
                            else
                                throw RuntimeException("Illegal conversion to type INT for col: " + originCol[colIndex]);
                            break;
                        case DT_LONG:
                            if (col.IsNull()) {
                                ((long long*)dataBuffer[colIndex].data())[dataOffset] = LLONG_MIN;
                                hasNulls[colIndex] = true;
                            }
                            else if(col.IsInt64()) {
                                ((long long *)dataBuffer[colIndex].data())[dataOffset] = (long long)col.GetInt64();
                            }
                            else if(col.IsInt()) {
                                ((long long *)dataBuffer[colIndex].data())[dataOffset] = (long long)col.GetInt();
                            }
                            else if(col.IsUint()) {
                                ((long long *)dataBuffer[colIndex].data())[dataOffset] = (long long)col.GetUint();
                            }
                            else if(col.IsUint64()) {
                                unsigned long long result = col.GetUint64();
                                if(result > LONG_LONG_MAX) {
                                    ((long long *)dataBuffer[colIndex].data())[dataOffset] = LONG_LONG_MAX;
                                } else {
                                    ((long long *)dataBuffer[colIndex].data())[dataOffset] = (long long)col.GetUint64();
                                }
                            }
                            else if(col.IsFloat()) {
                                float result = col.GetFloat();
                                if(result > float(LONG_LONG_MAX)) {
                                    ((long long *)dataBuffer[colIndex].data())[dataOffset] = LONG_LONG_MAX;
                                } else if (result < float(LONG_LONG_MIN)) {
                                    ((long long *)dataBuffer[colIndex].data())[dataOffset] = LONG_LONG_MIN;
                                }else {
                                    ((long long *)dataBuffer[colIndex].data())[dataOffset] = (long long)result;
                                }
                            }
                            else if(col.IsDouble()) {
                                double result = col.GetDouble();
                                if(result > double(LONG_LONG_MAX)) {
                                    ((long long *)dataBuffer[colIndex].data())[dataOffset] = LONG_LONG_MAX;
                                } else if (result < double(LONG_LONG_MIN)) {
                                    ((long long *)dataBuffer[colIndex].data())[dataOffset] = LONG_LONG_MIN;
                                }else {
                                    ((long long *)dataBuffer[colIndex].data())[dataOffset] = (long long)result;
                                }
                            }
                            else if(col.IsBool())
                                ((long long *)dataBuffer[colIndex].data())[dataOffset] = (long long)col.GetBool();
                            else if(col.IsString()) {
                                throw RuntimeException("Illegal conversion from string to type LONG for col: " + originCol[colIndex]);
                            }
                            else
                                throw RuntimeException("Illegal conversion to type LONG for col: " + originCol[colIndex]);
                            break;
                        case DT_FLOAT:
                            if (col.IsNull()) {
                                ((float*)dataBuffer[colIndex].data())[dataOffset] = FLT_NMIN;
                                hasNulls[colIndex] = true;
                            }
                            else if(col.IsFloat())
                                ((float *)dataBuffer[colIndex].data())[dataOffset] = (float)col.GetFloat();
                            else if(col.IsDouble())
                                ((float *)dataBuffer[colIndex].data())[dataOffset] = (float)col.GetDouble();
                            else if(col.IsInt())
                                ((float *)dataBuffer[colIndex].data())[dataOffset] = (float)col.GetInt();
                            else if(col.IsInt64())
                                ((float *)dataBuffer[colIndex].data())[dataOffset] = (float)col.GetInt64();
                            else if(col.IsUint())
                                ((float *)dataBuffer[colIndex].data())[dataOffset] = (float)col.GetUint();
                            else if(col.IsUint64())
                                ((float *)dataBuffer[colIndex].data())[dataOffset] = (float)col.GetUint64();
                            else if(col.IsBool())
                                ((float *)dataBuffer[colIndex].data())[dataOffset] = (float)col.GetBool();
                            else if(col.IsString()) {
                                throw RuntimeException("Illegal conversion from string to type FLOAT for col: " + originCol[colIndex]);
                            }
                            else
                                throw RuntimeException("Illegal conversion to type FLOAT for col: " + originCol[colIndex]);
                            break;
                        case DT_DOUBLE:
                            if (col.IsNull()) {
                                ((double*)dataBuffer[colIndex].data())[dataOffset] = DBL_NMIN;
                                hasNulls[colIndex] = true;
                            }
                            else if(col.IsDouble()) // double must before float, in case of loss of precision
                                ((double *)dataBuffer[colIndex].data())[dataOffset] = (double)col.GetDouble();
                            else if(col.IsFloat())
                                ((double *)dataBuffer[colIndex].data())[dataOffset] = (double)col.GetFloat();
                            else if(col.IsInt())
                                ((double *)dataBuffer[colIndex].data())[dataOffset] = (double)col.GetInt();
                            else if(col.IsInt64())
                                ((double *)dataBuffer[colIndex].data())[dataOffset] = (double)col.GetInt64();
                            else if(col.IsUint())
                                ((double *)dataBuffer[colIndex].data())[dataOffset] = (double)col.GetUint();
                            else if(col.IsUint64())
                                ((double *)dataBuffer[colIndex].data())[dataOffset] = (double)col.GetUint64();
                            else if(col.IsBool())
                                ((double *)dataBuffer[colIndex].data())[dataOffset] = (double)col.GetBool();
                            else if(col.IsString()) {
                                throw RuntimeException("Illegal conversion from string to type DOUBLE for col: " + originCol[colIndex]);
                            }
                            else
                                throw RuntimeException("Illegal conversion to type DOUBLE for col: " + originCol[colIndex]);
                            break;
                        case DT_BLOB:
                        case DT_STRING:
                        case DT_SYMBOL:
                            if (col.IsNull()) {
                                dataStringBuffer[colIndex][dataOffset] = "";
                                hasNulls[colIndex] = true;
                            } else if(col.IsString()) {
                                dataStringBuffer[colIndex][dataOffset] = col.GetString();
                            } else if(col.IsNumber()) {
                                rapidjson::Value& colStr = parsedObjNumberAsStr[originCol[colIndex].c_str()];
                                dataStringBuffer[colIndex][dataOffset] = colStr.GetString();
                            } else {
                                rapidjson::StringBuffer buf;
                                rapidjson::PrettyWriter<rapidjson::StringBuffer> wr(buf);
                                col.Accept(wr);
                                dataStringBuffer[colIndex][dataOffset] = buf.GetString();
                            }
                            break;
                        default:
                            throw RuntimeException("The dolphindb type " + Util::getDataTypeString(colTypes[colIndex]) + " is not supported to convert");
                        }
                    } else {
                        int size;
                        if(col.IsNull()) {
                            size = 0;
                        } else {
                            size = col.Size();
                        }
                        vector<INDEX>& colDataArrayIndex = arrayDataIndex[colIndex];
                        int preIndex = colDataArrayIndex.size() == 0 ? 0 : colDataArrayIndex[colDataArrayIndex.size() - 1];
                        if (col.GetType() != rapidjson::Type::kArrayType && col.GetType() != rapidjson::Type::kNullType)
                            throw RuntimeException("The col " + originCol[colIndex] + " json data must be array. ");
                        switch (colTypes[colIndex] - ARRAY_VECTOR_TYPE_BASE) {
                        case DT_BOOL:
                            if ((preIndex + size + 1) * sizeof(bool) > dataBuffer[colIndex].size())
                                dataBuffer[colIndex].resize(max(dataBuffer[colIndex].size(), (preIndex + size + 1) * sizeof(bool)) * 2);
                            if (col.IsNull() || size == 0) {
                                ((char*)dataBuffer[colIndex].data())[preIndex] = CHAR_MIN;
                                arrayDataIndex[colIndex].push_back(preIndex + 1);
                            } else {
                                int index = 0;
                                for (auto it = col.Begin(); it < col.End(); ++it, ++index) {
                                    if (it->IsNull())
                                        ((char*)dataBuffer[colIndex].data())[preIndex + index] = CHAR_MIN;
                                    else if(it->IsBool())
                                        ((bool*)dataBuffer[colIndex].data())[preIndex + index] = (bool)it->GetBool();
                                    else if(it->IsInt())
                                        ((bool*)dataBuffer[colIndex].data())[preIndex + index] = (bool)it->GetInt();
                                    else if(it->IsInt64())
                                        ((bool*)dataBuffer[colIndex].data())[preIndex + index] = (bool)it->GetInt64();
                                    else if(it->IsUint())
                                        ((bool*)dataBuffer[colIndex].data())[preIndex + index] = (bool)it->GetUint();
                                    else if(it->IsUint64())
                                        ((bool*)dataBuffer[colIndex].data())[preIndex + index] = (bool)it->GetUint64();
                                    else if(it->IsFloat())
                                        ((bool*)dataBuffer[colIndex].data())[preIndex + index] = (bool)it->GetFloat();
                                    else if(it->IsDouble())
                                        ((bool*)dataBuffer[colIndex].data())[preIndex + index] = (bool)it->GetDouble();
                                    else if(it->IsString()) {
                                        string value = col.GetString();
                                        ((bool*)dataBuffer[colIndex].data())[preIndex + index] = value == "" ? false: true;
                                    }
                                    else
                                        throw RuntimeException("Illegal conversion to type BOOL for col: " + originCol[colIndex]);
                                }
                                arrayDataIndex[colIndex].push_back(preIndex + size);
                            }
                            break;
                        case DT_INT:
                            if ((preIndex + size + 1) * sizeof(int) > dataBuffer[colIndex].size())
                                dataBuffer[colIndex].resize(max(dataBuffer[colIndex].size(), (preIndex + size + 1) * sizeof(int)) * 2);
                            if (col.IsNull() || size == 0) {
                                ((int*)dataBuffer[colIndex].data())[preIndex] = INT_MIN;
                                arrayDataIndex[colIndex].push_back(preIndex + 1);
                            } else {
                                int index = 0;
                                for (auto it = col.Begin(); it < col.End(); ++it, ++index) {
                                    if (it->IsNull())
                                        ((int*)dataBuffer[colIndex].data())[preIndex + index] = INT_MIN;
                                    else if(it->IsInt())
                                        ((int*)dataBuffer[colIndex].data())[preIndex + index] = (int)it->GetInt();
                                    else if(it->IsInt64())
                                        ((int*)dataBuffer[colIndex].data())[preIndex + index] = (int)it->GetInt64();
                                    else if(it->IsUint())
                                        ((int*)dataBuffer[colIndex].data())[preIndex + index] = (int)it->GetUint();
                                    else if(it->IsUint64())
                                        ((int*)dataBuffer[colIndex].data())[preIndex + index] = (int)it->GetUint64();
                                    else if(it->IsFloat())
                                        ((int*)dataBuffer[colIndex].data())[preIndex + index] = (int)it->GetFloat();
                                    else if(it->IsDouble())
                                        ((int*)dataBuffer[colIndex].data())[preIndex + index] = (int)it->GetDouble();
                                    else if(it->IsBool())
                                        ((int*)dataBuffer[colIndex].data())[preIndex + index] = (int)it->GetBool();
                                    else if(it->IsString()) {
                                        throw RuntimeException("Illegal conversion from string to type INT for col: " + originCol[colIndex]);
                                    }
                                    else
                                        throw RuntimeException("Illegal conversion to type INT for col: " + originCol[colIndex]);
                                }
                                arrayDataIndex[colIndex].push_back(preIndex + size);
                            }
                            break;
                        case DT_LONG:
                            if ((preIndex + size + 1) * sizeof(long long) > dataBuffer[colIndex].size())
                                dataBuffer[colIndex].resize(max(dataBuffer[colIndex].size(), (preIndex + size + 1) * sizeof(long long)) * 2);
                            if (col.IsNull() || size == 0) {
                                ((long long*)dataBuffer[colIndex].data())[preIndex] = LLONG_MIN;
                                arrayDataIndex[colIndex].push_back(preIndex + 1);
                            } else {
                                long long index = 0;
                                for (auto it = col.Begin(); it < col.End(); ++it, ++index) {
                                    if (it->IsNull())
                                        ((long long*)dataBuffer[colIndex].data())[preIndex + index] = LLONG_MIN;
                                    else if(it->IsInt64())
                                        ((long long *)dataBuffer[colIndex].data())[preIndex + index] = (long long)it->GetInt64();
                                    else if(it->IsInt())
                                        ((long long *)dataBuffer[colIndex].data())[preIndex + index] = (long long)it->GetInt();
                                    else if(it->IsUint())
                                        ((long long *)dataBuffer[colIndex].data())[preIndex + index] = (long long)it->GetUint();
                                    else if(it->IsUint64())
                                        ((long long *)dataBuffer[colIndex].data())[preIndex + index] = (long long)it->GetUint64();
                                    else if(it->IsFloat())
                                        ((long long *)dataBuffer[colIndex].data())[preIndex + index] = (long long)it->GetFloat();
                                    else if(it->IsDouble())
                                        ((long long *)dataBuffer[colIndex].data())[preIndex + index] = (long long)it->GetDouble();
                                    else if(it->IsBool())
                                        ((long long *)dataBuffer[colIndex].data())[preIndex + index] = (long long)it->GetBool();
                                    else if(it->IsString()) {
                                        throw RuntimeException("Illegal conversion from string to type LONG for col: " + originCol[colIndex]);
                                    }
                                    else
                                        throw RuntimeException("Illegal conversion to type LONG for col: " + originCol[colIndex]);
                                }
                                arrayDataIndex[colIndex].push_back(preIndex + size);
                            }
                            break;
                        case DT_FLOAT:
                            if ((preIndex + size + 1) * sizeof(float) > dataBuffer[colIndex].size())
                                dataBuffer[colIndex].resize(max(dataBuffer[colIndex].size(), (preIndex + size + 1) * sizeof(float)) * 2);
                            if (col.IsNull() || size == 0) {
                                ((float*)dataBuffer[colIndex].data())[preIndex] = FLT_NMIN;
                                arrayDataIndex[colIndex].push_back(preIndex + 1);
                            } else {
                                int index = 0;
                                for (auto it = col.Begin(); it < col.End(); ++it, ++index) {
                                    if (it->IsNull())
                                        ((float*)dataBuffer[colIndex].data())[preIndex + index] = FLT_NMIN;
                                    else if(it->IsFloat())
                                        ((float *)dataBuffer[colIndex].data())[preIndex + index] = (float)it->GetFloat();
                                    else if(it->IsDouble())
                                        ((float *)dataBuffer[colIndex].data())[preIndex + index] = (float)it->GetDouble();
                                    else if(it->IsInt())
                                        ((float *)dataBuffer[colIndex].data())[preIndex + index] = (float)it->GetInt();
                                    else if(it->IsInt64())
                                        ((float *)dataBuffer[colIndex].data())[preIndex + index] = (float)it->GetInt64();
                                    else if(it->IsUint())
                                        ((float *)dataBuffer[colIndex].data())[preIndex + index] = (float)it->GetUint();
                                    else if(it->IsUint64())
                                        ((float *)dataBuffer[colIndex].data())[preIndex + index] = (float)it->GetUint64();
                                    else if(it->IsBool())
                                        ((float *)dataBuffer[colIndex].data())[preIndex + index] = (float)it->GetBool();
                                    else if(it->IsString()) {
                                        throw RuntimeException("Illegal conversion from string to type FLOAT for col: " + originCol[colIndex]);
                                    }
                                    else
                                        throw RuntimeException("Illegal conversion to type FLOAT for col: " + originCol[colIndex]);
                                }
                                arrayDataIndex[colIndex].push_back(preIndex + size);
                            }
                            break;
                        case DT_DOUBLE:
                            if ((preIndex + size + 1) * sizeof(double) > dataBuffer[colIndex].size())
                                dataBuffer[colIndex].resize(max(dataBuffer[colIndex].size(), (preIndex + size + 1) * sizeof(double)) * 2);
                            if (col.IsNull() || size == 0) {
                                ((double*)dataBuffer[colIndex].data())[preIndex] = DBL_NMIN;
                                arrayDataIndex[colIndex].push_back(preIndex + 1);
                            } else {
                                int index = 0;
                                for (auto it = col.Begin(); it < col.End(); ++it, ++index) {
                                    if (it->IsNull())
                                        ((double*)dataBuffer[colIndex].data())[preIndex + index] = DBL_NMIN;
                                    else if(it->IsDouble())
                                        ((double *)dataBuffer[colIndex].data())[preIndex + index] = (double)it->GetDouble();
                                    else if(it->IsFloat())
                                        ((double *)dataBuffer[colIndex].data())[preIndex + index] = (double)it->GetFloat();
                                    else if(it->IsInt())
                                        ((double *)dataBuffer[colIndex].data())[preIndex + index] = (double)it->GetInt();
                                    else if(it->IsInt64())
                                        ((double *)dataBuffer[colIndex].data())[preIndex + index] = (double)it->GetInt64();
                                    else if(it->IsUint())
                                        ((double *)dataBuffer[colIndex].data())[preIndex + index] = (double)it->GetUint();
                                    else if(it->IsUint64())
                                        ((double *)dataBuffer[colIndex].data())[preIndex + index] = (double)it->GetUint64();
                                    else if(it->IsBool())
                                        ((double *)dataBuffer[colIndex].data())[preIndex + index] = (double)it->GetBool();
                                    else if(it->IsString()) {
                                        throw RuntimeException("Illegal conversion from string to type DOUBLE for col: " + originCol[colIndex]);
                                    }
                                    else
                                        throw RuntimeException("Illegal conversion to type DOUBLE for col: " + originCol[colIndex]);
                                }
                                arrayDataIndex[colIndex].push_back(preIndex + size);
                            }
                            break;
                        default:
                            throw RuntimeException("The dolphindb type " + Util::getDataTypeString(colTypes[colIndex]) + "is not supported to convert");
                        }
                    }
                } catch (exception& e) {
                    throw RuntimeException(ENCODERDECODER_PREFIX + "col [" + originCol[colIndex] + "]: " + e.what());
                }
            }
            dataOffset++;
        }
    }
    rows = dataOffset;
    for (int colIndex = 0; colIndex < colSize; ++colIndex) {
        VectorSP& vec = (VectorSP&)cols[colIndex];
        if ((int)colTypes[colIndex] < ARRAY_VECTOR_TYPE_BASE) {
            switch (colTypes[colIndex]) {
            case DT_BOOL:
                vec->appendBool((char*)dataBuffer[colIndex].data(), rows);
                break;
            case DT_CHAR:
                vec->appendChar((char*)dataBuffer[colIndex].data(), rows);
                break;
            case DT_INT:
                vec->appendInt((int*)dataBuffer[colIndex].data(), rows);
                break;
            case DT_LONG:
                vec->appendLong((long long*)dataBuffer[colIndex].data(), rows);
                break;
            case DT_FLOAT:
                vec->appendFloat((float*)dataBuffer[colIndex].data(), rows);
                break;
            case DT_DOUBLE:
                vec->appendDouble((double*)dataBuffer[colIndex].data(), rows);
                break;
            case DT_BLOB:
            case DT_STRING:
                vec->appendString((string*)dataStringBuffer[colIndex].data(), rows);
                break;
            case DT_SYMBOL:
                vec->appendString((string*)dataStringBuffer[colIndex].data(), rows);
                break;

            default:
                throw RuntimeException(ENCODERDECODER_PREFIX + " The dolphindb type " + Util::getDataTypeString(colTypes[colIndex]) + "is not supported to append");
            }
        } else {
            int indexVecSize = arrayDataIndex[colIndex].size();
            VectorSP indexVec = Util::createVector(DT_INDEX, indexVecSize, indexVecSize);
            int totalRows = arrayDataIndex[colIndex].size() == 0 ? 0 : arrayDataIndex[colIndex][arrayDataIndex[colIndex].size() - 1];
            VectorSP vecValue = Util::createVector((DATA_TYPE)(colTypes[colIndex] - 64), 0, totalRows);
            indexVec->setIndex(0, indexVecSize, arrayDataIndex[colIndex].data());
            switch (colTypes[colIndex] - ARRAY_VECTOR_TYPE_BASE) {
            case DT_BOOL:
                vecValue->appendBool((char*)dataBuffer[colIndex].data(), totalRows);
                break;
            case DT_INT:
                vecValue->appendInt((int*)dataBuffer[colIndex].data(), totalRows);
                break;
            case DT_LONG:
                vecValue->appendLong((long long*)dataBuffer[colIndex].data(), totalRows);
                break;
            case DT_FLOAT:
                vecValue->appendFloat((float*)dataBuffer[colIndex].data(), totalRows);
                break;
            case DT_DOUBLE:
                vecValue->appendDouble((double*)dataBuffer[colIndex].data(), totalRows);
                break;
            default:
                throw RuntimeException(ENCODERDECODER_PREFIX + " The dolphindb type " + Util::getDataTypeString(colTypes[colIndex]) + " is not supported to append");
            }
            vector<ConstantSP> args { indexVec, vecValue };
            try {
                vec = Util::getFuncDefFromHeap(heap, "arrayVector")->call(heap, args);
            } catch (exception& e) {
                throw RuntimeException(ENCODERDECODER_PREFIX + " Col " + originCol[colIndex] + " data fail to create arrayVector." + e.what());
            }
        }
    }
    int colIndex = 0;
    for(ConstantSP col: cols) {
        col->setTemporary(true);
        col->setNullFlag(hasNulls[colIndex++]);
    }
    return Util::createTable(convertCol, cols);
}


int flattenRecursively(const Value &jsonVal, vector<ConstantSP> &cols, vector<DATA_TYPE> &colTypes, int &currIndex);
void appendItemToCol(VectorSP &column, DATA_TYPE colType, const Value &jsonVal);
string getErrMsg(DATA_TYPE colType, Type jsonType);

ConstantSP parseNestedJson(Heap* heap, vector<ConstantSP>& arguments)
{
    return new Void();
    // /// Arguments processing
    // if (arguments[0]->getForm() != DF_VECTOR || arguments[0]->getType() != DT_STRING) {
    //     throw RuntimeException(ENCODERDECODER_PREFIX + " convert colNames must be a string vector. ");
    // }

    // if (arguments[1]->getForm() != DF_VECTOR || arguments[1]->getType() != DT_INT) {
    //     throw RuntimeException(ENCODERDECODER_PREFIX + " types must be a int vector. ");
    // }

    // if (arguments[2]->getForm() != DF_SCALAR || arguments[2]->getType() != DT_BOOL) {
    //     throw RuntimeException(ENCODERDECODER_PREFIX + " 3rd argument types must be a bool scalar. ");
    // }
    // bool isMultiJson = arguments[2]->getBool();

    // VectorSP jsonVec;
    // // Check the form of the fourth argument. Depending on its type, different actions are taken.
    // switch (arguments[3]->getForm()) {
    //     case DF_TABLE:
    //         // If the fourth argument is a table with only one string column, assign the column to the vec variable.
    //         if (arguments[3]->columns() != 1 || arguments[3]->getColumn(0)->getType() != DT_STRING) {
    //             throw RuntimeException(ENCODERDECODER_PREFIX + " obj must be one of: table with only one string column, string vector, string scalar.");
    //         }
    //         jsonVec = arguments[3]->getColumn(0);
    //         break;
    //     case DF_VECTOR:
    //         // If the fourth argument is a string vector, assign it to the vec variable.
    //         if (arguments[3]->getType() != DT_STRING) {
    //             throw RuntimeException(ENCODERDECODER_PREFIX + " obj must be one of: table with only one string column, string vector, string scalar.");
    //         }
    //         jsonVec = arguments[3];
    //         break;
    //     case DF_SCALAR:
    //         // If the fourth argument is a string scalar, create a new string vector and append the scalar to it.
    //         if (arguments[3]->getType() != DT_STRING) {
    //             throw RuntimeException(ENCODERDECODER_PREFIX + " obj must be one of: table with only one string column, string vector, string scalar.");
    //         }
    //         jsonVec = Util::createVector(DT_STRING, 0, 1);
    //         jsonVec->append(arguments[3]);
    //         break;
    //     default:
    //         // If the fourth argument is none of the above, throw an exception.
    //         throw RuntimeException(ENCODERDECODER_PREFIX + " obj must be one of: table with only one string column, string vector, string scalar.");
    // }

    // /// Table data structures prep
    // VectorSP argTypes = arguments[1];
    // int numCol = argTypes->size();

    // vector<ConstantSP> cols(numCol);
    // vector<DATA_TYPE> colTypes;
    // vector<string> colNames;

    // // Store original and converted column names.
    // for (int i = 0; i < numCol; i++) {
    //     colNames.emplace_back(arguments[0]->getString(i));
    // }

    // // Iterate over each column, create appropriate data structures for each data type.
    // for (int i = 0; i < numCol; ++i) {
    //     auto type = (DATA_TYPE)argTypes->getInt(i);
    //     cols[i] = Util::createVector(type, 0);
    //     colTypes.push_back(type);
    // }

    // /// JSON data prep
    // int numJson = jsonVec->size();
    // // If the JSON is multipart, split it into its constituent parts.
    // if (isMultiJson) {
    //     std::vector<std::string> splitJsonVec;
    //     for (int k = 0; k < numJson; ++k) {
    //         splitJson(jsonVec->getString(k), splitJsonVec);
    //     }
    //     // After splitting, replace the original vector with a new one containing the split parts.
    //     jsonVec = Util::createVector(DT_STRING, 0, (INDEX)splitJsonVec.size());
    //     jsonVec->appendString(splitJsonVec.data(), (INDEX)splitJsonVec.size());
    //     numJson = jsonVec->size();
    // }
    // vector<string> originData(numJson);
    // int maxBatchSize = 1024;
    // char* buffer[maxBatchSize];
    // int numBatch = numJson / maxBatchSize + 1;

    // /// Iterate over batch
    // for (int batchIndex = 0; batchIndex < numBatch; ++batchIndex) {

    //     int batchSize = min(maxBatchSize, numJson - maxBatchSize * batchIndex);
    //     char** ptr = jsonVec->getStringConst(maxBatchSize * batchIndex, batchSize, buffer);
    //     for (int jsonIndex = 0; jsonIndex < batchSize; ++jsonIndex) {
    //         originData[jsonIndex] = ptr[jsonIndex];
    //     }

    //     /// Iterate over JSON
    //     for (int jsonIndex = 0; jsonIndex < batchSize; ++jsonIndex) {
    //         Document doc;
    //         // Attempt to parse the JSON data.
    //         try {
    //             doc.Parse(originData[jsonIndex].c_str());
    //         } catch (exception& ex) {
    //             // Log any errors that occur during parsing.
    //             PLUGIN_LOG_ERR(ENCODERDECODER_PREFIX + string(ex.what()));
    //             continue; // If an error occurred, skip the rest of this iteration and proceed with the next row.
    //         }

    //         // Recursively add columns
    //         auto currIndex = 0;
    //         flattenRecursively(doc, cols, colTypes, currIndex);
    //         if (currIndex != (int)colNames.size()) {
    //             throw IllegalArgumentException(__FUNCTION__, "JSON data and colNames must have the same length.");
    //         }
    //     }
    // }

    // return Util::createTable(colNames, cols);
}

// int flattenRecursively(const Value &jsonVal, vector<ConstantSP> &cols, vector<DATA_TYPE> &colTypes, int &currIndex) {

//     vector<int> nonArrayIndexes;
//     auto maxDepth = 0;
//     auto arrayEncountered = false;

//     try {
//         switch (jsonVal.GetType()) {
//             /// For Object
//             //      1. iterate over items
//             //      2. memoize non-array items indexes, max depth, and if an array is already encountered
//             //      3. if an array is already encountered and another array appears, throw an exception (cannot be flattened)
//             //      4. at the end of the iteration, append same values to the columns with non-array items to match the max depth
//             case rapidjson::kObjectType:

//                 // iterate over items
//                 for (Value::ConstMemberIterator itr = jsonVal.MemberBegin(); itr != jsonVal.MemberEnd(); ++itr) {
//                     auto depth = flattenRecursively(itr->value, cols, colTypes, currIndex);

//                     // memoize non-array items indexes, max depth, and if an array is already encountered
//                     maxDepth = depth > maxDepth ? depth : maxDepth;
//                     if (depth == 1) {
//                         nonArrayIndexes.emplace_back(currIndex - 1);
//                     } else {

//                         // if an array is already encountered and another array appears, throw an exception (cannot be flattened)
//                         if (arrayEncountered) {
//                             throw IllegalArgumentException(__FUNCTION__, "JSON data must not have parallel arrays.");
//                         } else {
//                             arrayEncountered = true;
//                         }
//                     }
//                 }

//                 // at the end of the iteration, append same values to the columns with non-array items to match the max depth
//                 if (maxDepth > 1) {
//                     for (auto index: nonArrayIndexes) {
//                         auto numAppend = maxDepth - 1;
//                         auto column = (VectorSP) cols.at(index);
//                         auto item = column->get(column->size() - 1);
//                         for (auto i = 0; i < numAppend; i++) {
//                             column->append(item);
//                         }
//                     }
//                 }

//                 return maxDepth;

//                 /// For Array
//                 //      1. iterate over elements
//                 //      2. memoize max depth during the recursion
//             case rapidjson::kArrayType:
//                 // iterate over elements
//                 if (jsonVal.Size() > 0) {
//                     auto startingIndex = currIndex;

//                     for (SizeType i = 0; i < jsonVal.Size(); ++i) {
//                         // reset current index after finishing one element in the array,
//                         // since elements in an array are in the same columns
//                         currIndex = startingIndex;

//                         auto depth = flattenRecursively(jsonVal[i], cols, colTypes, currIndex);
//                         // memoize max depth during the recursion
//                         maxDepth += depth;
// #ifdef JSON_PARSER_DEBUG
//                         std::cout << "max_depth: " << maxDepth << std::endl;
// #endif
//                     }

//                     return maxDepth;
//                 } else {
//                     throw IllegalArgumentException(__FUNCTION__, "JSON data must not have empty arrays.");
//                 }

//                 /// For other types
//                 //      append its value to columns
//             default:
// #ifdef JSON_PARSER_DEBUG
//                 std::cout << std::endl << "currIndex: " << currIndex << std::endl;
// #endif
//                 if (currIndex >= (int)colTypes.size()) {
//                     throw IllegalArgumentException(__FUNCTION__, "JSON data and colNames must have the same length.");
//                 }
//                 auto column = (VectorSP) cols.at(currIndex);
//                 appendItemToCol(column, colTypes.at(currIndex), jsonVal);
//                 currIndex++;
//                 return 1;
//         }
//     } catch (exception &e) {
//         throw RuntimeException(e.what());
//     }
// }

// void appendItemToCol(VectorSP &column, DATA_TYPE colType, const Value &jsonVal) {
//     auto jsonType = jsonVal.GetType();

// #ifdef JSON_PARSER_DEBUG
//     std::cout << jsonType << std::endl;
// #endif

//     /// Function to check if the jsonVal is Null
//     auto isNull = [](Type jsonType, const Value &jsonVal) {
//         string nullStr;
//         return jsonType == kNullType || (jsonType == kStringType && jsonVal.GetString() == nullStr);
//     };

//     /// Append item to column
//     try {
//         switch (colType) {
//             case DT_BOOL:
//                 if (isNull(jsonType, jsonVal)) {
//                     column->append(new Char(CHAR_MIN));
//                 } else if (jsonType == kTrueType) {
//                     column->append(new Char(true));
// #ifdef JSON_PARSER_DEBUG
//                     std::cout << jsonVal.GetBool() << std::endl;
// #endif
//                 } else if (jsonType == kFalseType) {
//                     column->append(new Char(false));
// #ifdef JSON_PARSER_DEBUG
//                     std::cout << jsonVal.GetBool() << std::endl;
// #endif
//                 } else if (jsonType == kNumberType && (jsonVal.GetInt() == 0 || jsonVal.GetInt() == 1)) {
//                     column->append(new Char(jsonVal.GetInt()));
//                 } else {
//                     throw RuntimeException(getErrMsg(colType, jsonType));
//                 }
//                 break;
//             case DT_INT:
//                 if (isNull(jsonType, jsonVal)) {
//                     column->append(new Int(INT_MIN));
//                 } else if (jsonType == kNumberType) {
// #ifdef JSON_PARSER_DEBUG
//                     std::cout << jsonVal.GetInt() << std::endl;
// #endif
//                     column->append(new Int(jsonVal.GetInt()));
//                 } else {
//                     throw RuntimeException(getErrMsg(colType, jsonType));
//                 }
//                 break;
//             case DT_LONG:
//                 if (isNull(jsonType, jsonVal)) {
//                     column->append(new Long(LONG_MIN));
//                 } else if (jsonType == kNumberType) {
// #ifdef JSON_PARSER_DEBUG
//                     std::cout << jsonVal.GetInt64() << std::endl;
// #endif
//                     column->append(new Long(jsonVal.GetInt64()));
//                 } else {
//                     throw RuntimeException(getErrMsg(colType, jsonType));
//                 }
//                 break;
//             case DT_FLOAT:
//                 if (isNull(jsonType, jsonVal)) {
//                     column->append(new Float(FLT_MIN));
//                 } else if (jsonType == kNumberType) {
// #ifdef JSON_PARSER_DEBUG
//                     std::cout << jsonVal.GetFloat() << std::endl;
// #endif
//                     column->append(new Float(jsonVal.GetFloat()));
//                 } else {
//                     throw RuntimeException(getErrMsg(colType, jsonType));
//                 }
//                 break;
//             case DT_DOUBLE:
//                 if (isNull(jsonType, jsonVal)) {
//                     column->append(new Double(DBL_MIN));
//                 } else if (jsonType == kNumberType) {
// #ifdef JSON_PARSER_DEBUG
//                     std::cout << jsonVal.GetDouble() << std::endl;
// #endif
//                     column->append(new Double(jsonVal.GetDouble()));
//                 } else {
//                     throw RuntimeException(getErrMsg(colType, jsonType));
//                 }
//                 break;
//             case DT_STRING:
//             case DT_SYMBOL:
//                 if (isNull(jsonType, jsonVal)) {
//                     column->append(new String(""));
//                 } else if (jsonType == kStringType) {
// #ifdef JSON_PARSER_DEBUG
//                     std::cout << jsonVal.GetString() << std::endl;
// #endif
//                     column->append(new String(jsonVal.GetString()));
//                 } else {
//                     throw RuntimeException(getErrMsg(colType, jsonType));
//                 }
//                 break;
//             default:
//                 throw RuntimeException(
//                         ENCODERDECODER_PREFIX + " The dolphindb type " + Util::getDataTypeString(colType) +
//                         " is not supported to append");
//         }
//     } catch (exception &e) {
//         throw RuntimeException(e.what());
//     }
// }

// string getErrMsg(DATA_TYPE colType, Type jsonType) {
//     string errType;
//     switch (jsonType) {
//         case kNullType:
//             errType = "Null";
//             break;
//         case kFalseType:
//         case kTrueType:
//             errType = "Bool";
//             break;
//         case kObjectType:
//             errType = "Object";
//             break;
//         case kArrayType:
//             errType = "Array";
//             break;
//         case kStringType:
//             errType = "String";
//             break;
//         case kNumberType:
//             errType = "Number";
//             break;
//     }
//     return ENCODERDECODER_PREFIX + " The dolphindb type " + Util::getDataTypeString(colType) + " is not compatible with JSON type " + errType + ".";
// }
