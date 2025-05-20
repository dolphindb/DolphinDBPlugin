#include "opc_ua.h"

#include "Exceptions.h"
#include "ddbplugin/Plugin.h"
#include "ddbplugin/PluginLoggerImp.h"

Mutex OPCUASub::OPCUASub::OPCUA_SUB_MAP_LATCH;
std::unordered_map<long long, OPCUASubSP> OPCUASub::OPCUA_SUB_MAP;
static void handlerTheAnswerChanged(UA_Client *client, UA_UInt32 subId, void *subContext, UA_UInt32 monId,
                                    void *monContext, UA_DataValue *value);

ConstantSP createSVCFromVector(vector<string> &v, DATA_TYPE t) {
    int totallySize = v.size();
    auto ret = Util::createVector(t, totallySize);
    for (int i = 0; i < totallySize; i++) {
        ret->setString(i, v[i]);
    }
    return ret;
}

ConstantSP createSVCFromVector(vector<short> &v) {
    int totallySize = v.size();
    auto ret = Util::createVector(DT_SHORT, totallySize);
    for (int i = 0; i < totallySize; i++) {
        ret->setShort(i, v[i]);
    }
    return ret;
}

void convertUA_Guid(UA_Guid guid, unsigned char *data) {
    data[0] = guid.data4[7];
    data[1] = guid.data4[6];
    data[2] = guid.data4[5];
    data[3] = guid.data4[4];
    data[4] = guid.data4[3];
    data[5] = guid.data4[2];
    data[6] = guid.data4[1];
    data[7] = guid.data4[0];
    data[8] = (unsigned char)guid.data3;
    data[9] = (unsigned char)(guid.data3 >> 8);
    data[10] = (unsigned char)guid.data2;
    data[11] = (unsigned char)(guid.data2 >> 8);
    data[12] = (unsigned char)guid.data1;
    data[13] = (unsigned char)(guid.data1 >> 8);
    data[14] = (unsigned char)(guid.data1 >> 16);
    data[15] = (unsigned char)(guid.data1 >> 24);
}

UA_Guid convertUuidToUA_Guid(const unsigned char *data) {
    UA_Guid result;
    result.data1 =
        (UA_UInt32)data[15] << 24 | (UA_UInt32)data[14] << 16 | (UA_UInt32)data[13] << 8 | (UA_UInt32)data[12];
    result.data2 = (UA_UInt16)data[11] << 8 | (UA_UInt16)data[10];
    result.data3 = (UA_UInt16)data[9] << 8 | (UA_UInt16)data[8];
    result.data4[0] = data[7];
    result.data4[1] = data[6];
    result.data4[2] = data[5];
    result.data4[3] = data[4];
    result.data4[4] = data[3];
    result.data4[5] = data[2];
    result.data4[6] = data[1];
    result.data4[7] = data[0];
    return result;
}

bool convertDTToUA(UA_Variant *variant, ConstantSP value) {
    bool isScalar = value->isScalar();
    bool isMatrix = value->isMatrix();
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    if (value->isArray()) {
        if (!value->get(0)->isScalar()) return false;
    }
    INDEX rows = value->rows();
    INDEX columns = value->columns();
    switch (value->getType()) {
        case DT_BOOL: {
            if (isScalar) {
                bool v = value->getBool();
                retval = UA_Variant_setScalarCopy(variant, &v, &UA_TYPES[UA_TYPES_BOOLEAN]);
                break;
            } else {
                char *v = (char *)value->getDataArray();
                if (isMatrix) {
                    bool *m = new bool[rows * columns];
                    for (INDEX i = 0; i < columns; i++) {
                        for (INDEX j = 0; j < rows; j++) {
                            if (v[i * rows + j] == CHAR_MIN) {
                                m[j * columns + i] = 0;
                            } else {
                                m[j * columns + i] = v[i * rows + j];
                            }
                        }
                    }
                    retval = UA_Variant_setArrayCopy(variant, m, rows * columns, &UA_TYPES[UA_TYPES_BOOLEAN]);
                    delete[] m;
                } else {
                    for (INDEX i = 0; i < columns; i++) {
                        for (INDEX j = 0; j < rows; j++) {
                            if (v[i * rows + j] == CHAR_MIN) {
                                v[i * rows + j] = 0;
                            }
                        }
                    }
                    retval = UA_Variant_setArrayCopy(variant, v, value->size(), &UA_TYPES[UA_TYPES_BOOLEAN]);
                }
                break;
            }
        }
        case DT_CHAR: {
            if (isScalar) {
                char v = value->getChar();
                retval = UA_Variant_setScalarCopy(variant, &v, &UA_TYPES[UA_TYPES_SBYTE]);
                break;
            } else {
                char *v = (char *)value->getDataArray();
                if (isMatrix) {
                    char *m = new char[rows * columns];
                    for (INDEX i = 0; i < columns; i++) {
                        for (INDEX j = 0; j < rows; j++) {
                            m[j * columns + i] = v[i * rows + j];
                        }
                    }
                    retval = UA_Variant_setArrayCopy(variant, m, rows * columns, &UA_TYPES[UA_TYPES_SBYTE]);
                    delete[] m;
                } else {
                    retval = UA_Variant_setArrayCopy(variant, v, value->size(), &UA_TYPES[UA_TYPES_SBYTE]);
                }
                break;
            }
        }
        case DT_TIMESTAMP: {
            if (isScalar) {
                long long v = UnixTimeStamp_To_UA_DateTime(value->getLong());
                retval = UA_Variant_setScalarCopy(variant, &v, &UA_TYPES[UA_TYPES_DATETIME]);
                break;
            } else {
                long long *v = (long long *)value->getDataArray();
                if (isMatrix) {
                    long long *m = new long long[rows * columns];
                    for (INDEX i = 0; i < columns; i++) {
                        for (INDEX j = 0; j < rows; j++) {
                            m[j * columns + i] = UnixTimeStamp_To_UA_DateTime(v[i * rows + j]);
                        }
                    }
                    retval = UA_Variant_setArrayCopy(variant, m, rows * columns, &UA_TYPES[UA_TYPES_DATETIME]);
                    delete[] m;
                } else {
                    for (int i = 0; i < value->size(); i++) {
                        v[i] = UnixTimeStamp_To_UA_DateTime(v[i]);
                    }
                    retval = UA_Variant_setArrayCopy(variant, v, value->size(), &UA_TYPES[UA_TYPES_DATETIME]);
                }
                break;
            }
        }
        case DT_INT: {
            if (isScalar) {
                int v = value->getInt();
                retval = UA_Variant_setScalarCopy(variant, &v, &UA_TYPES[UA_TYPES_INT32]);
                break;
            } else {
                int *v = (int *)value->getDataArray();
                if (isMatrix) {
                    int *m = new int[rows * columns];
                    for (INDEX i = 0; i < columns; i++) {
                        for (INDEX j = 0; j < rows; j++) {
                            m[j * columns + i] = v[i * rows + j];
                        }
                    }
                    retval = UA_Variant_setArrayCopy(variant, m, rows * columns, &UA_TYPES[UA_TYPES_INT32]);
                    delete[] m;
                } else {
                    retval = UA_Variant_setArrayCopy(variant, v, value->size(), &UA_TYPES[UA_TYPES_INT32]);
                }
                break;
            }
        }
        case DT_FLOAT: {
            if (isScalar) {
                float v = value->getFloat();
                retval = UA_Variant_setScalarCopy(variant, &v, &UA_TYPES[UA_TYPES_FLOAT]);
                break;
            } else {
                float *v = (float *)value->getDataArray();
                if (isMatrix) {
                    float *m = new float[rows * columns];
                    for (INDEX i = 0; i < columns; i++) {
                        for (INDEX j = 0; j < rows; j++) {
                            m[j * columns + i] = v[i * rows + j];
                        }
                    }
                    retval = UA_Variant_setArrayCopy(variant, m, rows * columns, &UA_TYPES[UA_TYPES_FLOAT]);
                    delete[] m;
                } else {
                    retval = UA_Variant_setArrayCopy(variant, v, value->size(), &UA_TYPES[UA_TYPES_FLOAT]);
                }
                break;
            }
        }
        case DT_DOUBLE: {
            if (isScalar) {
                double v = value->getDouble();
                retval = UA_Variant_setScalarCopy(variant, &v, &UA_TYPES[UA_TYPES_DOUBLE]);
                break;
            } else {
                double *v = (double *)value->getDataArray();
                if (isMatrix) {
                    vector<double> matrixV(rows * columns);
                    for (INDEX i = 0; i < columns; i++) {
                        for (INDEX j = 0; j < rows; j++) {
                            matrixV[j * columns + i] = v[i * rows + j];
                        }
                    }
                    retval =
                        UA_Variant_setArrayCopy(variant, matrixV.data(), rows * columns, &UA_TYPES[UA_TYPES_DOUBLE]);
                } else {
                    retval = UA_Variant_setArrayCopy(variant, v, value->size(), &UA_TYPES[UA_TYPES_DOUBLE]);
                }
                break;
            }
        }
        case DT_LONG: {
            if (isScalar) {
                long long v = value->getLong();
                retval = UA_Variant_setScalarCopy(variant, &v, &UA_TYPES[UA_TYPES_INT64]);
                break;
            } else {
                long long *v = (long long *)value->getDataArray();
                if (isMatrix) {
                    long long *m = new long long[rows * columns];
                    for (INDEX i = 0; i < columns; i++) {
                        for (INDEX j = 0; j < rows; j++) {
                            m[j * columns + i] = v[i * rows + j];
                        }
                    }
                    retval = UA_Variant_setArrayCopy(variant, m, rows * columns, &UA_TYPES[UA_TYPES_INT64]);
                    delete[] m;
                } else {
                    retval = UA_Variant_setArrayCopy(variant, v, value->size(), &UA_TYPES[UA_TYPES_INT64]);
                }
                break;
            }
        }
        case DT_SYMBOL:
        case DT_STRING: {
            if (isScalar) {
                string s = value->getString();
                UA_String uas = UA_STRING(&s[0]);
                retval = UA_Variant_setScalarCopy(variant, &uas, &UA_TYPES[UA_TYPES_STRING]);
                break;
            } else {
                UA_String *uas = (UA_String *)UA_Array_new(rows * columns, &UA_TYPES[UA_TYPES_STRING]);
                if (isMatrix) {
                    vector<string> v;
                    int length = value->size();
                    for (int i = 0; i < length; ++i) {
                        v.push_back(value->getString(i));
                    }
                    for (INDEX i = 0; i < columns; i++) {
                        for (INDEX j = 0; j < rows; j++) {
                            uas[j * columns + i] = UA_STRING_ALLOC(&v[i * rows + j][0]);
                        }
                    }
                } else {
                    std::vector<char*> buf(Util::BUF_SIZE);
                    INDEX start = 0;
                    INDEX len = value->size();
                    int count = 0;
                    char **data = nullptr;
                    while (start < len) {
                        count = std::min(len - start, Util::BUF_SIZE);
                        data = value->getStringConst(start, count, buf.data());
                        for (int i = 0; i < count; i++) {
                            uas[i + start] = UA_STRING_ALLOC(data[i]);
                        }
                        start += count;
                    }
                }
                UA_Variant_setArray(variant, uas, rows * columns, &UA_TYPES[UA_TYPES_STRING]);
                break;
            }
        }
        case DT_SHORT: {
            if (isScalar) {
                short v = value->getShort();
                retval = UA_Variant_setScalarCopy(variant, &v, &UA_TYPES[UA_TYPES_INT16]);
                break;
            } else {
                short *v = (short *)value->getDataArray();
                if (isMatrix) {
                    short *m = new short[rows * columns];
                    for (INDEX i = 0; i < columns; i++) {
                        for (INDEX j = 0; j < rows; j++) {
                            m[j * columns + i] = v[i * rows + j];
                        }
                    }
                    retval = UA_Variant_setArrayCopy(variant, m, rows * columns, &UA_TYPES[UA_TYPES_INT16]);
                    delete[] m;
                } else {
                    retval = UA_Variant_setArrayCopy(variant, v, value->size(), &UA_TYPES[UA_TYPES_INT16]);
                }
                break;
            }
        }
        case DT_UUID: {
            if (isScalar) {
                Guid v = value->getInt128();
                UA_Guid guid = convertUuidToUA_Guid(v.bytes());
                retval = UA_Variant_setScalarCopy(variant, &guid, &UA_TYPES[UA_TYPES_GUID]);
                break;
            } else {
                Guid *v = (Guid *)value->getDataArray();
                UA_Guid *guid = (UA_Guid *)UA_Array_new(rows * columns, &UA_TYPES[UA_TYPES_GUID]);
                if (isMatrix) {
                    for (INDEX i = 0; i < columns; i++) {
                        for (INDEX j = 0; j < rows; j++) {
                            guid[j * columns + i] = convertUuidToUA_Guid(v[i * rows + j].bytes());
                        }
                    }
                } else {
                    for (int i = 0; i < value->size(); i++) {
                        guid[i] = convertUuidToUA_Guid(v[i].bytes());
                    }
                }
                retval = UA_Variant_setArrayCopy(variant, guid, rows * columns, &UA_TYPES[UA_TYPES_GUID]);
                UA_Array_delete(guid, rows * columns, &UA_TYPES[UA_TYPES_GUID]);
                break;
            }
        }
        default:
            return false;
    }
    if (isMatrix) {
        variant->arrayDimensionsSize = 2;
        variant->arrayDimensions = (UA_UInt32 *)UA_Array_new(2, &UA_TYPES[UA_TYPES_UINT32]);
        variant->arrayDimensions[0] = (UA_UInt32)rows;
        variant->arrayDimensions[1] = (UA_UInt32)columns;
    }
    if (retval == UA_STATUSCODE_GOOD) return true;
    return false;
}

string UAStringToString(UA_String uastr) { return string(uastr.data, uastr.data + uastr.length); }

ConstantSP toConstant(UA_Variant variant) {
    bool flag = variant.arrayLength <= 1;
    char delimiter = ',';
    char rowDelimiter = ';';
    size_t dimensionsSize = variant.arrayDimensionsSize;
    if (dimensionsSize > 2) {
        throw RuntimeException(OPCUA_PREFIX + "Dimensions size larger than 2 is not supported yet.");
    }
    UA_UInt32 nRows = 0;
    UA_UInt32 nCols = 0;
    if (dimensionsSize == 2) {
        nRows = *variant.arrayDimensions;
        nCols = *(variant.arrayDimensions + 1);
    } else if (dimensionsSize == 1) {
        nRows = 1;
        nCols = *variant.arrayDimensions;
    } else if (dimensionsSize == 0) {
        nRows = 1;
        nCols = variant.arrayLength;
    }
    switch (variant.type->typeIndex) {
        case UA_TYPES_BOOLEAN:
            if (flag)
                return new Bool(*(bool *)variant.data);
            else {
                string s;
                if (nRows * nCols != variant.arrayLength)
                    throw RuntimeException(OPCUA_PREFIX + "variant arrayLength not matched.");
                for (UA_UInt32 row = 0; row < nRows; ++row) {
                    for (UA_UInt32 col = 0; col < nCols; ++col) {
                        if (*((bool *)variant.data + row * nCols + col) == true)
                            s += "true";
                        else
                            s += "false";
                        if (col < nCols - 1) {
                            s += delimiter;
                        }
                    }
                    s += rowDelimiter;
                }
                return new String(s);
            }
        case UA_TYPES_FLOAT:
            if (flag)
                return new Float(*(float *)variant.data);
            else {
                string s;
                if (nRows * nCols != variant.arrayLength)
                    throw RuntimeException(OPCUA_PREFIX + "variant arrayLength not matched.");
                for (UA_UInt32 row = 0; row < nRows; ++row) {
                    for (UA_UInt32 col = 0; col < nCols; ++col) {
                        s += std::to_string(*((float *)(variant.data) + row * nCols + col));
                        if (col < nCols - 1) {
                            s += delimiter;
                        }
                    }
                    s += rowDelimiter;
                }
                return new String(s);
            }
        case UA_TYPES_DOUBLE:
            if (flag)
                return new Double(*(double *)variant.data);
            else {
                string s;
                if (nRows * nCols != variant.arrayLength)
                    throw RuntimeException(OPCUA_PREFIX + "variant arrayLength not matched.");
                for (UA_UInt32 row = 0; row < nRows; ++row) {
                    for (UA_UInt32 col = 0; col < nCols; ++col) {
                        s += std::to_string(*((double *)(variant.data) + row * nCols + col));
                        if (col < nCols - 1) {
                            s += delimiter;
                        }
                    }
                    s += rowDelimiter;
                }
                return new String(s);
            }
        case UA_TYPES_UINT16:
            if (flag)
                return new Int((int)(*(UA_UInt16 *)variant.data));
            else {
                string s;
                if (nRows * nCols != variant.arrayLength)
                    throw RuntimeException(OPCUA_PREFIX + "variant arrayLength not matched.");
                for (UA_UInt32 row = 0; row < nRows; ++row) {
                    for (UA_UInt32 col = 0; col < nCols; ++col) {
                        s += std::to_string((int)(*((UA_UInt16 *)variant.data + row * nCols + col)));
                        if (col < nCols - 1) {
                            s += delimiter;
                        }
                    }
                    s += rowDelimiter;
                }
                return new String(s);
            }
        case UA_TYPES_INT32:
            if (flag)
                return new Int(*(int *)variant.data);
            else {
                string s;
                if (nRows * nCols != variant.arrayLength)
                    throw RuntimeException(OPCUA_PREFIX + "variant arrayLength not matched.");
                for (UA_UInt32 row = 0; row < nRows; ++row) {
                    for (UA_UInt32 col = 0; col < nCols; ++col) {
                        s += std::to_string(*((int *)(variant.data) + row * nCols + col));
                        if (col < nCols - 1) {
                            s += delimiter;
                        }
                    }
                    s += rowDelimiter;
                }
                return new String(s);
            }
        case UA_TYPES_UINT32:
            if (flag)
                return new Long((long long)(*(UA_UInt32 *)variant.data));
            else {
                string s;
                if (nRows * nCols != variant.arrayLength)
                    throw RuntimeException(OPCUA_PREFIX + "variant arrayLength not matched.");
                for (UA_UInt32 row = 0; row < nRows; ++row) {
                    for (UA_UInt32 col = 0; col < nCols; ++col) {
                        s += std::to_string((long long)(*((UA_UInt32 *)(variant.data) + row * nCols + col)));
                        if (col < nCols - 1) {
                            s += delimiter;
                        }
                    }
                    s += rowDelimiter;
                }
                return new String(s);
            }
        case UA_TYPES_INT64:
            if (flag)
                return new Long(*(long long *)variant.data);
            else {
                string s;
                if (nRows * nCols != variant.arrayLength)
                    throw RuntimeException(OPCUA_PREFIX + "variant arrayLength not matched.");
                for (UA_UInt32 row = 0; row < nRows; ++row) {
                    for (UA_UInt32 col = 0; col < nCols; ++col) {
                        s += std::to_string(*((long long *)(variant.data) + row * nCols + col));
                        if (col < nCols - 1) {
                            s += delimiter;
                        }
                    }
                    s += rowDelimiter;
                }
                return new String(s);
            }
        case UA_TYPES_SBYTE:
            if (flag)
                return new Char(*(char *)variant.data);
            else {
                string s;
                if (nRows * nCols != variant.arrayLength)
                    throw RuntimeException(OPCUA_PREFIX + "variant arrayLength not matched.");
                for (UA_UInt32 row = 0; row < nRows; ++row) {
                    for (UA_UInt32 col = 0; col < nCols; ++col) {
                        s += std::to_string(*((char *)(variant.data) + row * nCols + col));
                        if (col < nCols - 1) {
                            s += delimiter;
                        }
                    }
                    s += rowDelimiter;
                }
                return new String(s);
            }
        case UA_TYPES_BYTE:
            if (flag)
                return new Short((short)(*(unsigned char *)variant.data));
            else {
                string s;
                if (nRows * nCols != variant.arrayLength)
                    throw RuntimeException(OPCUA_PREFIX + "variant arrayLength not matched.");
                for (UA_UInt32 row = 0; row < nRows; ++row) {
                    for (UA_UInt32 col = 0; col < nCols; ++col) {
                        s += std::to_string((short)(*((unsigned char *)(variant.data) + row * nCols + col)));
                        if (col < nCols - 1) {
                            s += delimiter;
                        }
                    }
                    s += rowDelimiter;
                }
                return new String(s);
            }
        case UA_TYPES_DATETIME:
            if (flag) {
                UA_DateTime time = *(UA_DateTime *)variant.data;
                return new Timestamp(UA_DateTime_toUnixTimeStamp(time));
            } else {
                throw RuntimeException(OPCUA_PREFIX + "timestamp vector or timestamp matrix is not supported yet");
            }
        case UA_TYPES_INT16:
            if (flag)
                return new Short(*(short *)variant.data);
            else {
                string s;
                if (nRows * nCols != variant.arrayLength)
                    throw RuntimeException(OPCUA_PREFIX + "variant arrayLength not matched.");
                for (UA_UInt32 row = 0; row < nRows; ++row) {
                    for (UA_UInt32 col = 0; col < nCols; ++col) {
                        s += std::to_string(*((short *)(variant.data) + row * nCols + col));
                        if (col < nCols - 1) {
                            s += delimiter;
                        }
                    }
                    s += rowDelimiter;
                }
                return new String(s);
            }
        case UA_TYPES_BYTESTRING:
        case UA_TYPES_STRING:
            if (flag)
                return new String(UAStringToString(*(UA_String *)variant.data));
            else {
                string s;
                if (nRows * nCols != variant.arrayLength)
                    throw RuntimeException(OPCUA_PREFIX + "variant arrayLength not matched.");
                for (UA_UInt32 row = 0; row < nRows; ++row) {
                    for (UA_UInt32 col = 0; col < nCols; ++col) {
                        s += UAStringToString(*((UA_String *)(variant.data) + row * nCols + col));
                        if (col < nCols - 1) {
                            s += delimiter;
                        }
                    }
                    s += rowDelimiter;
                }
                return new String(s);
            }
        case UA_TYPES_GUID:
            if (flag) {
                unsigned char data[16];
                convertUA_Guid(*(UA_Guid *)variant.data, data);
                return new Uuid(data);
            } else {
                throw RuntimeException(OPCUA_PREFIX + "Guid vector or Guid matrix is not supported yet.");
            }
        default:
            throw RuntimeException(OPCUA_PREFIX + "type " + std::to_string(variant.type->typeIndex) +
                                   " is not supported yet");
    }
}

void OPCUASub::reconnect() {
    bool success = false;
    while (!success && running_) {
        Util::sleep(resubTimeout_);
        try {
            client_->reconnect();
            subs();
            success = true;
            reconnectTime_ = Util::getNanoEpochTime() + timeGap_;
            isConnected_ = true;
        } catch (std::exception &e) {
            PLUGIN_LOG_INFO(OPCUA_PREFIX, "subscription [" + actionName_ + "] reconnection failed due to ", e.what());
        }
    }
}

void OPCUASub::startThread() {
    running_ = true;
    auto f = [this]() {
        try {
            while (running_) {
                UA_StatusCode retVal = UA_Client_run_iterate(client_->getClientPtr(), 100);
                if (retVal != UA_STATUSCODE_GOOD) {
                    long long dataTime = Util::getNanoEpochTime() + timeGap_;
                    isConnected_ = false;
                    if (!client_->getConnected() && reconnect_) {
                        string errMsg =
                            "subscription [" + actionName_ + "] has been disconnected. Trying to reconnect.";
                        status_.lastErrMsg_ = errMsg;
                        status_.lastFailedTimestamp_ = dataTime;
                        PLUGIN_LOG_WARN(OPCUA_PREFIX, errMsg);
                        reconnect();
                    } else {
                        string errMsg = "Error occurs in OPCUA sub: " + string(UA_StatusCode_name(retVal));
                        status_.lastErrMsg_ = errMsg;
                        status_.lastFailedTimestamp_ = dataTime;
                        PLUGIN_LOG_ERR(OPCUA_PREFIX, errMsg);
                        break;
                    }
                }
            }
        } catch (...) {
            string errMsg = "Error occurs in OPCUA sub";
            status_.lastErrMsg_ = errMsg;
            long long dataTime = Util::getNanoEpochTime() + timeGap_;
            status_.lastFailedTimestamp_ = dataTime;
            PLUGIN_LOG_ERR(OPCUA_PREFIX, errMsg);
        }
    };
    thread_ = new Thread(new ddb::Executor(f));
    thread_->start();
}

void OPCUASub::stopThread() {
    LockGuard<Mutex> lock(&mutex_);
    if (running_) {
        running_ = false;
        if (!thread_.isNull()) {
            thread_->join();
        }
        unSub();
    }
}

UA_Client *OPCUASub::getClientPtr() const { return client_->getClientPtr(); }

void OPCUASub::unSub() {
    UA_StatusCode retVal = UA_Client_Subscriptions_deleteSingle(client_->getClientPtr(), subId_);
    if (retVal != UA_STATUSCODE_GOOD) {
        throw RuntimeException(OPCUA_PREFIX + OPCUA_PREFIX + "Could not call unsubscribe service. StatusCode " +
                               string(UA_StatusCode_name(retVal)));
    }
}

OPCUASub::OPCUASub(Heap *heap, string endPoint, string clientUri, OPCUAClientSP client, const vector<int> &nsIdx,
                   const vector<string> &nodeIdString, ConstantSP &handle, bool reconnect, long long resubTimeout,
                   const string &actionName)
    : reconnect_(reconnect),
      resubTimeout_(resubTimeout),
      actionName_(actionName),
      endPoint_(endPoint),
      clientUri_(clientUri),
      client_(client),
      handle_(handle),
      heap_(heap),
      nsIdx_(nsIdx),
      nodeIdString_(nodeIdString) {
    user_ = heap->currentSession()->getUser()->getUserId();
    long long time = Util::getNanoEpochTime();
    timeGap_ = Util::toLocalNanoTimestamp(time) - time;
}
void OPCUASub::subs() {
    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    UA_CreateSubscriptionResponse response =
        UA_Client_Subscriptions_create(client_->getClientPtr(), request, NULL, NULL, NULL);
    if (response.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
        throw RuntimeException(OPCUA_PREFIX + "Could not call Subscribe service. StatusCode " +
                               string(UA_StatusCode_name(response.responseHeader.serviceResult)));
    int numNode = nsIdx_.size();
    subId_ = response.subscriptionId;
    monMap_.clear();  // clean previous subs info
    for (int i = 0; i < numNode; i++) {
        UA_NodeId nodeId = UA_NODEID_STRING_ALLOC(nsIdx_[i], nodeIdString_[i].c_str());
        UA_MonitoredItemCreateRequest monRequest = UA_MonitoredItemCreateRequest_default(nodeId);
        UA_MonitoredItemCreateResult monResponse = UA_Client_MonitoredItems_createDataChange(
            client_->getClientPtr(), response.subscriptionId, UA_TIMESTAMPSTORETURN_BOTH, monRequest, NULL,
            handlerTheAnswerChanged, NULL);
        if (monResponse.statusCode != UA_STATUSCODE_GOOD) {
            throw RuntimeException(OPCUA_PREFIX + "Could not call Subscribe service. StatusCode " +
                                   string(UA_StatusCode_name(monResponse.statusCode)));
        }
        monMap_[monResponse.monitoredItemId] = string(std::to_string(nsIdx_[i]) + " : " + nodeIdString_[i]);
    }

    if (actionName_.empty()) {  // HACK when re-sub, the actionName_ already exist, the actionName won't change
        actionName_ = std::to_string((long long)client_->getClientPtr()) + "_" + std::to_string(subId_);
    }
}

static void handlerTheAnswerChanged(UA_Client *client, UA_UInt32 subId, void *subContext, UA_UInt32 monId,
                                    void *monContext, UA_DataValue *value) {
    std::ignore = subId;
    std::ignore = subContext;
    std::ignore = monContext;
    LockGuard<Mutex> _(&OPCUASub::OPCUA_SUB_MAP_LATCH);
    long long clientLong = (long long)client;
    auto it = OPCUASub::OPCUA_SUB_MAP.find(clientLong);
    if (it == OPCUASub::OPCUA_SUB_MAP.end()) {
        PLUGIN_LOG_INFO(OPCUA_PREFIX, "invalid client: ", client);
        return;
    }
    OPCUASubSP sub = it->second;
    long long dataTime = Util::getNanoEpochTime() + sub->getTimeGap();
    if (sub->getStatus().firstMsgTime_ == LONG_LONG_MIN) {
        sub->getStatus().firstMsgTime_ = dataTime;
    }
    sub->getStatus().lastMsgTime_ = dataTime;
    try {
        string nodeid = sub->monMap_[monId];
        if (!value || !value->value.data) {
            throw RuntimeException("Invalid UA_DataValue.");
        }
        ConstantSP val = toConstant(value->value);
        ConstantSP nodeIdSP = Util::createConstant(DT_STRING);
        nodeIdSP->setString(nodeid);
        ConstantSP sourceTimestamp = Util::createConstant(DT_TIMESTAMP);
        if (value->hasSourceTimestamp) {
            sourceTimestamp->setLong(UA_DateTime_toUnixTimeStamp(value->sourceTimestamp));
        } else {
            sourceTimestamp->setLong(INT64_MIN);
        }
        ConstantSP status = Util::createConstant(DT_STRING);
        status->setString(string(UA_StatusCode_name(value->status)));
        vector<ConstantSP> cols = {nodeIdSP, val, sourceTimestamp, status};
        vector<string> colNames = {"node id", "value", "timestamp", "status"};
        TableSP resultTable = Util::createTable(colNames, cols);
        if (sub->getHandle()->isTable()) {
            TableSP t = (TableSP)sub->getHandle();
            if (t->isSegmentedTable()) {
                vector<ConstantSP> args = {t, resultTable};
                Heap *h = sub->getHeap();
                h->currentSession()->getFunctionDef("append!")->call(h, args);
            } else {
                INDEX insertedRows = 1;
                string errMsg;
                vector<ConstantSP> args = {resultTable};
                LockGuard<Mutex> lock(t->getLock());
                bool add = t->append(args, insertedRows, errMsg);
                if (!add) {
                    throw RuntimeException(errMsg);
                }
            }
        } else {
            vector<ConstantSP> args = {resultTable};
            auto heap = sub->getHeap();
            FunctionDefSP sp = sub->getHandle();
            sp->call(heap, args);
        }
        sub->getStatus().processedMsgCount_++;
    } catch (std::exception &e) {
        PLUGIN_LOG_ERR(OPCUA_PREFIX, e.what());
        sub->getStatus().lastErrMsg_ = e.what();
        sub->getStatus().failedMsgCount_++;
        sub->getStatus().lastFailedTimestamp_ = dataTime;
    }
}

ConstantSP OPCUAClient::readNode(const vector<int> &nsIdx, const vector<string> &nodeIdString, ConstantSP &table) {
    LockGuard<Mutex> _(&mutex_);
    if (isSubscribed) {
        throw RuntimeException(OPCUA_PREFIX + "A subscribed client cannot be used for other operations.");
    }
    int numNode = nsIdx.size();
    UA_ReadRequest rReq;
    UA_ReadRequest_init(&rReq);
    rReq.nodesToReadSize = numNode;
    rReq.nodesToRead = (UA_ReadValueId *)UA_Array_new(numNode, &UA_TYPES[UA_TYPES_READVALUEID]);
    for (int i = 0; i < numNode; i++) {
        rReq.nodesToRead[i].nodeId = UA_NODEID_STRING_ALLOC((UA_UInt16)nsIdx[i], nodeIdString[i].c_str());
        rReq.nodesToRead[i].attributeId = UA_ATTRIBUTEID_VALUE;
    }

    UA_ReadResponse rResp = UA_Client_Service_read(clientPtr_, rReq);
    if (rResp.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
        throw RuntimeException(OPCUA_PREFIX + "Could not call ReadNodesValue service. StatusCode " +
                               string(UA_StatusCode_name(rResp.responseHeader.serviceResult)));
    if (table->isArray()) {
        if ((int)rResp.resultsSize != numNode) throw RuntimeException(OPCUA_PREFIX + "Can't read all nodes. ");
        for (size_t i = 0; i < rResp.resultsSize; i++) {
            ConstantSP status = new String(string(UA_StatusCode_name(rResp.results[i].status)));
            if (rResp.results[i].status == UA_STATUSCODE_BADNODEIDUNKNOWN)
                throw RuntimeException(OPCUA_PREFIX + "Could not call ReadNodesValue service. Node " +
                                       std::to_string(nsIdx[i]) + ":" + nodeIdString[i] + " not exist");
            ConstantSP value;
            if (rResp.results[i].hasValue == true && rResp.results[i].value.data) {
                value = toConstant(rResp.results[i].value);
            } else {
                value = new Double(DBL_NMIN);
            }
            ConstantSP nodeId = Util::createConstant(DT_STRING);
            nodeId->setString(std::to_string(nsIdx[i]) + " : " + nodeIdString[i]);
            ConstantSP sourceTimestamp = Util::createConstant(DT_TIMESTAMP);
            if (rResp.results[i].hasSourceTimestamp) {
                sourceTimestamp->setLong(UA_DateTime_toUnixTimeStamp(rResp.results[i].sourceTimestamp));
            } else
                sourceTimestamp->setLong(INT64_MIN);
            vector<string> colNames = {"node id", "value", "timestamp", "status"};
            vector<ConstantSP> cols = {nodeId, value, sourceTimestamp, status};
            TableSP resultTable = Util::createTable(colNames, cols);
            vector<ConstantSP> args1 = {resultTable};

            INDEX insertedRows = 1;
            string errMsg;
            TableSP tp = table->get(i);
            LockGuard<Mutex> lock(tp->getLock());
            tp->append(args1, insertedRows, errMsg);
            if (insertedRows != resultTable->rows()) {
                throw RuntimeException(OPCUA_PREFIX + errMsg);
            }
        }
        vector<string> colNames{"col"};
        vector<ConstantSP> cols{Util::createVector(DT_INT, 0, 0)};
        return Util::createTable(colNames, cols);
    } else {
        ConstantSP valueVec;
        if (rResp.results[0].status == UA_STATUSCODE_BADNODEIDUNKNOWN)
            throw RuntimeException(OPCUA_PREFIX + "Could not call ReadNodesValue service. Node " +
                                   std::to_string(nsIdx[0]) + ":" + nodeIdString[0] + " not exist");
        if (rResp.results[0].hasValue && rResp.results[0].value.data) {
            valueVec = Util::createVector(toConstant(rResp.results[0].value)->getType(), numNode);
        } else {
            valueVec = Util::createVector(DT_VOID, numNode);
        }
        ConstantSP nodeIdVec = Util::createVector(DT_STRING, numNode);
        ConstantSP sourceTimestampVec = Util::createVector(DT_TIMESTAMP, numNode);
        ConstantSP statusVec = Util::createVector(DT_STRING, numNode);
        for (size_t i = 0; i < rResp.resultsSize; i++) {
            ConstantSP status = Util::createConstant(DT_STRING);
            if (rResp.results[i].status == UA_STATUSCODE_BADNODEIDUNKNOWN)
                throw RuntimeException(OPCUA_PREFIX + "Could not call ReadNodesValue service. Node " +
                                       std::to_string(nsIdx[i]) + ":" + nodeIdString[i] + " not exist");
            status->setString(string(UA_StatusCode_name(rResp.results[i].status)));
            statusVec->set(i, status);
            if (rResp.results[i].hasValue && rResp.results[i].value.data)
                valueVec->set(i, toConstant(rResp.results[i].value));
            else
                valueVec->set(i, Util::createNullConstant(valueVec->getType()));
            ConstantSP nodeId = Util::createConstant(DT_STRING);
            nodeId->setString(std::to_string(nsIdx[i]) + " : " + nodeIdString[i]);
            nodeIdVec->set(i, nodeId);
            ConstantSP sourceTimestamp = Util::createConstant(DT_TIMESTAMP);
            if (rResp.results[i].hasSourceTimestamp) {
                sourceTimestamp->setLong(UA_DateTime_toUnixTimeStamp(rResp.results[i].sourceTimestamp));
            } else
                sourceTimestamp->setLong(INT64_MIN);
            sourceTimestampVec->set(i, sourceTimestamp);
        }
        if (table->isNull()) {
            vector<string> colNames = {"node id", "value", "timestamp", "status"};
            vector<ConstantSP> cols = {nodeIdVec, valueVec, sourceTimestampVec, statusVec};
            TableSP resultTable = Util::createTable(colNames, cols);
            return resultTable;
        } else {
            TableSP tp = table;
            vector<string> colNames = {"node id", "value", "timestamp", "status"};
            vector<ConstantSP> cols = {nodeIdVec, valueVec, sourceTimestampVec, statusVec};
            TableSP resultTable = Util::createTable(colNames, cols);
            vector<ConstantSP> args1 = {resultTable};
            INDEX insertedRows = 1;
            string errMsg;

            LockGuard<Mutex> lock(tp->getLock());
            tp->append(args1, insertedRows, errMsg);
            if (insertedRows != resultTable->rows()) {
                throw RuntimeException(OPCUA_PREFIX + errMsg);
            }
            return resultTable;
        }
    }
}

ConstantSP OPCUAClient::writeNode(const vector<int> &nsIdx, const vector<string> &nodeIdString, ConstantSP &value) {
    LockGuard<Mutex> _(&mutex_);
    if (isSubscribed) {
        throw RuntimeException(OPCUA_PREFIX + "A subscribed client cannot be used for other operations.");
    }
    int numNode = nsIdx.size();
    for (int i = 0; i < numNode; ++i) {
        UA_Variant *myVariant = UA_Variant_new();
        ConstantSP val;
        if (numNode == 1) {
            val = value;
        } else {
            val = value->get(i);
        }
        if (!convertDTToUA(myVariant, val)) {
            UA_Variant_delete(myVariant);
            throw RuntimeException(OPCUA_PREFIX + "unsupported type.");
        }
        UA_StatusCode retval = UA_Client_writeValueAttribute(
            clientPtr_, UA_NODEID_STRING_ALLOC((UA_UInt16)nsIdx[i], nodeIdString[i].c_str()), myVariant);
        UA_Variant_delete(myVariant);
        if (retval != UA_STATUSCODE_GOOD) {
            throw RuntimeException(OPCUA_PREFIX + "Could not call WriteNodesValue service. StatusCode " +
                                   string(UA_StatusCode_name(retval)));
        }
    }
    return new Bool(true);
}

string guidToString(const UA_Guid &guid) {
    unsigned char data[16];
    convertUA_Guid(guid, data);
    return Guid(data).getString();
}

void OPCUAClient::browseNode(UA_NodeId object, vector<int> &nameSpace, vector<string> &nodeid) {
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = object;                      /* browse objects folder */
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
    UA_BrowseResponse bResp = UA_Client_Service_browse(clientPtr_, bReq);
    vector<UA_NodeId> nextNodeId;
    size_t nextCount = 0;
    for (size_t i = 0; i < bResp.resultsSize; ++i) {
        if (nextNodeId.size() - nextCount < bResp.results[i].referencesSize)
            nextNodeId.resize(nextNodeId.size() + bResp.results[i].referencesSize);
        for (size_t j = 1; j < bResp.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
            if (ref->nodeClass == UA_NodeClass::UA_NODECLASS_VARIABLE) {
                auto identifierType = ref->nodeId.nodeId.identifierType;
                if (identifierType == UA_NODEIDTYPE_STRING) {
                    nameSpace.push_back((int)ref->nodeId.nodeId.namespaceIndex);
                    string ids(ref->nodeId.nodeId.identifier.string.data,
                               ref->nodeId.nodeId.identifier.string.data + ref->nodeId.nodeId.identifier.string.length);
                    nodeid.push_back(ids);
                } else if (identifierType == UA_NODEIDTYPE_NUMERIC) {
                    nameSpace.push_back((int)ref->nodeId.nodeId.namespaceIndex);
                    string ids = std::to_string(ref->nodeId.nodeId.identifier.numeric);
                    nodeid.push_back(ids);
                } else if (identifierType == UA_NODEIDTYPE_GUID) {
                    nameSpace.push_back((int)ref->nodeId.nodeId.namespaceIndex);
                    string ids = guidToString(ref->nodeId.nodeId.identifier.guid);
                    nodeid.push_back(ids);
                } else if (identifierType == UA_NODEIDTYPE_BYTESTRING) {
                    nameSpace.push_back((int)ref->nodeId.nodeId.namespaceIndex);
                    string ids(ref->nodeId.nodeId.identifier.byteString.data,
                               ref->nodeId.nodeId.identifier.byteString.data +
                                   ref->nodeId.nodeId.identifier.byteString.length);
                    nodeid.push_back(ids);
                }
            } else if (ref->nodeClass == UA_NodeClass::UA_NODECLASS_OBJECT) {
                int k = nextCount - 1;
                for (; k >= 0; k--) {
                    if (UA_NodeId_equal(&nextNodeId[k], &ref->nodeId.nodeId)) break;
                }
                if (k == -1) {
                    UA_NodeId_copy(&ref->nodeId.nodeId, &nextNodeId[nextCount++]);
                }
            }
        }
    }
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bResp);

    for (size_t i = 0; i < nextCount; i++) {
        browseNode(nextNodeId[i], nameSpace, nodeid);
    }
    return;
}

ConstantSP OPCUAClient::browseNode() {
    LockGuard<Mutex> _(&mutex_);
    if (isSubscribed) {
        throw RuntimeException(OPCUA_PREFIX + "A subscribed client cannot be used for other operations.");
    }
    UA_NodeId object = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    VectorSP nameSpaceVec = Util::createVector(DT_INT, 0);
    VectorSP nodeidVec = Util::createVector(DT_STRING, 0);
    vector<int> nameSpace;
    vector<string> nodeid;
    browseNode(object, nameSpace, nodeid);
    nameSpaceVec->appendInt(nameSpace.data(), nameSpace.size());
    nodeidVec->appendString(nodeid.data(), nodeid.size());
    vector<string> colNames = {"nodeNamespace", "nodeIdString"};
    vector<ConstantSP> cols(2);
    cols[0] = {nameSpaceVec};
    cols[1] = {nodeidVec};
    return Util::createTable(colNames, cols);
}

UA_StatusCode UAClientConfigSetEncryption(UA_ClientConfig *config, UA_ByteString localCertificate,
                                          UA_ByteString privateKey, const UA_ByteString *trustList,
                                          size_t trustListSize, const UA_ByteString *revocationList,
                                          size_t revocationListSize) {
    UA_StatusCode retval = UA_CertificateVerification_Trustlist(
        &config->certificateVerification, trustList, trustListSize, NULL, 0, revocationList, revocationListSize);
    if (retval != UA_STATUSCODE_GOOD) {
        return retval;
    }

    /* Populate SecurityPolicies */
    UA_SecurityPolicy *sp = (UA_SecurityPolicy *)UA_realloc(config->securityPolicies, sizeof(UA_SecurityPolicy) * 4);
    if (!sp) return UA_STATUSCODE_BADOUTOFMEMORY;
    config->securityPolicies = sp;
    config->securityPoliciesSize = 1;

    retval = UA_SecurityPolicy_Basic128Rsa15(&config->securityPolicies[1], &config->certificateVerification,
                                             localCertificate, privateKey, &config->logger);
    if (retval != UA_STATUSCODE_GOOD) return retval;
    ++config->securityPoliciesSize;

    retval = UA_SecurityPolicy_Basic256(&config->securityPolicies[2], &config->certificateVerification,
                                        localCertificate, privateKey, &config->logger);
    if (retval != UA_STATUSCODE_GOOD) return retval;
    ++config->securityPoliciesSize;

    retval = UA_SecurityPolicy_Basic256Sha256(&config->securityPolicies[3], &config->certificateVerification,
                                              localCertificate, privateKey, &config->logger);
    if (retval != UA_STATUSCODE_GOOD) return retval;
    ++config->securityPoliciesSize;

    return UA_STATUSCODE_GOOD;
}

void OPCUAClient::connect(string endPointUrl, string clientUri, string username, string password,
                          UA_MessageSecurityMode securityMode, UA_String securityPolicy, UA_ByteString certificate,
                          UA_ByteString privateKey, bool reconnect) {
    UA_StatusCode retval;
    if (!reconnect) {
        UA_ClientConfig *config = UA_Client_getConfig(clientPtr_);
        UA_ClientConfig_setDefault(config);
        config->clientDescription.applicationUri = UA_STRING_ALLOC(clientUri.c_str());
        config->securityMode = securityMode;
        config->securityPolicyUri = securityPolicy;
        if (!UA_String_equal(&certificate, &UA_STRING_NULL)) {
            retval = UAClientConfigSetEncryption(config, certificate, privateKey, NULL, 0, NULL, 0);
            if (retval != UA_STATUSCODE_GOOD) {
                throw RuntimeException(OPCUA_PREFIX + "Could not call Connect service. StatusCode " +
                                       string(UA_StatusCode_name(retval)));
            }
        }
        if (!username.empty()) {
            UA_UserNameIdentityToken *identityToken = UA_UserNameIdentityToken_new();
            if (!identityToken) {
                throw RuntimeException(OPCUA_PREFIX + "Could not call Connect service. StatusCode " +
                                       string(UA_StatusCode_name(UA_STATUSCODE_BADOUTOFMEMORY)));
            }
            identityToken->userName = UA_STRING_ALLOC(username.c_str());
            identityToken->password = UA_STRING_ALLOC(password.c_str());
            UA_ExtensionObject_clear(&config->userIdentityToken);
            config->userIdentityToken.encoding = UA_EXTENSIONOBJECT_DECODED;
            config->userIdentityToken.content.decoded.type = &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN];
            config->userIdentityToken.content.decoded.data = identityToken;
        }
    }
    retval = UA_Client_connect(clientPtr_, endPointUrl.c_str());
    if (retval != UA_STATUSCODE_GOOD) {
        throw RuntimeException(OPCUA_PREFIX + "Could not call Connect service. StatusCode " +
                               string(UA_StatusCode_name(retval)));
    }

    endPointUrl_ = endPointUrl;
    clientUri_ = clientUri;
    username_ = username;
    password_ = password;
    securityMode_ = securityMode;
    securityPolicy_ = securityPolicy;
    certificate_ = certificate;
    privateKey_ = privateKey;
    return;
}

void OPCUAClient::reconnect() {
    connect(endPointUrl_, clientUri_, username_, password_, securityMode_, securityPolicy_, certificate_, privateKey_,
            true);
}

ConstantSP getOpcServerList(string serverUrl) {
    UA_ApplicationDescription *registeredServers = NULL;
    size_t registeredServersSize = 0;
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode retval =
        UA_Client_findServers(client, serverUrl.c_str(), 0, NULL, 0, NULL, &registeredServersSize, &registeredServers);

    if (retval != UA_STATUSCODE_GOOD) {
        UA_Array_delete(registeredServers, registeredServersSize, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
        UA_Client_delete(client);
        throw RuntimeException(OPCUA_PREFIX + "Could not call FindServers service. StatusCode " +
                               string(UA_StatusCode_name(retval)));
    }

    vector<string> serverUriList(registeredServersSize);
    vector<string> serverNameList(registeredServersSize);
    vector<string> serverProductUriList(registeredServersSize);
    vector<string> severTypeList(registeredServersSize);
    vector<string> serverUrlList(registeredServersSize);

    // output all the returned/registered servers
    for (size_t i = 0; i < registeredServersSize; i++) {
        UA_ApplicationDescription *description = &registeredServers[i];
        serverUriList[i] = string(description->applicationUri.data,
                                  description->applicationUri.data + description->applicationUri.length);
        serverNameList[i] = string(description->applicationName.text.data,
                                   description->applicationName.text.data + description->applicationName.text.length);
        serverProductUriList[i] =
            string(description->productUri.data, description->productUri.data + description->productUri.length);
        switch (description->applicationType) {
            case UA_APPLICATIONTYPE_SERVER:
                severTypeList[i] = "Server";
                break;
            case UA_APPLICATIONTYPE_CLIENT:
                severTypeList[i] = "Client";
                break;
            case UA_APPLICATIONTYPE_CLIENTANDSERVER:
                severTypeList[i] = "Client and Server";
                break;
            case UA_APPLICATIONTYPE_DISCOVERYSERVER:
                severTypeList[i] = "Discovery Server";
                break;
            default:
                severTypeList[i] = "Unknown";
        }

        for (size_t j = 0; j < description->discoveryUrlsSize; j++) {
            serverUrlList[i] += string(description->discoveryUrls[j].data,
                                       description->discoveryUrls[j].data + description->discoveryUrls[j].length);
            serverUrlList[i] += "; ";
        }
    }
    vector<string> colNames = {"ServerUri", "ServerName", "ProductUri", "Type", "DiscoveryUrl"};
    ConstantSP ServerUri = createSVCFromVector(serverUriList, DT_STRING);
    ConstantSP ServerName = createSVCFromVector(serverNameList, DT_STRING);
    ConstantSP ProductUri = createSVCFromVector(serverProductUriList, DT_STRING);
    ConstantSP Type = createSVCFromVector(severTypeList, DT_SYMBOL);
    ConstantSP DiscoveryUrl = createSVCFromVector(serverUrlList, DT_STRING);
    vector<ConstantSP> cols = {ServerUri, ServerName, ProductUri, Type, DiscoveryUrl};
    Table *retTable = Util::createTable(colNames, cols);
    UA_Array_delete(registeredServers, registeredServersSize, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
    UA_Client_delete(client);
    return retTable;
}

ConstantSP getOpcEndPointList(string serverUrl) {
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_EndpointDescription *endpointArray = NULL;
    size_t endpointArraySize = 0;
    // TODO: adapt to the new async getEndpoint
    UA_StatusCode retval = UA_Client_getEndpoints(client, serverUrl.c_str(), &endpointArraySize, &endpointArray);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_disconnect(client);
        UA_Client_delete(client);
        throw RuntimeException(OPCUA_PREFIX + "Could not call GetEndPoint service. StatusCode " +
                               string(UA_StatusCode_name(retval)));
    }
    vector<string> endpointUrl(endpointArraySize);
    vector<string> transportProfileUri(endpointArraySize);
    vector<string> securityMode(endpointArraySize);
    vector<string> securityPolicyUri(endpointArraySize);
    vector<short> securityLevel(endpointArraySize);
    for (size_t j = 0; j < endpointArraySize; j++) {
        UA_EndpointDescription *endpoint = &endpointArray[j];
        endpointUrl[j] = string(endpoint->endpointUrl.data, endpoint->endpointUrl.data + endpoint->endpointUrl.length);
        transportProfileUri[j] = string(endpoint->transportProfileUri.data,
                                        endpoint->transportProfileUri.data + endpoint->transportProfileUri.length);
        switch (endpoint->securityMode) {
            case UA_MESSAGESECURITYMODE_INVALID:
                securityMode[j] = "Invalid";
                break;
            case UA_MESSAGESECURITYMODE_NONE:
                securityMode[j] = "None";
                break;
            case UA_MESSAGESECURITYMODE_SIGN:
                securityMode[j] = "Sign";
                break;
            case UA_MESSAGESECURITYMODE_SIGNANDENCRYPT:
                securityMode[j] = "Sign and Encrypt";
                break;
            default:
                securityMode[j] = "No valid security mode";
                break;
        }
        securityPolicyUri[j] = string(endpoint->securityPolicyUri.data,
                                      endpoint->securityPolicyUri.data + endpoint->securityPolicyUri.length);
        securityLevel[j] = (short)(endpoint->securityLevel);
    }
    vector<string> colNames = {"EndpointUrl", "TransportProfileUri", "SecurityMode", "SecurityPolicyUri",
                               "SecurityLevel"};
    ConstantSP endpointUrlSP = createSVCFromVector(endpointUrl, DT_STRING);
    ConstantSP transportProfileUriSP = createSVCFromVector(transportProfileUri, DT_STRING);
    ConstantSP securityModeSP = createSVCFromVector(securityMode, DT_SYMBOL);
    ConstantSP securityPolicyUriSP = createSVCFromVector(securityPolicyUri, DT_STRING);
    ConstantSP securityLevelSP = createSVCFromVector(securityLevel);
    vector<ConstantSP> cols = {endpointUrlSP, transportProfileUriSP, securityModeSP, securityPolicyUriSP,
                               securityLevelSP};
    Table *retTable = Util::createTable(colNames, cols);
    UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    UA_Client_delete(client);
    return retTable;
}

ConstantSP getOpcUaServerList(Heap *heap, vector<ConstantSP> &arguments) {
    std::ignore = heap;
    string usage{"Usage: getOpcServerList(serverUrl) "};
    if ((arguments[0]->getType() != DT_STRING) || !arguments[0]->isScalar()) {
        throw IllegalArgumentException(__FUNCTION__, usage + "the host must be string scalar");
    }
    // create server
    auto severUrl = arguments[0]->getString();

    return getOpcServerList(severUrl);
}

ConstantSP getOpcUaEndPointList(Heap *heap, vector<ConstantSP> &arguments) {
    std::ignore = heap;
    string usage{"Usage: getOpcEndPointList(serverUrl) "};
    if ((arguments[0]->getType() != DT_STRING) || !arguments[0]->isScalar()) {
        throw IllegalArgumentException(__FUNCTION__, usage + "the host must be string scalar");
    }
    // create server
    auto severUrl = arguments[0]->getString();

    return getOpcEndPointList(severUrl);
}

static UA_INLINE UA_ByteString loadFile(string path) {
    UA_ByteString fileContents = UA_STRING_NULL;

    /* Open the file */
    FILE *fp = fopen(path.c_str(), "rb");
    if (!fp) {
        throw RuntimeException(OPCUA_PREFIX + "Can't open file " + path);
    }

    /* Get the file length, allocate the data and read */
    fseek(fp, 0, SEEK_END);
    fileContents.length = (size_t)ftell(fp);
    fileContents.data = (UA_Byte *)UA_malloc(fileContents.length * sizeof(UA_Byte));
    if (fileContents.data) {
        fseek(fp, 0, SEEK_SET);
        size_t read = fread(fileContents.data, sizeof(UA_Byte), fileContents.length, fp);
        if (read != fileContents.length) UA_ByteString_clear(&fileContents);
    } else {
        fileContents.length = 0;
    }
    fclose(fp);

    return fileContents;
}

ConstantSP connectOpcUaServer(Heap *heap, vector<ConstantSP> &arguments) {
    std::string usage =
        "Usage: connect(endPointUrl, clientUri, [username], [password], [securityMode], [securityPolicy], "
        "[certificatePath], [privateKeyPath]) ";
    UA_MessageSecurityMode securityMode = UA_MessageSecurityMode::UA_MESSAGESECURITYMODE_INVALID;
    UA_String securityPolicy = UA_STRING_ALLOC("http://opcfoundation.org/UA/SecurityPolicy#None");
    UA_ByteString certificate = UA_STRING_NULL;
    UA_ByteString privateKey = UA_STRING_NULL;

    if ((arguments[0]->getType() != DT_STRING) || !arguments[0]->isScalar()) {
        throw IllegalArgumentException(__FUNCTION__, "the endPointUrl must be string scalar.");
    }
    string serverUrl = arguments[0]->getString();
    if ((arguments[1]->getType() != DT_STRING) || !arguments[1]->isScalar()) {
        throw IllegalArgumentException(__FUNCTION__, "the clientUri must be string scalar.");
    }
    string clientUri = arguments[1]->getString();
    string username;
    string password;
    if (arguments.size() == 3) {
        throw IllegalArgumentException(__FUNCTION__, "the username and the password must be a pair");
    }
    if (arguments.size() >= 4) {
        if (!arguments[2]->isNull() && (arguments[2]->getType() != DT_STRING || !arguments[2]->isScalar())) {
            throw IllegalArgumentException(__FUNCTION__, usage + "the username must be string scalar.");
        }
        if (!arguments[3]->isNull() && (arguments[3]->getType() != DT_STRING || !arguments[3]->isScalar())) {
            throw IllegalArgumentException(__FUNCTION__, usage + "the password must be string scalar.");
        }
        if (!arguments[2]->isNull()) {
            username = arguments[2]->getString();
        }
        if (!arguments[3]->isNull()) {
            password = arguments[3]->getString();
        }
    }
    if (arguments.size() >= 5) {
        if (!arguments[4]->isNull() && (arguments[4]->getType() != DT_STRING || !arguments[4]->isScalar())) {
            throw IllegalArgumentException(
                __FUNCTION__, usage + "the securityMode must be one of None, Sign, SignAndEncrypt, default is None.");
        }
        if (!arguments[4]->isNull()) {
            string mode = arguments[4]->getString();
            if (mode == "None")
                securityMode = UA_MessageSecurityMode::UA_MESSAGESECURITYMODE_NONE;
            else if (mode == "Sign")
                securityMode = UA_MessageSecurityMode::UA_MESSAGESECURITYMODE_SIGN;
            else if (mode == "SignAndEncrypt")
                securityMode = UA_MessageSecurityMode::UA_MESSAGESECURITYMODE_SIGNANDENCRYPT;
            else
                throw IllegalArgumentException(
                    __FUNCTION__,
                    usage + "the securityMode must be one of None, Sign, SignAndEncrypt, default is None.");
        }
    }
    if (arguments.size() >= 6) {
        if (!arguments[5]->isNull() && (arguments[5]->getType() != DT_STRING || !arguments[5]->isScalar())) {
            throw IllegalArgumentException(__FUNCTION__, usage +
                                                             "the securityPolicy must be one of None, Basic256, "
                                                             "Basic128Rsa15, Basic256Sha256, default is None.");
        }
        if (!arguments[5]->isNull()) {
            string policy = arguments[5]->getString();
            if (policy == "None") {
                securityPolicy = UA_STRING_ALLOC("http://opcfoundation.org/UA/SecurityPolicy#None");
            } else if (policy == "Basic256") {
                securityPolicy = UA_STRING_ALLOC("http://opcfoundation.org/UA/SecurityPolicy#Basic256");
            } else if (policy == "Basic128Rsa15") {
                securityPolicy = UA_STRING_ALLOC("http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15");
            } else if (policy == "Basic256Sha256") {
                securityPolicy = UA_STRING_ALLOC("http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256");
            } else {
                throw IllegalArgumentException(__FUNCTION__, usage +
                                                                 "the securityPolicy must be one of None, Basic256, "
                                                                 "Basic128Rsa15, Basic256Sha256, default is None.");
            }
        }
    }
    if (arguments.size() == 7)
        throw IllegalArgumentException(__FUNCTION__, "the certificatePath and the privateKeyPath must be a pair");
    if (arguments.size() == 8) {
        if (arguments[6]->getType() != DT_STRING || !arguments[6]->isScalar()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "the certificatePath must be string scalar.");
        }
        if (arguments[7]->getType() != DT_STRING || !arguments[7]->isScalar()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "the privateKeyPath must be string scalar.");
        }
        certificate = loadFile(arguments[6]->getString());
        privateKey = loadFile(arguments[7]->getString());
    }

    OPCUAClientSP client = new OPCUAClient(heap);
    client->connect(serverUrl, clientUri, username, password, securityMode, securityPolicy, certificate, privateKey);
    return client;
}

ConstantSP disconnect(Heap *heap, vector<ConstantSP> &arguments) {
    std::ignore = heap;
    std::string usage = "Usage: close(conn) ";
    OPCUAClientSP client;
    if (arguments[0]->getType() == DT_RESOURCE && arguments[0]->getString() == OPCUA_CLIENT_DESC) {
        client = arguments[0];
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "Invalid OPCUA conn object.");
    }
    long long clientLong = (long long)client->getClientPtr();
    OPCUASubSP sub = nullptr;
    {
        LockGuard<Mutex> lock(&OPCUASub::OPCUA_SUB_MAP_LATCH);
        if (OPCUASub::OPCUA_SUB_MAP.find(clientLong) != OPCUASub::OPCUA_SUB_MAP.end()) {
            sub = OPCUASub::OPCUA_SUB_MAP[clientLong];
        }
    }
    if (!sub.isNull()) {
        {
            LockGuard<Mutex> lock(&OPCUASub::OPCUA_SUB_MAP_LATCH);
            OPCUASub::OPCUA_SUB_MAP.erase(clientLong);
        }
        sub->stopThread();
    }
    client->disconnect();

    return new Bool(true);
}

ConstantSP readNode(Heap *heap, vector<ConstantSP> &arguments) {
    std::ignore = heap;
    std::string usage = "Usage: readNode(conn, nodeNamespace, nodeIdString, [table]) ";

    vector<int> nodeNamespace;
    vector<string> nodeIdString;
    OPCUAClientSP client;
    if (arguments[0]->getType() == DT_RESOURCE && arguments[0]->getString() == OPCUA_CLIENT_DESC) {
        client = arguments[0];
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "Invalid OPCUA conn object.");
    }
    if (arguments[1]->getType() != DT_INT) {
        throw IllegalArgumentException(__FUNCTION__, usage + "the nodeNamespace must be int scalar or int array");
    }
    if (arguments[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "the nodeIdString must be string scalar or string array");
    }
    if (arguments[1]->isScalar()) {
        int nodeNamespaceValue = arguments[1]->getInt();
        if (arguments[2]->isArray()) {
            nodeNamespace.resize(arguments[2]->size());
            nodeIdString.resize(arguments[2]->size());
            for (int i = 0; i < arguments[2]->size(); i++) {
                nodeNamespace[i] = nodeNamespaceValue;
                nodeIdString[i] = arguments[2]->getString(i);
            }
        } else if (arguments[2]->isScalar()) {
            nodeNamespace.resize(1);
            nodeIdString.resize(1);
            nodeNamespace[0] = nodeNamespaceValue;
            nodeIdString[0] = arguments[2]->getString();
        }
    } else if (arguments[1]->isArray()) {
        if (!arguments[2]->isArray() || arguments[2]->size() != arguments[1]->size()) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "the nodeNamespace size and the nodeIdString size must be same");
        }
        nodeNamespace.resize(arguments[2]->size());
        nodeIdString.resize(arguments[2]->size());
        for (int i = 0; i < arguments[2]->size(); i++) {
            nodeNamespace[i] = arguments[1]->getInt(i);
            nodeIdString[i] = arguments[2]->getString(i);
        }
    } else
        throw IllegalArgumentException(__FUNCTION__, usage + "the nodeNamespace must be int scalar or int array");
    ConstantSP table;
    if (arguments.size() > 3) {
        if (!arguments[3]->isTable()) {
            if (arguments[3]->isArray()) {
                if (!arguments[2]->isArray()) {
                    throw IllegalArgumentException(__FUNCTION__, usage + "the table is a array but the nodeId not");
                } else if (arguments[2]->size() != arguments[3]->size()) {
                    throw IllegalArgumentException(__FUNCTION__,
                                                   usage + "the nodeId size and the table size must be same");
                }
                for (int i = 0; i < arguments[3]->size(); i++) {
                    if (!arguments[3]->get(i)->isTable())
                        throw IllegalArgumentException(__FUNCTION__, usage + "the table array must be all table");
                }
            } else {
                throw IllegalArgumentException(__FUNCTION__, usage + "the table must be table scalar or table array");
            }
        }
        table = arguments[3];
    } else {
        table = new Void();
    }

    return client->readNode(nodeNamespace, nodeIdString, table);
}

ConstantSP browseNode(Heap *heap, vector<ConstantSP> &arguments) {
    std::ignore = heap;
    std::string usage = "Usage: browseNode(conn) ";
    vector<int> nodeNamespace;
    vector<string> nodeIdString;
    OPCUAClientSP client;
    if (arguments[0]->getType() == DT_RESOURCE && arguments[0]->getString() == OPCUA_CLIENT_DESC) {
        client = arguments[0];
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "Invalid OPCUA conn object.");
    }
    if (!client->getConnected()) {
        throw RuntimeException(OPCUA_PREFIX + "OPCUA conn has been closed.");
    }
    return client->browseNode();
}

ConstantSP writeNode(Heap *heap, vector<ConstantSP> &arguments) {
    std::ignore = heap;
    std::string usage = "Usage: writeNode(conn, nodeNamespace, nodeIdString, value) ";

    vector<int> nodeNamespace;
    vector<string> nodeIdString;
    OPCUAClientSP client;
    if (arguments[0]->getType() == DT_RESOURCE && arguments[0]->getString() == OPCUA_CLIENT_DESC) {
        client = arguments[0];
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "Invalid OPCUA conn object.");
    }
    if (arguments[1]->getType() != DT_INT) {
        throw IllegalArgumentException(__FUNCTION__, usage + "the nodeNamespace must be int scalar or int array");
    }
    if (arguments[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "the nodeIdString must be string scalar or string array");
    }
    if (arguments[2]->isArray()) {
        if (!arguments[3]->isArray() || arguments[3]->size() != arguments[1]->size()) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "the nodeIdString size and the value size must be same");
        }
        if (arguments[1]->isScalar()) {
            int nodeNamespaceValue = arguments[1]->getInt();
            nodeNamespace.resize(arguments[2]->size());
            for (int i = 0; i < arguments[2]->size(); i++) {
                nodeNamespace[i] = nodeNamespaceValue;
            }
        } else if (!arguments[2]->isArray() || arguments[2]->size() != arguments[1]->size()) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "the nodeNamespace size and the nodeIdString size must be same");
        } else {
            nodeNamespace.resize(arguments[2]->size());
            for (int i = 0; i < arguments[2]->size(); i++) {
                nodeNamespace[i] = arguments[1]->getInt(i);
            }
        }
        nodeIdString.resize(arguments[2]->size());
        for (int i = 0; i < arguments[2]->size(); i++) {
            nodeIdString[i] = arguments[2]->getString(i);
        }
    } else if (arguments[2]->isScalar()) {
        if (!arguments[1]->isScalar()) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "the nodeIdString is a scalar but the nodeNamespace is not");
        }
        if (!arguments[3]->isScalar() && !arguments[3]->isArray() && !arguments[3]->isMatrix()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "the value must be scalar or array");
        }
        nodeNamespace.resize(1);
        nodeIdString.resize(1);
        nodeNamespace[0] = arguments[1]->getInt();
        nodeIdString[0] = arguments[2]->getString();
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "the nodeIdString must be string scalar or string array");
    }

    return client->writeNode(nodeNamespace, nodeIdString, arguments[3]);
}

ConstantSP subscribeNode(Heap *heap, vector<ConstantSP> &arguments) {
    std::ignore = heap;
    std::string usage =
        "Usage: subscribe(conn, nodeNamespace, nodeIdString, handler, [actionName], [reconnect=false], "
        "[resubscribeInterval=0]) ";
    vector<int> nodeNamespace;
    vector<string> nodeIdString;
    OPCUAClientSP client;
    if (arguments[0]->getType() == DT_RESOURCE && arguments[0]->getString() == OPCUA_CLIENT_DESC) {
        client = arguments[0];
        LockGuard<Mutex> _(&OPCUASub::OPCUA_SUB_MAP_LATCH);
        for (auto it = OPCUASub::OPCUA_SUB_MAP.begin(); it != OPCUASub::OPCUA_SUB_MAP.end(); ++it) {
            if (client->getClientPtr() == it->second->getClientPtr()) {
                throw RuntimeException(OPCUA_PREFIX + "client is already been subscribed.");
            }
        }
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "Invalid OPCUA conn object.");
    }
    if (arguments[1]->getType() != DT_INT) {
        throw IllegalArgumentException(__FUNCTION__, usage + "the nodeNamespace must be int scalar or int array");
    }
    if (arguments[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "the nodeIdString must be string scalar or string array");
    }
    if (arguments[2]->isArray()) {
        if (arguments[1]->isScalar()) {
            nodeNamespace = vector<int>(arguments[2]->size(), arguments[1]->getInt());
        } else if (!arguments[2]->isArray() || arguments[2]->size() != arguments[1]->size()) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "the nodeNamespace size and the nodeIdString size must be same");
        } else {
            nodeNamespace.resize(arguments[2]->size());
            for (int i = 0; i < arguments[2]->size(); i++) {
                nodeNamespace[i] = arguments[1]->getInt(i);
            }
        }
        nodeIdString.resize(arguments[2]->size());
        for (int i = 0; i < arguments[2]->size(); i++) {
            nodeIdString[i] = arguments[2]->getString(i);
        }
    } else if (arguments[2]->isScalar()) {
        if (!arguments[1]->isScalar()) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "the nodeIdString is a scalar but the nodeNamespace  not");
        }
        nodeNamespace.resize(1);
        nodeIdString.resize(1);
        nodeNamespace[0] = arguments[1]->getInt();
        nodeIdString[0] = arguments[2]->getString();
    } else {
        throw IllegalArgumentException(__FUNCTION__, usage + "the nodeIdString must be string scalar or string array");
    }
    if (!arguments[3]->isTable() && (arguments[3]->getType() != DT_FUNCTIONDEF)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "handler must be a table or function");
    }
    string actionName;
    if (arguments.size() > 4 && !arguments[4]->isNull()) {
        if (arguments[4]->getType() != DT_STRING || !arguments[4]->isScalar()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "actionName must be a string scalar");
        }
        actionName = arguments[4]->getString();
        LockGuard<Mutex> _(&OPCUASub::OPCUA_SUB_MAP_LATCH);
        for (auto it = OPCUASub::OPCUA_SUB_MAP.begin(); it != OPCUASub::OPCUA_SUB_MAP.end(); ++it) {
            if (actionName == it->second->getActionName()) {
                throw RuntimeException(OPCUA_PREFIX + "actionName [" + actionName + "] already exists.");
            }
        }
    }
    bool reconnect = false;
    if (arguments.size() > 5 && !arguments[5]->isNull()) {
        if (arguments[5]->getType() != DT_BOOL || !arguments[5]->isScalar()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "reconnect must be a boolean scalar");
        }
        reconnect = arguments[5]->getBool();
    }

    long long resubTimeout = 0;
    if (arguments.size() > 6 && !arguments[6]->isNull()) {
        if (arguments[6]->getCategory() != INTEGRAL || !arguments[6]->isScalar()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "resubscribeInterval must be an integer scalar");
        }
        resubTimeout = arguments[6]->getLong();
        if (resubTimeout < 0) {
            throw IllegalArgumentException(__FUNCTION__, usage + "resubscribeInterval must be a positive integer");
        }
    }
    OPCUASubSP sub =
        new OPCUASub(client->getSession()->getHeap().get(), client->getConnEndPointUrl(), client->getClientUrl(),
                     client, nodeNamespace, nodeIdString, arguments[3], reconnect, resubTimeout, actionName);
    sub->subs();
    LockGuard<Mutex> _(&OPCUASub::OPCUA_SUB_MAP_LATCH);
    OPCUASub::OPCUA_SUB_MAP.insert(std::make_pair((long long)client->getClientPtr(), sub));
    sub->getStatus().startTime_ = Util::getNanoEpochTime() + sub->getTimeGap();
    client->setSubscribeFlag();
    sub->startThread();
    return new Void();
}

ConstantSP unsubscribeNode(Heap *heap, vector<ConstantSP> &arguments) {
    std::ignore = heap;
    std::string usage = "Usage: unsubscribe(conn|actionName) ";
    ConstantSP handle = arguments[0];
    OPCUASubSP sub = nullptr;
    long long clientLong = 0;
    if (handle->getType() == DT_RESOURCE) {
        OPCUAClientSP conn;
        if (arguments[0]->getType() == DT_RESOURCE && arguments[0]->getString() == OPCUA_CLIENT_DESC) {
            conn = arguments[0];
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + "Invalid OPCUA conn object.");
        }
        {
            clientLong = (long long)conn->getClientPtr();
            if (OPCUASub::OPCUA_SUB_MAP.find(clientLong) == OPCUASub::OPCUA_SUB_MAP.end()) {
                throw RuntimeException(OPCUA_PREFIX + "No subscription on this conn.");
            }
            sub = OPCUASub::OPCUA_SUB_MAP[clientLong];
        }
        if (!sub.isNull()) {
            {
                LockGuard<Mutex> lock(&OPCUASub::OPCUA_SUB_MAP_LATCH);
                OPCUASub::OPCUA_SUB_MAP.erase(clientLong);
            }
            sub->stopThread();
        }
    } else if (handle->getType() == DT_STRING && handle->isScalar()) {
        string actionName = handle->getString();
        bool find = false;
        {
            LockGuard<Mutex> lock(&OPCUASub::OPCUA_SUB_MAP_LATCH);
            for (auto it = OPCUASub::OPCUA_SUB_MAP.begin(); it != OPCUASub::OPCUA_SUB_MAP.end(); ++it) {
                if (it->second->getActionName() == actionName) {
                    clientLong = it->first;
                    sub = OPCUASub::OPCUA_SUB_MAP[clientLong];
                    find = true;
                }
            }
        }
        if (!sub.isNull()) {
            {
                LockGuard<Mutex> lock(&OPCUASub::OPCUA_SUB_MAP_LATCH);
                OPCUASub::OPCUA_SUB_MAP.erase(clientLong);
            }
            sub->stopThread();
        }
        if (!find) {
            throw RuntimeException(OPCUA_PREFIX + "invalid actionName " + actionName);
        }
    } else if (handle->getCategory() == INTEGRAL && handle->isScalar()) {
        UA_UInt32 subId = (UA_UInt32)handle->getLong();
        bool find = false;
        {
            LockGuard<Mutex> lock(&OPCUASub::OPCUA_SUB_MAP_LATCH);
            for (auto it = OPCUASub::OPCUA_SUB_MAP.begin(); it != OPCUASub::OPCUA_SUB_MAP.end(); ++it) {
                if (it->second->getSubID() == subId) {
                    clientLong = it->first;
                    sub = OPCUASub::OPCUA_SUB_MAP[clientLong];
                    find = true;
                }
            }
        }
        if (!sub.isNull()) {
            {
                LockGuard<Mutex> lock(&OPCUASub::OPCUA_SUB_MAP_LATCH);
                OPCUASub::OPCUA_SUB_MAP.erase(clientLong);
            }
            sub->stopThread();
        }
        if (!find) {
            throw RuntimeException(OPCUA_PREFIX + "invalid subscriptionId " + std::to_string(subId));
        }
    } else {
        throw IllegalArgumentException(__FUNCTION__,
                                       usage + "first argument should be OPCUA conn or actionName.");
    }
    return new Void();
}

ConstantSP getSubscriberStat(Heap *heap, vector<ConstantSP> &arguments) {
    std::ignore = heap;
    std::ignore = arguments;
    LockGuard<Mutex> _(&OPCUASub::OPCUA_SUB_MAP_LATCH);
    int size = OPCUASub::OPCUA_SUB_MAP.size();
    ConstantSP actionNameVec = Util::createVector(DT_STRING, size);
    ConstantSP subscriptionIdVec = Util::createVector(DT_INT, size);
    ConstantSP userVec = Util::createVector(DT_STRING, size);
    ConstantSP endPointUrlVec = Util::createVector(DT_STRING, size);
    ConstantSP clientUriVec = Util::createVector(DT_STRING, size);
    ConstantSP nodeIDVec = Util::createVector(DT_STRING, size);
    ConstantSP timestampVec = Util::createVector(DT_NANOTIMESTAMP, size);
    ConstantSP isConnectedVec = Util::createVector(DT_BOOL, size);
    ConstantSP firstMsgTimeVec = Util::createVector(DT_NANOTIMESTAMP, size);
    ConstantSP lastMsgTimeVec = Util::createVector(DT_NANOTIMESTAMP, size);
    ConstantSP processedMsgCountVec = Util::createVector(DT_LONG, size);
    ConstantSP failedMsgCountVec = Util::createVector(DT_LONG, size);
    ConstantSP errorMsgVec = Util::createVector(DT_STRING, size);
    ConstantSP lastFailedTimeVec = Util::createVector(DT_NANOTIMESTAMP, size);
    ConstantSP reconnectTimeVec = Util::createVector(DT_NANOTIMESTAMP, size);

    int i = 0;
    for (auto it = OPCUASub::OPCUA_SUB_MAP.begin(); it != OPCUASub::OPCUA_SUB_MAP.end(); it++) {
        OPCUASubSP sub = it->second;
        auto &status = sub->getStatus();
        actionNameVec->setString(i, sub->getActionName());
        subscriptionIdVec->setInt(i, sub->getSubID());
        userVec->setString(i, sub->getUser());
        endPointUrlVec->setString(i, sub->getEndPoint());
        clientUriVec->setString(i, sub->getClientUri());
        nodeIDVec->setString(i, sub->getNodeID());
        timestampVec->setLong(i, status.startTime_);
        isConnectedVec->setBool(i, sub->isConnected());
        firstMsgTimeVec->setLong(i, status.firstMsgTime_);
        lastMsgTimeVec->setLong(i, status.lastMsgTime_);
        processedMsgCountVec->setLong(i, status.processedMsgCount_);
        failedMsgCountVec->setLong(i, status.failedMsgCount_);
        errorMsgVec->setString(i, status.lastErrMsg_);
        lastFailedTimeVec->setLong(i, status.lastFailedTimestamp_);
        reconnectTimeVec->setLong(i, sub->getReconnectTime());
        i++;
    }

    vector<string> colNames = {"actionName",      "subscriptionId",      "user",
                               "endpointUrl",     "clientUri",           "nodeID",
                               "createTimestamp", "isConnected",         "firstMsgTime",
                               "lastMsgTime",     "processedMsgCount",   "failedMsgCount",
                               "lastErrMsg",      "lastFailedTimestamp", "lastReconnectTime"};
    vector<ConstantSP> cols = {
        actionNameVec, subscriptionIdVec, userVec,         endPointUrlVec, clientUriVec,         nodeIDVec,
        timestampVec,  isConnectedVec,    firstMsgTimeVec, lastMsgTimeVec, processedMsgCountVec, failedMsgCountVec,
        errorMsgVec,   lastFailedTimeVec, reconnectTimeVec};
    return Util::createTable(colNames, cols);
}