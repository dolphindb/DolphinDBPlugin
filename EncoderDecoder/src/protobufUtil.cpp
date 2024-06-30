#include "protobufUtil.h"

#include <algorithm>
#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <map>
#include <string>
#include <unordered_map>

#include "CoreConcept.h"
#include "EncoderDecoder.h"
#include "Exceptions.h"
#include "Types.h"

static unsigned int FLOAT_NAN = 0x7f800000;
static unsigned long long DOUBLE_NAN = 0x7ff0000000000000;
static int ARRAY_VECTOR_TYPE_BASE = 64;

struct MsgUtilPack {
    MsgUtilPack(const vector<string> &names, const vector<DATA_TYPE> &types, const vector<ConstantSP> &dataVec,
                const unordered_map<string, DATA_TYPE> &dict)
        : names_(names), types_(types), dataVec_(dataVec), dict_(dict) {
        for (unsigned int i = 0; i < names.size(); ++i) {
            positionMap_[names[i]] = i;
        }
    }
    INDEX startIndex_;
    INDEX endIndex_;
    vector<string> names_;
    vector<DATA_TYPE> types_;
    vector<ConstantSP> dataVec_;
    unordered_map<string, DATA_TYPE> dict_;
    unordered_map<string, unsigned int> positionMap_;
    std::map<int, int> repeatDelayIndexMap_;
};

ConstantSP parseProtobufWrapper(Heap* heap, vector<ConstantSP>& arguments) {
    vector<ConstantSP> newArgs{};
    unsigned int index = 0;
    for(;index < 5; ++index) {
        newArgs.push_back(arguments[index]);
    }
    for(unsigned int i = index+1; i < arguments.size(); ++i) {
        newArgs.push_back(arguments[i]);
    }
    return parseProtobuf(heap, newArgs);
}

// integrated parse function for protobuf module
ConstantSP parseProtobuf(Heap *heap, vector<ConstantSP> &args) {
    string schemaPath = args[0]->getString();
    bool needArrayVector = args[1]->getBool();
    unordered_map<string, DATA_TYPE> columnTypeMap;
    if (!args[2]->isNull() && args[2]->getForm() == DF_TABLE) {
        ConstantSP schema = args[2];
        VectorSP name = ((TableSP)schema)->getColumn("name");
        VectorSP type = ((TableSP)schema)->getColumn(1);
        for (INDEX i = 0; i < name->size(); ++i) {
            columnTypeMap[name->getString(i)] = Util::getDataType(type->getString(i));
        }
    }
    string messageName = args[3]->getString();
    bool useZeroAsNull = args[4]->getBool();
    VectorSP vec;
    ConstantSP input = args[5];
    string errMsg = "obj must be one of: table with only  one string column, string vector, string scalar.";
    switch (input->getForm()) {
        case DF_TABLE:
            if (input->columns() != 1 || input->getColumn(0)->getCategory() != LITERAL) {
                throw RuntimeException(ENCODERDECODER_PREFIX + errMsg);
            }
            vec = input->getColumn(0);
            break;
        case DF_VECTOR:
            if (input->getType() != DT_STRING && input->getType() != DT_BLOB) {
                throw RuntimeException(ENCODERDECODER_PREFIX + errMsg);
            }
            vec = input;
            break;
        case DF_SCALAR:
            if (input->getType() != DT_STRING && input->getType() != DT_BLOB) {
                throw RuntimeException(ENCODERDECODER_PREFIX + errMsg);
            }
            vec = Util::createVector(DT_STRING, 0, 1);
            vec->append(input);
            break;
        default:
            throw RuntimeException(ENCODERDECODER_PREFIX + errMsg);
    }

    if (Util::exists(schemaPath)) {
        return parseProtobufDynamic(schemaPath, vec, columnTypeMap, needArrayVector, heap, messageName, useZeroAsNull);
    } else {
        throw RuntimeException(ENCODERDECODER_PREFIX + "Could not parse protobuf type " + schemaPath + ".");
    }
    return new Void();
}

// integrated format function for protobuf module
ConstantSP formatProtobuf(Heap *heap, vector<ConstantSP> &args) {
    string schemaName = args[0]->getString();
    int mode = 0;
    if (args[1]->getString() == "single") {
        mode = protobufCoder::SINGLE;
    } else if (args[1]->getString() == "multi") {
        mode = protobufCoder::MULTI;
    } else {
        throw RuntimeException(ENCODERDECODER_PREFIX + "serializeMode must be one of 'single' or 'multi'.");
    }
    if (args[2]->getForm() != DF_TABLE) {
        throw RuntimeException(ENCODERDECODER_PREFIX + "content data must be a table.");
    }

    return new Void();
}

// integrated getProtobufSchema function for protobuf module
ConstantSP protobufSchema(string schemaName, bool needArrayVector, const unordered_set<string> &ignoredColumn,
                          const string& messageName) {
    if (schemaName == "dolphindb") {
        throw RuntimeException(ENCODERDECODER_PREFIX + "dolphindb schema doesn't support getProtobufSchema().");
    } else {
        return getProtobufSchemaDynamic(schemaName, needArrayVector, ignoredColumn, messageName);
    }
    return new Void();
}

//========================================================================
// dynamic parse/format protobuf
//========================================================================

#include <google/protobuf/compiler/parser.h>
#include <google/protobuf/io/tokenizer.h>

#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"
#include "google/protobuf/stubs/strutil.h"
#include "google/protobuf/stubs/substitute.h"
#include "google/protobuf/text_format.h"

using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::compiler;
using MessageSP = SmartPointer<Message>;

using std::ifstream;
using std::istreambuf_iterator;

// inherit from protobuf ErrorCollector to handle error message
class ddbErrorCollector : public ErrorCollector {
  public:
    inline ddbErrorCollector() {}
    // you can adapt this to give more error info
    virtual void AddError(int line, ColumnNumber column, const std::string &message) { errorMsg_ = message; }
    string getErrorMsg() { return errorMsg_; }

  private:
    string errorMsg_;
};

// get colNames & colTypes in Descriptor, form a list of ddbVector in param 'dataVec'
int createTableFrame(const Descriptor *descriptor, vector<string> &names, vector<DATA_TYPE> &types, /*for schema*/
                     string prefix, unordered_map<string, DATA_TYPE> &dict,                         /*for getData */
                     vector<bool> &repeatStatus, std::map<string, int> &flagMap, const unordered_set<string> &ignoredColumn,
                     vector<ConstantSP> &dataVec, vector<vector<int>> &indexArrays, bool needArrayVector = false,
                     int rowSize = 0 /*use as a flag to indicate getSchema or parseDynamic*/) {
    int repeatLevel = 0;
    try {
        int fieldNum = descriptor->field_count();
        // change prefix for current createTableFrame progress
        // if it's not the very first createTableFrame progress
        //     add '_' to concatenate colNames.
        if (prefix != "") {
            prefix = prefix + "_";
        }
        vector<int> currentFieldSeq{};
        for (int i = 0; i < fieldNum; ++i) {
            int repeatCount = 0;
            const FieldDescriptor *field = descriptor->field(i);

            string currentFieldName = prefix + field->name();

            if (field->containing_oneof()) {
                flagMap["oneof"] += 1;
                if (rowSize > 0) {
                    throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported one of syntax");
                }
            } else if (field->is_extension()) {
                flagMap["extension"] += 1;
                if (rowSize > 0) {
                    throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported extension syntax");
                }
            }

            bool repeatState = false;
            if (field->is_repeated()) {
                flagMap["inRepeat"] += 1;
                if (needArrayVector) {
                    if (flagMap["inRepeat"] >= 2) {
                        repeatState = true;
                    }
                    repeatCount += 1;
                    repeatLevel = repeatLevel < repeatCount ? repeatCount : repeatLevel;
                } else {
                    if (repeatLevel > 0) {
                        if (rowSize > 0) {
                            throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported unrelated repeated fields");
                        }
                        repeatState = true;
                        flagMap["badRepeat"] = 1;
                    }
                    repeatCount += 1;
                    repeatLevel = repeatLevel < repeatCount ? repeatCount : repeatLevel;
                }
            }
            if (!needArrayVector && flagMap["inRepeat"] > 0) {
                repeatState = true;
            }

            if (field->type() != FieldDescriptor::Type::TYPE_MESSAGE) {
                names.emplace_back(currentFieldName); /* names: emplace_back here */
                repeatStatus.push_back(repeatState);
            }
            // if not message, mark Unsupported field
            if (field->type() != FieldDescriptor::Type::TYPE_MESSAGE && (flagMap["oneof"] || flagMap["extension"])) {
                types.emplace_back(DT_VOID);
                // no way to get this line when rowSize > 0 or oneof or
                // extension is true,
            } else {
                string incompErr = ENCODERDECODER_PREFIX + "schema is incompatible with proto.";
                // if not matching or can't convert, throw exceptions
                // take into account of repeat situation, concern type & dict & dataVec
                //      int:        represent normal calculation type
                //      int[]:      represent normal calculation arrayVector type
                //      string:     represent string type
                //      user-int:   represent different type of normal calculation type
                //      user-int[]:   represent different type of normal calculation arrayVector type
                // possible case:
                // realType - inputType
                // 1. int - int             :
                // 2. int - string          :
                // 3. int - int[]           :
                // 4. int - user-int        :
                // 5. int - user-int[]      :
                // 6. int[] - Int           :
                // 7. int[] - string        :
                // 8. int[] - int[]         :
                // 9. int[] - user-Int      :
                // 10.int[] - user-int[]    :
                // 11.string - Int          :
                // 12.string - int[]        :
                // 13.string - string       :
                // 14.string - user-Int     :
                // 15.string - user-int[]   :

                switch (field->type()) {
#define CREATE_TABLE_FRAME(dtType, cppType)                                \
    currentFieldSeq.emplace_back(names.size() - 1);                        \
    if (needArrayVector) {                                                 \
        if (field->is_repeated()) {                                        \
            types.emplace_back(DATA_TYPE(dtType + ARRAY_TYPE_BASE));       \
            if (rowSize > 0) {                                             \
                if (dict.find(currentFieldName) == dict.end()) {           \
                    dict[currentFieldName] = types.back();                 \
                } else {                                                   \
                    if (dict[currentFieldName] != types.back()) {          \
                        throw RuntimeException(incompErr);                 \
                    }                                                      \
                }                                                          \
                dataVec.push_back(Util::createVector(dtType, 0, rowSize)); \
                indexArrays.emplace_back(vector<int>());                   \
            }                                                              \
        } else {                                                           \
            types.emplace_back(dtType);                                    \
            if (rowSize > 0) {                                             \
                if (dict.find(currentFieldName) == dict.end()) {           \
                    dict[currentFieldName] = types.back();                 \
                }                                                          \
                dataVec.push_back(Util::createVector(dtType, 0, rowSize)); \
                indexArrays.emplace_back(vector<int>());                   \
            }                                                              \
        }                                                                  \
    } else {                                                               \
        types.emplace_back(dtType);                                        \
        if (rowSize > 0) {                                                 \
            dataVec.push_back(Util::createVector(dtType, 0, rowSize));     \
        }                                                                  \
    }                                                                      \
    break;
                    case FieldDescriptor::Type::TYPE_INT32:
                    case FieldDescriptor::Type::TYPE_SINT32:
                    case FieldDescriptor::Type::TYPE_SFIXED32:
                        CREATE_TABLE_FRAME(DT_INT, int)
                    case FieldDescriptor::Type::TYPE_INT64:
                    case FieldDescriptor::Type::TYPE_SINT64:
                    case FieldDescriptor::Type::TYPE_FIXED32:
                    case FieldDescriptor::Type::TYPE_FIXED64:
                    case FieldDescriptor::Type::TYPE_SFIXED64:
                    case FieldDescriptor::Type::TYPE_UINT32:
                    case FieldDescriptor::Type::TYPE_UINT64:
                        CREATE_TABLE_FRAME(DT_LONG, long long)
                    case FieldDescriptor::Type::TYPE_FLOAT:
                        CREATE_TABLE_FRAME(DT_FLOAT, float)
                    case FieldDescriptor::Type::TYPE_DOUBLE:
                        CREATE_TABLE_FRAME(DT_DOUBLE, double)
                    case FieldDescriptor::Type::TYPE_BOOL:
                        CREATE_TABLE_FRAME(DT_BOOL, char)
                    case FieldDescriptor::Type::TYPE_STRING:
                        currentFieldSeq.emplace_back(names.size() - 1);
                        if (needArrayVector) {
                            if (field->is_repeated()) {
                                types.emplace_back(DT_BLOB);
                                if (rowSize > 0) {
                                    if (dict.find(currentFieldName) == dict.end()) {
                                        dict[currentFieldName] = DT_BLOB;
                                    } /* b-1-1 dict: user-type, with dataVec */
                                    if (dict[currentFieldName] == DT_BLOB || dict[currentFieldName] == DT_STRING) {
                                        dataVec.push_back(
                                            Util::createVector(DT_STRING, 0, rowSize)); /* b-1-1 dataVec: BLOB */
                                        dict[currentFieldName] =
                                            DATA_TYPE(DT_BLOB + ARRAY_TYPE_BASE); /* b-1-1 dict: BLOB[], with dataVec */
                                    } else {
                                        switch (int(dict[currentFieldName]) - ARRAY_TYPE_BASE) {
                                            case DT_INT:
                                            case DT_LONG:
                                            case DT_BOOL:
                                            case DT_FLOAT:
                                            case DT_DOUBLE:
                                                dataVec.push_back(Util::createVector(
                                                    DATA_TYPE(int(dict[currentFieldName]) - ARRAY_TYPE_BASE), 0,
                                                    rowSize)); /* b-1-2 dataVec: user-type */
                                                break;
                                            default: /* b-1-3 no dataVec, safe to throw*/
                                                throw RuntimeException(ENCODERDECODER_PREFIX +
                                                                       "invalid transform type for col " +
                                                                       currentFieldName);
                                        }
                                    }
                                    indexArrays.emplace_back(vector<int>());
                                }
                            } else {
                                types.emplace_back(DT_STRING);
                                if (rowSize > 0) {
                                    if (dict.find(currentFieldName) ==
                                        dict.end()) { /* b-2-1 dict: string, with dataVec */
                                            dict[currentFieldName] = DT_STRING;
                                    } /* b-2-2 dict: use type, with dataVec */
                                    dataVec.push_back(
                                        Util::createVector(DT_STRING, 0, rowSize)); /* b-2 dict: dataVec: string */
                                    indexArrays.emplace_back(vector<int>());
                                }
                            }
                        } else {
                            types.emplace_back(DT_STRING);
                            if (rowSize > 0) {
                                dataVec.push_back(Util::createVector(DT_STRING, 0, rowSize));
                            }
                        }
                        break;
                    case FieldDescriptor::Type::TYPE_BYTES:
                        CREATE_TABLE_FRAME(DT_BLOB, string)
                    case FieldDescriptor::Type::TYPE_MESSAGE: {
                        // when meeting Message type fields
                        // start another createTableFrame function to get colNames & colTypes
                        if (needArrayVector) {
                            int ret;
                            if (rowSize > 0) {
                                ret = createTableFrame(field->message_type(), names, types, prefix + field->name(),
                                                       dict, repeatStatus, flagMap, ignoredColumn, dataVec, indexArrays,
                                                       true, rowSize);
                            } else {
                                dataVec = {};
                                indexArrays = {};
                                ret =
                                    createTableFrame(field->message_type(), names, types, prefix + field->name(), dict,
                                                     repeatStatus, flagMap, ignoredColumn, dataVec, indexArrays, true);
                            }

                            repeatCount += ret;
                            if (repeatCount >= 2 && repeatLevel >= 2) {
                                flagMap["badRepeat"] = 1;
                            }
                            repeatLevel = repeatLevel < repeatCount ? repeatCount : repeatLevel;
                        } else {
                            int ret;
                            if (rowSize > 0) {
                                indexArrays = {};
                                ret = createTableFrame(field->message_type(), names, types, prefix + field->name(),
                                                       dict, repeatStatus, flagMap, ignoredColumn, dataVec, indexArrays,
                                                       false, rowSize);
                            } else {
                                dataVec = {};
                                indexArrays = {};
                                ret =
                                    createTableFrame(field->message_type(), names, types, prefix + field->name(), dict,
                                                     repeatStatus, flagMap, ignoredColumn, dataVec, indexArrays);
                            }
                            repeatCount += ret;
                            if (repeatCount > 0) {
                                if (repeatLevel > 0 && !field->is_repeated()) {
                                    flagMap["badRepeat"] = 1;
                                }
                            }
                            repeatLevel = repeatLevel < repeatCount ? repeatCount : repeatLevel;
                        }
                        break;
                    }
#undef CREATE_TABLE_FRAME
                    default:
                        types.emplace_back(DT_VOID);
                        if (rowSize > 0) {
                            dict[currentFieldName] = DT_VOID;
                            throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported type " +
                                                   string(FieldDescriptor::TypeName(field->type())) + ".");
                        }
                }
            }

            if (field->containing_oneof()) {
                flagMap["oneof"] -= 1;
            } else if (field->is_extension()) {
                flagMap["extension"] -= 1;
            }
            if (field->is_repeated()) {
                flagMap["inRepeat"] -= 1;
            }
        }
        // if it's the bottom of a structure, while it's in repeat env, make it arrayVector
        if (needArrayVector && flagMap["inRepeat"] > 0 && !repeatLevel) {
            for (int &seq : currentFieldSeq) {
                if (types[seq] == DT_STRING) {
                    types[seq] = DT_BLOB;
                    if (rowSize > 0) {
                        indexArrays[seq] = vector<int>();

                        if (dict[names[seq]] == DT_BLOB || dict[names[seq]] == DT_STRING) {
                            dataVec[seq] = Util::createVector(DT_STRING, 0, rowSize);
                            dict[names[seq]] =
                                DATA_TYPE(-1 * (DT_BLOB + ARRAY_TYPE_BASE)); /* b-2-1 dict: string[], with dataVec */
                        } else {
                            switch (int(dict[names[seq]]) - ARRAY_TYPE_BASE) { /* b-2-2 dict: use-type, with dataVec */
                                case DT_INT:
                                    dataVec[seq] = Util::createVector(DT_INT, 0, rowSize);
                                    break;
                                case DT_LONG:
                                    dataVec[seq] = Util::createVector(DT_LONG, 0, rowSize);
                                    break;
                                case DT_BOOL:
                                    dataVec[seq] = Util::createVector(DT_CHAR, 0, rowSize);
                                    break;
                                case DT_FLOAT:
                                    dataVec[seq] = Util::createVector(DT_FLOAT, 0, rowSize);
                                    break;
                                case DT_DOUBLE:
                                    dataVec[seq] = Util::createVector(DT_DOUBLE, 0, rowSize);
                                    break;
                                default:
                                    throw RuntimeException(ENCODERDECODER_PREFIX + "invalid transform type for col " +
                                                           names[seq]);
                            }
                            dict[names[seq]] = DATA_TYPE(-1 * int(dict[names[seq]]));
                        }
                    }
                } else {
                    types[seq] = DATA_TYPE(types[seq] + ARRAY_TYPE_BASE);
                    if (rowSize > 0) {
                        dict[names[seq]] = DATA_TYPE(types[seq]);
                        switch (types[seq] - ARRAY_TYPE_BASE) {
                            case DT_INT:
                            case DT_LONG:
                            case DT_BOOL:
                            case DT_FLOAT:
                            case DT_DOUBLE:
                                break;
                            default:
                                // TODO change exception error string
                                throw RuntimeException(ENCODERDECODER_PREFIX + "Invalid repeat type. " +
                                                       Util::getDataTypeString(types[seq]));
                        }
                        indexArrays[seq] = vector<int>();
                        dict[names[seq]] = DATA_TYPE(-1 * int(dict[names[seq]]));
                    }
                }
            }
        }

    } catch (std::exception &e) {
        string extraError = "";
        if (!needArrayVector) {
            if (dict.size() == 0) {
                for (unsigned int i = 0; i < names.size(); ++i) {
                    dict[names[i]] = types[i];
                }
            } else {
                bool diff = false;
                for (unsigned int i = 0; i < names.size(); ++i) {
                    if (dict[names[i]] != types[i]) {
                        diff = true;
                        dict[names[i]] = types[i];
                    }
                }
                if (diff) {
                    extraError = ". Schema is incompatible with proto.";
                }
            }
        }
        // Defer df([&](){destroyResource(names, dict, dataVec);});
        string errMsg = string(e.what() + extraError);
        if (errMsg.find(ENCODERDECODER_PREFIX) == errMsg.npos) {
            errMsg = ENCODERDECODER_PREFIX + errMsg;
        }
        throw RuntimeException(errMsg);
    }
    return repeatLevel;
}

ConstantSP getProtobufSchemaDynamic(string schemaPath, bool needArrayVector,
                                    const unordered_set<string> &ignoredColumn, const string& protoName) {
    try {
        std::ifstream in;
        in.open(schemaPath);
        std::string protobufStr((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        const char *text = protobufStr.c_str();

        ArrayInputStream rawInput(text, protobufStr.size());
        ddbErrorCollector errorCollector;
        Tokenizer input(&rawInput, &errorCollector);

        FileDescriptorProto fileDescProto;
        google::protobuf::compiler::Parser parser;
        if (!parser.Parse(&input, &fileDescProto)) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "Failed to parse .proto definition " + schemaPath + " : " +
                                   errorCollector.getErrorMsg());
        }

        if (!fileDescProto.has_name()) {
            fileDescProto.set_name(schemaPath);
        }

        DescriptorPool pool;
        const FileDescriptor *fileDesc = pool.BuildFile(fileDescProto);
        if (fileDesc == NULL) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "Cannot get file descriptor from file descriptor proto" +
                                   fileDescProto.DebugString());
        }

        const Descriptor *messageDesc;
        if (protoName != "") {
            messageDesc = fileDesc->FindMessageTypeByName(protoName);
        } else {
            if (fileDesc->message_type_count() <= 0) {
                throw RuntimeException(ENCODERDECODER_PREFIX + "no message type in file [" + schemaPath + "]");
            }
            messageDesc = fileDesc->message_type(0);
        }

        if (messageDesc == NULL) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "Cannot get message descriptor of message: " + schemaPath +
                                   ", DebugString(): " + fileDesc->DebugString());
        }

        DynamicMessageFactory factory;
        const Message *prototypeMsg = factory.GetPrototype(messageDesc);
        if (prototypeMsg == NULL) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "Cannot create prototype message from message descriptor");
        }

        vector<string> names;
        vector<DATA_TYPE> types;
        vector<bool> repeatStatus;
        std::map<string, int> flagMap;

        const Descriptor *msgDescriptor = prototypeMsg->GetDescriptor();
        if (msgDescriptor == NULL) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "null des.");
        }

        auto dict = unordered_map<string, DATA_TYPE>();
        vector<ConstantSP> vec = {};
        vector<vector<int>> indexVec = {};
        createTableFrame(msgDescriptor, names, types, "", dict, repeatStatus, flagMap, ignoredColumn, vec, indexVec,
                         needArrayVector);

        if (flagMap.find("badRepeat") != flagMap.end()) {
            for (unsigned int i = 0; i < repeatStatus.size(); ++i) {
                if (repeatStatus[i]) types[i] = DT_VOID;
            }
        }

        std::map<string, int> nameMap;
        for (unsigned int i = 0; i < names.size(); ++i) {
            if (nameMap.find(names[i]) != nameMap.end()) {
                types[i] = DT_VOID;
                types[nameMap[names[i]]] = DT_VOID;
            }
            nameMap[names[i]] = i;
        }
        vector<string> schemaName = {"name", "typeString", "typeInt"};
        VectorSP colNames = Util::createVector(DT_STRING, 0, types.size());
        VectorSP colTypes = Util::createVector(DT_STRING, 0, types.size());
        VectorSP colTypeInt = Util::createVector(DT_INT, 0, types.size());
        vector<string> colTypesString;
        vector<int> colTypesInt;

        // if bad repeat, change corresponding type to Unsupported
        for (auto &type : types) {
            if (type == DT_VOID) {
                colTypesString.emplace_back("Unsupported");
                colTypesInt.push_back(-1);
            } else {
                colTypesString.emplace_back(Util::getDataTypeString(type));
                colTypesInt.push_back(type);
            }
        }
        colNames->appendString(names.data(), names.size());
        colTypes->appendString(colTypesString.data(), colTypesString.size());
        colTypeInt->appendInt(colTypesInt.data(), colTypesInt.size());
        vector<ConstantSP> cols{colNames, colTypes, colTypeInt};
        return Util::createTable(schemaName, cols);

    } catch (std::exception &e) {
        string errMsg = string(e.what());
        if (errMsg.find(ENCODERDECODER_PREFIX) == errMsg.npos) {
            errMsg = ENCODERDECODER_PREFIX + errMsg;
        }
        throw RuntimeException(errMsg);
    }
}

void appendNull(DATA_TYPE type, int index, vector<ConstantSP> &dataVec, bool useZeroAsNull) {
    switch (type) {
        case DT_BOOL: {
            char value = useZeroAsNull ? 0 : CHAR_MIN;
            ((VectorSP)dataVec[index])->appendBool(&value, 1);
        } break;
        case DT_INT: {
            int value = useZeroAsNull ? 0 : INT_MIN;
            ((VectorSP)dataVec[index])->appendInt(&value, 1);
        } break;
        case DT_LONG: {
            long long value = useZeroAsNull ? 0 : LONG_LONG_MIN;
            ((VectorSP)dataVec[index])->appendLong(&value, 1);
        } break;
        case DT_FLOAT: {
            float value = useZeroAsNull ? 0 : FLT_NMIN;
            ((VectorSP)dataVec[index])->appendFloat(&value, 1);
        } break;
        case DT_DOUBLE: {
            double value = useZeroAsNull ? 0 : DBL_NMIN;
            ((VectorSP)dataVec[index])->appendDouble(&value, 1);
        } break;
        case DT_STRING:
        case DT_SYMBOL:
        case DT_BLOB: {
            string value = "";
            ((VectorSP)dataVec[index])->appendString(&value, 1);
        } break;
        default:
            throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported data type " + Util::getDataTypeString(type));
    }
}

void appendNull(DATA_TYPE rawType, int index, vector<ConstantSP> &dataVec, vector<vector<int>> &indexArrays,
                std::map<int, int> &repeatDelayIndexMap, bool useZeroAsNull) {
    DATA_TYPE type = DATA_TYPE(abs(int(rawType)));
    if ((int)type < ARRAY_VECTOR_TYPE_BASE) {
        switch (type) {
            case DT_BOOL: {
                char value = useZeroAsNull ? 0 : CHAR_MIN;
                ((VectorSP)dataVec[index])->appendBool(&value, 1);
            } break;
            case DT_INT: {
                int value = useZeroAsNull ? 0 : INT_MIN;
                ((VectorSP)dataVec[index])->appendInt(&value, 1);
            } break;
            case DT_LONG: {
                long long value = useZeroAsNull ? 0 : LONG_LONG_MIN;
                ((VectorSP)dataVec[index])->appendLong(&value, 1);
            } break;
            case DT_FLOAT: {
                float value = useZeroAsNull ? 0 : FLT_NMIN;
                ((VectorSP)dataVec[index])->appendFloat(&value, 1);
            } break;
            case DT_DOUBLE: {
                double value = useZeroAsNull ? 0 : DBL_NMIN;
                ((VectorSP)dataVec[index])->appendDouble(&value, 1);
            } break;
            case DT_STRING:
            case DT_SYMBOL:
            case DT_BLOB: {
                string value = "";
                ((VectorSP)dataVec[index])->appendString(&value, 1);
            } break;
            default:
                throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported data type " + Util::getDataTypeString(type));
        }
    } else {
        vector<int> &indexArray = indexArrays[index];
        if (int(rawType) >= 0) {
            if (indexArray.size() != 0) {
                indexArray.push_back(indexArray.back() + 1);
            } else {
                indexArray.push_back(1);
            }
        } else {
            repeatDelayIndexMap[index] += 1;
        }
        switch (type - ARRAY_VECTOR_TYPE_BASE) {
            case DT_BOOL: {
                char value = useZeroAsNull ? 0 : CHAR_MIN;
                ((VectorSP)dataVec[index])->appendBool(&value, 1);
            } break;
            case DT_INT: {
                int value = useZeroAsNull ? 0 : INT_MIN;
                ((VectorSP)dataVec[index])->appendInt(&value, 1);
            } break;
            case DT_LONG: {
                long long value = useZeroAsNull ? 0 : LONG_LONG_MIN;
                ((VectorSP)dataVec[index])->appendLong(&value, 1);
            } break;
            case DT_FLOAT: {
                float value = useZeroAsNull ? 0 : FLT_NMIN;
                ((VectorSP)dataVec[index])->appendFloat(&value, 1);
            } break;
            case DT_DOUBLE: {
                double value = useZeroAsNull ? 0 : DBL_NMIN;
                ((VectorSP)dataVec[index])->appendDouble(&value, 1);
            } break;
            // case DT_STRING:
            // case DT_SYMBOL:
            case DT_BLOB: {
                string value = "";
                ((VectorSP)dataVec[index])->appendString(&value, 1);
            } break;
            default:
                throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported data type " +
                                       Util::getDataTypeString((DATA_TYPE)(type - ARRAY_VECTOR_TYPE_BASE)));
        }
    }
}
void appendMsgNull(const Descriptor *field, MsgUtilPack &pack, string prefix, bool useZeroAsNull) {
    int fieldNum = field->field_count();
    vector<ConstantSP> &dataVec = pack.dataVec_;
    if (prefix != "") {
        prefix = prefix + "_";
    }
    for (int i = 0; i < fieldNum; ++i) {
        const FieldDescriptor *inField = field->field(i);
        string fieldName = prefix + inField->name();
        INDEX fieldIndex = pack.positionMap_[fieldName];
        if (inField->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
            appendMsgNull(inField->message_type(), pack, prefix, useZeroAsNull);
        } else {
            appendNull(pack.types_[fieldIndex], fieldIndex, dataVec, useZeroAsNull);
        }
    }
}

void appendMsgNull(const Descriptor *field, MsgUtilPack &pack, vector<vector<int>> &indexArrays, string prefix,
                   bool useZeroAsNull) {
    int fieldNum = field->field_count();
    vector<ConstantSP> &dataVec = pack.dataVec_;
    if (prefix != "") {
        prefix = prefix + "_";
    }
    for (int i = 0; i < fieldNum; ++i) {
        const FieldDescriptor *inField = field->field(i);
        string fieldName = prefix + inField->name();
        INDEX fieldIndex = pack.positionMap_[fieldName];
        if (inField->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
            appendMsgNull(inField->message_type(), pack, indexArrays, prefix, useZeroAsNull);
        } else {
            appendNull(pack.dict_[pack.names_[fieldIndex]], fieldIndex, dataVec, indexArrays, pack.repeatDelayIndexMap_,
                       useZeroAsNull);
        }
    }
}

void castAndPushBack(const string &str, DATA_TYPE type, vector<ConstantSP> &dataVec, int seq) {
    switch (type - ARRAY_TYPE_BASE) {
        case DT_INT:
            int intValue;
            try {
                intValue = std::stoi(str);
            } catch (...) {
                intValue = INT_MIN;
            }
            ((VectorSP)dataVec[seq])->appendInt(&intValue, 1);
            break;
        case DT_LONG:
            long long longValue;
            try {
                longValue = std::stoll(str);
            } catch (...) {
                longValue = INT_MIN;
            }
            ((VectorSP)dataVec[seq])->appendLong(&longValue, 1);
            break;
        case DT_FLOAT:
            float floatValue;
            try {
                floatValue = std::stof(str);
            } catch (...) {
                floatValue = FLT_NMIN;
            }
            ((VectorSP)dataVec[seq])->appendFloat(&floatValue, 1);
            break;
        case DT_DOUBLE:
            double doubleValue;
            try {
                doubleValue = std::stod(str);
            } catch (...) {
                doubleValue = DBL_NMIN;
            }
            ((VectorSP)dataVec[seq])->appendDouble(&doubleValue, 1);
            break;
        case DT_BLOB:
            ((VectorSP)dataVec[seq])->appendString(&str, 1);
            break;
        default:
            throw RuntimeException(ENCODERDECODER_PREFIX +
                                   "assign wrong data type for arrayVector column:" + Util::getDataTypeString(type));
    }
}

template <typename T>
inline T convertProtoToDolphinDB(T &val) {
    if (std::is_same<T, uint64_t>::value) {
        if (val > 0 && val > (uint64_t)LLONG_MAX) {
            return LLONG_MAX;
        }
    } else if (std::is_same<T, float>::value) {
        unsigned int num = *reinterpret_cast<unsigned int *>(&val);
        if ((num & FLOAT_NAN) == FLOAT_NAN) {
            *reinterpret_cast<unsigned int *>(&val) = 0xff7fffff;
        }
    } else if (std::is_same<T, double>::value) {
        unsigned long long num = *reinterpret_cast<unsigned long long *>(&val);
        if ((num & DOUBLE_NAN) == DOUBLE_NAN) {
            *reinterpret_cast<unsigned long long *>(&val) = 0xffefffffffffffff;
        }
    } else if (std::is_same<T, bool>::value) {
        if (val) {
            return 1;
        } else {
            return 0;
        }
    }
    return val;
}

template <>
inline string convertProtoToDolphinDB(string &val) {
    return val;
}

void setIndex(vector<vector<int>> &indexArrays, INDEX index, int size) {
    vector<int> &indexArray = indexArrays[index];
    if (indexArray.size() != 0) {
        indexArray.push_back(indexArray.back() + size);
    } else {
        indexArray.push_back(size);
    }
}

void getMsgDataWithArrayVector(const Message &msg, MsgUtilPack &pack, vector<vector<int>> &indexArrays, string prefix,
                               string &repeatDelayField, bool useZeroAsNull) {
    //TODO check length of all use of pack content.
    vector<string> &names = pack.names_;
    vector<DATA_TYPE> &types = pack.types_;
    vector<ConstantSP> &dataVec = pack.dataVec_;
    unordered_map<string, DATA_TYPE> &dict = pack.dict_;
    unordered_map<string, unsigned int> &positionMap = pack.positionMap_;
    std::map<int, int> &repeatDelayIndexMap = pack.repeatDelayIndexMap_;
    if (prefix != "") {
        prefix = prefix + "_";
    }

    const Reflection *ref = msg.GetReflection();
    const Descriptor *msgDesc = msg.GetDescriptor();
    int fieldNum = msgDesc->field_count();
    INDEX startIndex = -1;
    INDEX endIndex = -1;
    for (int i = 0; i < fieldNum; ++i) {
        const FieldDescriptor *field = msgDesc->field(i);
        string fieldName = prefix + field->name();
        if (field->type() != FieldDescriptor::TYPE_MESSAGE && dict.find(fieldName) == dict.end()) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "Unknown field " + fieldName + ".");
        }
        INDEX fieldIndex = positionMap[fieldName];
        DATA_TYPE rawType = dict[fieldName];
        DATA_TYPE currentFieldType = DATA_TYPE(abs(rawType));

        if (field->type() != FieldDescriptor::TYPE_MESSAGE) {
            if (startIndex == -1) {
                startIndex = fieldIndex;
            };
            if (endIndex == -1) {
                endIndex = fieldIndex;
            };
            startIndex = fieldIndex < startIndex ? fieldIndex : startIndex;
            endIndex = fieldIndex > endIndex ? fieldIndex : endIndex;
        }
        if (field->is_repeated()) {
            int size = ref->FieldSize(msg, field);
            switch (field->type()) {
                case FieldDescriptor::Type::TYPE_INT32:
                case FieldDescriptor::Type::TYPE_SFIXED32:
                case FieldDescriptor::Type::TYPE_SINT32:
                    if (size == 0) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    for (int j = 0; j < size; ++j) {
                        int ele = ref->GetRepeatedInt32(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendInt(&ele, 1);
                    }
                    setIndex(indexArrays, fieldIndex, size);
                    break;
                case FieldDescriptor::Type::TYPE_FIXED32:  // unsigned int
                case FieldDescriptor::Type::TYPE_UINT32:   // unsigned int
                    if (size == 0) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    for (int j = 0; j < size; ++j) {
                        long long ele = ref->GetRepeatedUInt32(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendLong(&ele, 1);
                    }
                    setIndex(indexArrays, fieldIndex, size);
                    break;
                case FieldDescriptor::Type::TYPE_INT64:
                case FieldDescriptor::Type::TYPE_SINT64:
                case FieldDescriptor::Type::TYPE_SFIXED64:
                    if (size == 0) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    for (int j = 0; j < size; ++j) {
                        long long ele = ref->GetRepeatedInt64(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendLong(&ele, 1);
                    }
                    setIndex(indexArrays, fieldIndex, size);
                    break;
                case FieldDescriptor::Type::TYPE_FIXED64:
                case FieldDescriptor::Type::TYPE_UINT64:
                    if (size == 0) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    for (int j = 0; j < size; ++j) {
                        unsigned long long ele = ref->GetRepeatedUInt64(msg, field, j);
                        if(ele > LONG_LONG_MAX) {
                            ele = LONG_LONG_MAX;
                        }
                        long long value = ele;
                        ((VectorSP)dataVec[fieldIndex])->appendLong(&value, 1);
                    }
                    setIndex(indexArrays, fieldIndex, size);
                    break;
                case FieldDescriptor::Type::TYPE_FLOAT:
                    if (size == 0) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    for (int j = 0; j < size; ++j) {
                        float ele = ref->GetRepeatedFloat(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendFloat(&ele, 1);
                    }
                    setIndex(indexArrays, fieldIndex, size);
                    break;
                case FieldDescriptor::Type::TYPE_DOUBLE:
                    if (size == 0) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    for (int j = 0; j < size; ++j) {
                        double ele = ref->GetRepeatedDouble(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendDouble(&ele, 1);
                    }
                    setIndex(indexArrays, fieldIndex, size);
                    break;
                case FieldDescriptor::Type::TYPE_BOOL:
                    if (size == 0) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    for (int j = 0; j < size; ++j) {
                        char ele = ref->GetRepeatedBool(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendChar(&ele, 1);
                    }
                    setIndex(indexArrays, fieldIndex, size);
                    break;
                case FieldDescriptor::Type::TYPE_STRING:
                case FieldDescriptor::Type::TYPE_BYTES:
                    if (size == 0) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    if (int(currentFieldType) > int(ARRAY_TYPE_BASE)) {
                        for (int j = 0; j < size; ++j) {
                            string ele = ref->GetRepeatedString(msg, field, j);
                            castAndPushBack(ele, currentFieldType, dataVec, fieldIndex);
                        }
                        setIndex(indexArrays, fieldIndex, size);
                    } else {
                        for (int j = 0; j < size; ++j) {
                            string ele = ref->GetRepeatedString(msg, field, j);
                            ((VectorSP)dataVec[fieldIndex])->appendString(&ele, 1);
                        }
                    }
                    break;
                case FieldDescriptor::Type::TYPE_MESSAGE:
                    if (size == 0) {
                        appendMsgNull(field->message_type(), pack, indexArrays, fieldName, useZeroAsNull);
                        break;
                    }
                    for (int j = 0; j < size; ++j) {
                        const Message &fieldMsg = ref->GetRepeatedMessage(msg, field, j);
                        getMsgDataWithArrayVector(fieldMsg, pack, indexArrays, fieldName, repeatDelayField,
                                                  useZeroAsNull);
                        if (startIndex == -1) {
                            startIndex = pack.startIndex_;
                        };
                        if (endIndex == -1) {
                            endIndex = pack.endIndex_;
                        };
                        startIndex = pack.startIndex_ < startIndex ? pack.startIndex_ : startIndex;
                        endIndex = pack.endIndex_ > endIndex ? pack.endIndex_ : endIndex;
                    }
                    // the most outside struct must not be repeat
                    // so in one loop, all delayed repeated field could be finished
                    if (repeatDelayIndexMap.size() > 0) {
                        for (auto it = repeatDelayIndexMap.begin(); it != repeatDelayIndexMap.end(); ++it) {
                            vector<int> &indexArray = indexArrays[it->first];
                            indexArray.push_back(it->second);
                        }
                        repeatDelayField = "";
                        repeatDelayIndexMap.clear();
                    }
                    break;
                default:
                    throw RuntimeException(ENCODERDECODER_PREFIX + "Invalid type.");
            }
        } else {
            bool hasField = ref->HasField(msg, field);
            switch (field->type()) {
                case FieldDescriptor::Type::TYPE_INT32:
                case FieldDescriptor::Type::TYPE_SFIXED32:
                case FieldDescriptor::Type::TYPE_SINT32: {
                    if (!hasField) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    int ele = ref->GetInt32(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendInt(&ele, 1);
                    if (int(currentFieldType) > int(ARRAY_TYPE_BASE)) {
                        repeatDelayIndexMap[fieldIndex] = dataVec[fieldIndex]->size();
                    }
                } break;
                case FieldDescriptor::Type::TYPE_FIXED32:   // unsigned int
                case FieldDescriptor::Type::TYPE_UINT32: {  // unsigned int
                    if (!hasField) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    long long ele = ref->GetUInt32(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendLong(&ele, 1);
                    if (int(currentFieldType) > int(ARRAY_TYPE_BASE)) {
                        repeatDelayIndexMap[fieldIndex] = dataVec[fieldIndex]->size();
                    }
                } break;
                case FieldDescriptor::Type::TYPE_INT64:
                case FieldDescriptor::Type::TYPE_SINT64:
                case FieldDescriptor::Type::TYPE_SFIXED64: {
                    if (!hasField) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    long long ele = ref->GetInt64(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendLong(&ele, 1);
                    if (int(currentFieldType) > int(ARRAY_TYPE_BASE)) {
                        repeatDelayIndexMap[fieldIndex] = dataVec[fieldIndex]->size();
                    }
                } break;
                case FieldDescriptor::Type::TYPE_FIXED64:
                case FieldDescriptor::Type::TYPE_UINT64: {
                    if (!hasField) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    unsigned long long ele = ref->GetUInt64(msg, field);
                    if(ele > LONG_LONG_MAX) {
                        ele = LONG_LONG_MAX;
                    }
                    long long value = ele;
                    ((VectorSP)dataVec[fieldIndex])->appendLong(&value, 1);
                    if (int(currentFieldType) > int(ARRAY_TYPE_BASE)) {
                        repeatDelayIndexMap[fieldIndex] = dataVec[fieldIndex]->size();
                    }
                } break;
                case FieldDescriptor::Type::TYPE_FLOAT: {
                    if (!hasField) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    float ele = ref->GetFloat(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendFloat(&ele, 1);
                    if (int(currentFieldType) > int(ARRAY_TYPE_BASE)) {
                        repeatDelayIndexMap[fieldIndex] = dataVec[fieldIndex]->size();
                    }
                } break;
                case FieldDescriptor::Type::TYPE_DOUBLE: {
                    if (!hasField) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    double ele = ref->GetDouble(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendDouble(&ele, 1);
                    if (int(currentFieldType) > int(ARRAY_TYPE_BASE)) {
                        repeatDelayIndexMap[fieldIndex] = dataVec[fieldIndex]->size();
                    }
                } break;
                case FieldDescriptor::Type::TYPE_BOOL: {
                    if (!hasField) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    char ele = ref->GetBool(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendChar(&ele, 1);
                    if (int(currentFieldType) > int(ARRAY_TYPE_BASE)) {
                        repeatDelayIndexMap[fieldIndex] = dataVec[fieldIndex]->size();
                    }
                } break;
                case FieldDescriptor::Type::TYPE_STRING:
                case FieldDescriptor::Type::TYPE_BYTES: {
                    if (!hasField) {
                        appendNull(rawType, fieldIndex, dataVec, indexArrays, repeatDelayIndexMap, useZeroAsNull);
                        break;
                    }
                    string ele = ref->GetString(msg, field);
                    if (int(currentFieldType) > int(ARRAY_TYPE_BASE)) {
                        castAndPushBack(ele, currentFieldType, dataVec, fieldIndex);
                        repeatDelayIndexMap[fieldIndex] = dataVec[fieldIndex]->size();
                    } else {
                        ((VectorSP)dataVec[fieldIndex])->appendString(&ele, 1);
                    }
                } break;
                case FieldDescriptor::Type::TYPE_MESSAGE: {
                    if (!hasField) {
                        appendMsgNull(field->message_type(), pack, indexArrays, fieldName, useZeroAsNull);
                        break;
                    }
                    const Message &fieldMsg = ref->GetMessage(msg, field);
                    getMsgDataWithArrayVector(fieldMsg, pack, indexArrays, fieldName, repeatDelayField, useZeroAsNull);

                    if (startIndex == -1) {
                        startIndex = pack.startIndex_;
                    };
                    if (endIndex == -1) {
                        endIndex = pack.endIndex_;
                    };
                    startIndex = pack.startIndex_ < startIndex ? pack.startIndex_ : startIndex;
                    endIndex = pack.endIndex_ > endIndex ? pack.endIndex_ : endIndex;
                } break;
                default:
                    throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported type " +
                                           string(FieldDescriptor::TypeName(field->type())) + ".");
            }
        }
    }
    pack.startIndex_ = startIndex;
    pack.endIndex_ = endIndex;

    // if in an repeat delay struct, skip margin fill.
    if (repeatDelayIndexMap.size() > 0) {
        return;
    }
    // find max ddbVector length of current msg range
    int maxSize = 0;
    for (int i = startIndex; i <= endIndex; ++i) {
        DATA_TYPE type = DATA_TYPE(abs(int(dict[names[i]])));
        if (int(type) < ARRAY_TYPE_BASE) {
            switch (type) {
                case DT_INT:
                case DT_LONG:
                case DT_BOOL:
                case DT_FLOAT:
                case DT_DOUBLE:
                case DT_BLOB:
                case DT_SYMBOL:
                case DT_STRING:
                    maxSize = maxSize < dataVec[i]->size() ? dataVec[i]->size() : maxSize;
                    break;
                default:
                    throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported type " +
                                           Util::getDataTypeString(types[i]) + ". " + Util::getDataTypeString(type));
            }
        } else {
            maxSize = maxSize < int(indexArrays[i].size()) ? indexArrays[i].size() : maxSize;
        }
    }
    // if a ddbVector is shorter than the max ddbVector in current msg range
    //     use the last value to fill the gap
    for (int i = startIndex; i <= endIndex; ++i) {
        // DATA_TYPE type = types[i];
        DATA_TYPE type = DATA_TYPE(abs(int(dict[names[i]])));
        if (int(type) < ARRAY_TYPE_BASE) {
            switch (type) {
#define FILL_GAP(dtType, cppType, postfix)                                     \
    case dtType: {                                                             \
        int colSize = dataVec[i]->size();                                      \
        if (maxSize > colSize) {                                               \
            cppType value = ((VectorSP)dataVec[i])->get##postfix(colSize - 1); \
            for (int j = colSize; j < maxSize; ++j) {                          \
                ((VectorSP)dataVec[i])->append##postfix(&value, 1);            \
            }                                                                  \
        }                                                                      \
        break;                                                                 \
    }
                FILL_GAP(DT_INT, int, Int)
                FILL_GAP(DT_LONG, long long, Long)
                FILL_GAP(DT_BOOL, char, Char)
                FILL_GAP(DT_FLOAT, float, Float)
                FILL_GAP(DT_DOUBLE, double, Double)
                case DT_SYMBOL:
                case DT_BLOB:
                    FILL_GAP(DT_STRING, string, String)
#undef FILL_GAP
                default:
                    throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported type " +
                                           Util::getDataTypeString(types[i]) + ". " + Util::getDataTypeString(type));
            }
        } else {
            string marginErrMsg = ENCODERDECODER_PREFIX + "Failed to margin value of type";
            switch (type - ARRAY_TYPE_BASE) {
#define FILL_ARRAY_GAP(dtType, cppType, postfix)                                                           \
    case dtType: {                                                                                         \
        VectorSP dataContainer = dataVec[i];                                                               \
        vector<int> &indexArray = indexArrays[i];                                                          \
        int colSize = indexArray.size();                                                                   \
        if (maxSize > colSize) {                                                                           \
            if (colSize <= 0) {                                                                            \
                throw RuntimeException(marginErrMsg + std::to_string(type) + " col(" + std::to_string(i) + \
                                       ") size: " + std::to_string(colSize));                              \
            }                                                                                              \
            int lastBegin = 0;                                                                             \
            if (indexArray.size() > 1) {                                                                   \
                lastBegin = indexArray[colSize - 2];                                                       \
            }                                                                                              \
            int lastEnd = indexArray.back();                                                               \
            for (int j = colSize; j < maxSize; ++j) {                                                      \
                for (int k = lastBegin; k < lastEnd; ++k) {                                                \
                    cppType value = dataContainer->get##postfix(k);                                        \
                    dataContainer->append##postfix(&value, 1);                                             \
                }                                                                                          \
                indexArray.push_back(dataContainer->size());                                               \
            }                                                                                              \
        }                                                                                                  \
        break;                                                                                             \
    }
                FILL_ARRAY_GAP(DT_INT, int, Int)
                FILL_ARRAY_GAP(DT_BOOL, char, Char)
                FILL_ARRAY_GAP(DT_LONG, long long, Long)
                FILL_ARRAY_GAP(DT_FLOAT, float, Float)
                FILL_ARRAY_GAP(DT_DOUBLE, double, Double)
                FILL_ARRAY_GAP(DT_BLOB, string, String)
#undef FILL_ARRAY_GAP
                default:
                    throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported type " +
                                           Util::getDataTypeString(types[i]) + ". " + Util::getDataTypeString(type));
            }
        }
    }
}

/*
 * this function would flatten a protobuf message 'msg'
 * append its table-like data into ddbVector list param 'dataVec'
 *
 * take the same proto schema as example:
 * proto data:
 *   {A: a1,
 *    M: {B:b1, C:{c11,c12}},
 *    M: {B:b2, C:{c21}},
 *    M: {B:b3, C:{c31,c32, c33}},
 *    D: d1}
 *
 * the data above would transform into a dolphindb table like:
 * +---------+----------+----------+----------+
 * |    A    |   M_B    |    M_C   |    D     |
 * +---------+----------+----------+----------+
 * |    a1   |    b1    |    c11   |    d1    |
 * +---------+----------+----------+----------+
 * |    a1   |    b1    |    c12   |    d1    |
 * +---------+----------+----------+----------+
 * |    a1   |    b2    |    c21   |    d1    |
 * +---------+----------+----------+----------+
 * |    a1   |    b3    |    c31   |    d1    |
 * +---------+----------+----------+----------+
 * |    a1   |    b3    |    c32   |    d1    |
 * +---------+----------+----------+----------+
 * |    a1   |    b3    |    c33   |    d1    |
 * +---------+----------+----------+----------+
 */
void getMsgData(const Message &msg, MsgUtilPack &pack, string prefix, bool useZeroAsNull) {
    // vector<string> &names = pack.names_;
    vector<DATA_TYPE> &types = pack.types_;
    vector<ConstantSP> &dataVec = pack.dataVec_;
    unordered_map<string, DATA_TYPE> &dict = pack.dict_;
    unordered_map<string, unsigned int> &positionMap = pack.positionMap_;

    const Reflection *ref = msg.GetReflection();
    if (prefix != "") {
        prefix = prefix + "_";
    }
    const Descriptor *msgDesc = msg.GetDescriptor();
    int fieldNum = msgDesc->field_count();
    INDEX startIndex = -1;
    INDEX endIndex = -1;
    for (int i = 0; i < fieldNum; ++i) {
        const FieldDescriptor *field = msgDesc->field(i);
        string fieldName = prefix + field->name();
        if (field->type() != FieldDescriptor::TYPE_MESSAGE && dict.find(fieldName) == dict.end()) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "Unknown field " + fieldName + ".");
        }

        INDEX fieldIndex = positionMap[fieldName];

        if (field->type() != FieldDescriptor::TYPE_MESSAGE) {
            if (startIndex == -1) {
                startIndex = fieldIndex;
            };
            if (endIndex == -1) {
                endIndex = fieldIndex;
            };
            startIndex = fieldIndex < startIndex ? fieldIndex : startIndex;
            endIndex = fieldIndex > endIndex ? fieldIndex : endIndex;
        }

        if (field->is_repeated()) {
            int size = ref->FieldSize(msg, field);
            switch (field->type()) {
                case FieldDescriptor::Type::TYPE_INT32:
                case FieldDescriptor::Type::TYPE_SFIXED32:
                case FieldDescriptor::Type::TYPE_SINT32:
                    for (int j = 0; j < size; ++j) {
                        int ele = ref->GetRepeatedInt32(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendInt(&ele, 1);
                    }
                    if (size == 0) {
                        appendNull(DT_INT, fieldIndex, dataVec, useZeroAsNull);
                    }
                    break;
                case FieldDescriptor::Type::TYPE_FIXED32:  // unsigned int
                case FieldDescriptor::Type::TYPE_UINT32:   // unsigned int
                    for (int j = 0; j < size; ++j) {
                        long long ele = ref->GetRepeatedUInt32(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendLong(&ele, 1);
                    }
                    if (size == 0) {
                        appendNull(DT_LONG, fieldIndex, dataVec, useZeroAsNull);
                    }
                    break;
                case FieldDescriptor::Type::TYPE_INT64:
                case FieldDescriptor::Type::TYPE_SINT64:
                case FieldDescriptor::Type::TYPE_SFIXED64:
                    for (int j = 0; j < size; ++j) {
                        long long ele = ref->GetRepeatedInt64(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendLong(&ele, 1);
                    }
                    if (size == 0) {
                        appendNull(DT_LONG, fieldIndex, dataVec, useZeroAsNull);
                    }
                    break;
                case FieldDescriptor::Type::TYPE_FIXED64:
                case FieldDescriptor::Type::TYPE_UINT64:
                    for (int j = 0; j < size; ++j) {
                        unsigned long long ele = ref->GetRepeatedUInt64(msg, field, j);
                        if(ele > LONG_LONG_MAX) {
                            ele = LONG_LONG_MAX;
                        }
                        long long value = ele;
                        ((VectorSP)dataVec[fieldIndex])->appendLong(&value, 1);
                    }
                    if (size == 0) {
                        appendNull(DT_LONG, fieldIndex, dataVec, useZeroAsNull);
                    }
                    break;
                case FieldDescriptor::Type::TYPE_FLOAT:
                    for (int j = 0; j < size; ++j) {
                        float ele = ref->GetRepeatedFloat(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendFloat(&ele, 1);
                    }
                    if (size == 0) {
                        appendNull(DT_FLOAT, fieldIndex, dataVec, useZeroAsNull);
                    }
                    break;
                case FieldDescriptor::Type::TYPE_DOUBLE:
                    for (int j = 0; j < size; ++j) {
                        double ele = ref->GetRepeatedDouble(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendDouble(&ele, 1);
                    }
                    if (size == 0) {
                        appendNull(DT_DOUBLE, fieldIndex, dataVec, useZeroAsNull);
                    }
                    break;
                case FieldDescriptor::Type::TYPE_BOOL:
                    for (int j = 0; j < size; ++j) {
                        char ele = ref->GetRepeatedBool(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendChar(&ele, 1);
                    }
                    if (size == 0) {
                        appendNull(DT_BOOL, fieldIndex, dataVec, useZeroAsNull);
                    }
                    break;
                case FieldDescriptor::Type::TYPE_STRING:
                case FieldDescriptor::Type::TYPE_BYTES:
                    for (int j = 0; j < size; ++j) {
                        string ele = ref->GetRepeatedString(msg, field, j);
                        ((VectorSP)dataVec[fieldIndex])->appendString(&ele, 1);
                    }
                    if (size == 0) {
                        appendNull(DT_STRING, fieldIndex, dataVec, useZeroAsNull);
                    }
                    break;
                case FieldDescriptor::Type::TYPE_MESSAGE:
                    if (size == 0) {
                        appendMsgNull(field->message_type(), pack, fieldName, useZeroAsNull);
                    }

                    for (int j = 0; j < size; ++j) {
                        const Message &fieldMsg = ref->GetRepeatedMessage(msg, field, j);
                        getMsgData(fieldMsg, pack, fieldName, useZeroAsNull);

                        if (startIndex == -1) {
                            startIndex = pack.startIndex_;
                        };
                        if (endIndex == -1) {
                            endIndex = pack.endIndex_;
                        };
                        startIndex = pack.startIndex_ < startIndex ? pack.startIndex_ : startIndex;
                        endIndex = pack.endIndex_ > endIndex ? pack.endIndex_ : endIndex;
                    }
                    break;
                default:
                    throw RuntimeException(ENCODERDECODER_PREFIX + "Invalid type.");
            }
        } else {
            bool hasField = ref->HasField(msg, field);
            switch (field->type()) {
                case FieldDescriptor::Type::TYPE_INT32:
                case FieldDescriptor::Type::TYPE_SFIXED32:
                case FieldDescriptor::Type::TYPE_SINT32: {
                    if (!hasField) {
                        appendNull(DT_INT, fieldIndex, dataVec, useZeroAsNull);
                        break;
                    }
                    int ele = ref->GetInt32(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendInt(&ele, 1);
                } break;
                case FieldDescriptor::Type::TYPE_FIXED32:   // unsigned int
                case FieldDescriptor::Type::TYPE_UINT32: {  // unsigned int
                    if (!hasField) {
                        appendNull(DT_LONG, fieldIndex, dataVec, useZeroAsNull);
                        break;
                    }
                    long long ele = ref->GetUInt32(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendLong(&ele, 1);
                } break;
                case FieldDescriptor::Type::TYPE_INT64:
                case FieldDescriptor::Type::TYPE_SINT64:
                case FieldDescriptor::Type::TYPE_SFIXED64: {
                    if (!hasField) {
                        appendNull(DT_LONG, fieldIndex, dataVec, useZeroAsNull);
                        break;
                    }
                    long long ele = ref->GetInt64(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendLong(&ele, 1);
                } break;
                case FieldDescriptor::Type::TYPE_FIXED64:
                case FieldDescriptor::Type::TYPE_UINT64: {
                    if (!hasField) {
                        appendNull(DT_LONG, fieldIndex, dataVec, useZeroAsNull);
                        break;
                    }
                    unsigned long long ele = ref->GetUInt64(msg, field);
                    if(ele > LONG_LONG_MAX) {
                        ele = LONG_LONG_MAX;
                    }
                    long long value = ele;
                    ((VectorSP)dataVec[fieldIndex])->appendLong(&value, 1);
                } break;
                case FieldDescriptor::Type::TYPE_FLOAT: {
                    if (!hasField) {
                        appendNull(DT_FLOAT, fieldIndex, dataVec, useZeroAsNull);
                        break;
                    }
                    float ele = ref->GetFloat(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendFloat(&ele, 1);
                } break;
                case FieldDescriptor::Type::TYPE_DOUBLE: {
                    if (!hasField) {
                        appendNull(DT_DOUBLE, fieldIndex, dataVec, useZeroAsNull);
                        break;
                    }
                    double ele = ref->GetDouble(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendDouble(&ele, 1);
                } break;
                case FieldDescriptor::Type::TYPE_BOOL: {
                    if (!hasField) {
                        appendNull(DT_BOOL, fieldIndex, dataVec, useZeroAsNull);
                        break;
                    }
                    char ele = ref->GetBool(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendChar(&ele, 1);
                } break;
                case FieldDescriptor::Type::TYPE_STRING:
                case FieldDescriptor::Type::TYPE_BYTES: {
                    if (!hasField) {
                        appendNull(DT_STRING, fieldIndex, dataVec, useZeroAsNull);
                        break;
                    }
                    string ele = ref->GetString(msg, field);
                    ((VectorSP)dataVec[fieldIndex])->appendString(&ele, 1);
                } break;
                case FieldDescriptor::Type::TYPE_MESSAGE: {
                    if (!hasField) {
                        appendMsgNull(field->message_type(), pack, fieldName, useZeroAsNull);
                        break;
                    }
                    const Message &fieldMsg = ref->GetMessage(msg, field);
                    getMsgData(fieldMsg, pack, fieldName, useZeroAsNull);
                    if (startIndex == -1) {
                        startIndex = pack.startIndex_;
                    };
                    if (endIndex == -1) {
                        endIndex = pack.endIndex_;
                    };
                    startIndex = pack.startIndex_ < startIndex ? pack.startIndex_ : startIndex;
                    endIndex = pack.endIndex_ > endIndex ? pack.endIndex_ : endIndex;
                } break;
                default:
                    throw RuntimeException(ENCODERDECODER_PREFIX + "Invalid type.");
            }
        }
    }
    pack.startIndex_ = startIndex;
    pack.endIndex_ = endIndex;
    // find max ddbVector length of current msg range
    int maxSize = 0;
    for (int i = startIndex; i <= endIndex; ++i) {
        if (startIndex < 0 || startIndex > int(dataVec.size())) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "parse err.");
        }
        maxSize = maxSize < dataVec[i]->size() ? dataVec[i]->size() : maxSize;
    }
    // if a ddbVector is shorter than the max ddbVector in current msg range
    //     use the last value to fill the gap
    for (int i = startIndex; i <= endIndex; ++i) {
        switch (types[i]) {
#define FILL_GAP(dtType, cppType, postfix)                                     \
    case dtType: {                                                             \
        int colSize = dataVec[i]->size();                                      \
        if (maxSize > colSize) {                                               \
            cppType value = ((VectorSP)dataVec[i])->get##postfix(colSize - 1); \
            for (int j = colSize; j < maxSize; ++j) {                          \
                ((VectorSP)dataVec[i])->append##postfix(&value, 1);            \
            }                                                                  \
        }                                                                      \
        break;                                                                 \
    }
            FILL_GAP(DT_INT, int, Int)
            FILL_GAP(DT_LONG, long long, Long)
            FILL_GAP(DT_BOOL, char, Char)
            FILL_GAP(DT_FLOAT, float, Float)
            FILL_GAP(DT_DOUBLE, double, Double)
            case DT_SYMBOL:
            case DT_BLOB:
                FILL_GAP(DT_STRING, string, String)
#undef FILL_GAP
            default:
                throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported type " + Util::getDataTypeString(types[i]));
        }
    }
}

ConstantSP parseProtobufDynamic(string schemaPath, VectorSP data, unordered_map<string, DATA_TYPE> &columnTypeMap,
                                bool needArrayVector, Heap *heap, const string &protoName, bool useZeroAsNull) {
    // get binary data of protobuf schema
    ifstream in;
    in.open(schemaPath);
    string protobufStr((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    const char *text = protobufStr.c_str();

    // make Tokenizer
    ArrayInputStream rawInput(text, protobufStr.size());
    ddbErrorCollector errorCollector;
    Tokenizer input(&rawInput, &errorCollector);

    // Proto definition to a representation as used by the protobuf lib:
    /* FileDescriptorProto documentation:
     * A valid .proto file can be translated directly to a FileDescriptorProto
     * without any other information (e.g. without reading its imports).
     * */
    FileDescriptorProto fileDescProto;
    google::protobuf::compiler::Parser parser;
    if (!parser.Parse(&input, &fileDescProto)) {
        throw RuntimeException(ENCODERDECODER_PREFIX + "Failed to parse .proto definition " + schemaPath + " : " +
                               errorCollector.getErrorMsg());
    }

    // Set the name in fileDescProto as Parser::Parse does not do this:
    if (!fileDescProto.has_name()) {
        fileDescProto.set_name(schemaPath);
    }

    // Construct our own FileDescriptor for the proto file:
    /* FileDescriptor documentation:
     * Describes a whole .proto file.  To get the FileDescriptor for a compiled-in
     * file, get the descriptor for something defined in that file and call
     * descriptor->file().  Use DescriptorPool to construct your own descriptors.
     * */
    DescriptorPool pool;
    const FileDescriptor *fileDesc = pool.BuildFile(fileDescProto);
    if (fileDesc == NULL) {
        throw RuntimeException(ENCODERDECODER_PREFIX + "Cannot get file descriptor from file descriptor proto" +
                               fileDescProto.DebugString());
    }

    // As a .proto definition can contain more than one message Type,
    // select the message type that we are interested in
    // const google::protobuf::Descriptor* messageDesc =
    // fileDesc->FindMessageTypeByName(message_type);
    const Descriptor *messageDesc;
    if (protoName != "") {
        messageDesc = fileDesc->FindMessageTypeByName(protoName);
    } else {
        if (fileDesc->message_type_count() <= 0) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "no message type in file [" + schemaPath + "]");
        }
        messageDesc = fileDesc->message_type(0);
    }
    if (messageDesc == NULL) {
        throw RuntimeException(ENCODERDECODER_PREFIX + "Cannot get message descriptor of message: " + schemaPath +
                               ", DebugString(): " + fileDesc->DebugString());
    }
    string messageName = messageDesc->name();

    // Create an empty Message object that will hold the result of deserializing
    // a byte array for the proto definition:
    DynamicMessageFactory factory;
    const Message *prototypeMsg = factory.GetPrototype(messageDesc);  // prototypeMsg is immutable
    if (prototypeMsg == NULL) {
        throw RuntimeException(ENCODERDECODER_PREFIX + "Cannot create prototype message from message descriptor");
    }

    vector<string> names;
    vector<ConstantSP> cols;
    vector<DATA_TYPE> types;
    vector<ConstantSP> dataVec;
    vector<bool> repeatStatus;
    std::map<string, int> flagMap;

    // index array for array vector, use this and data array in dataVec to construct arrayVector
    vector<vector<int>> indexArrays;
    int vecSize = data->size();
    const Descriptor *msgDescriptor = prototypeMsg->GetDescriptor();
    if (msgDescriptor == NULL) {
        throw RuntimeException(ENCODERDECODER_PREFIX + "null des.");
    }

    using MessageSP = SmartPointer<Message>;
    vector<MessageSP> msgList;
    msgList.reserve(vecSize);

    LOG_INFO(ENCODERDECODER_PREFIX + "Size of messages: ", vecSize);
    if (vecSize == 0) {
        throw RuntimeException(ENCODERDECODER_PREFIX + "input is empty.");
    }
    // get Message of every protobuf strings in VectorSP 'data'
    for (int i = 0; i < vecSize; ++i) {
        MessageSP mutableMsg;
        try {
            string pbData = data->getString(i);
            const char *buffer = pbData.c_str();
            mutableMsg = prototypeMsg->New();
            if (mutableMsg == NULL) {
                throw RuntimeException(ENCODERDECODER_PREFIX + "Failed in Message->New(); to create mutable message");
            }
            // Deserialize a binary buffer that contains a message that is described by
            // the proto definition:
            if (!mutableMsg->ParseFromArray(buffer, pbData.size())) {
                throw RuntimeException(ENCODERDECODER_PREFIX + "Failed to parse value in buffer");
            }
            string dataName = mutableMsg->GetDescriptor()->name();
            if (dataName != messageName) {
                throw RuntimeException(ENCODERDECODER_PREFIX + "failed to parse protobuf data of type [" + dataName +
                                       "], expecting protobuf data of type [" + dataName + "] . ");
            }
        } catch (exception &ex) {
            string errMsg = ex.what();
            if (errMsg.find(ENCODERDECODER_PREFIX) == string::npos) {
                LOG_ERR(ENCODERDECODER_PREFIX + "", errMsg);
            } else {
                LOG_ERR(errMsg);
            }
            continue;
        }
        msgList.push_back(mutableMsg);
    }
    if (msgList.empty()) {
        throw RuntimeException(ENCODERDECODER_PREFIX + "all input protobuf data parse failed.");
    }

    int averageRepeatCount = 10;
    int containerLength = msgList.size() * averageRepeatCount;

    if (needArrayVector) {
        // prepare data frame for getMsgDataContainsVector function
        createTableFrame(messageDesc, names, types, "", columnTypeMap, repeatStatus, flagMap, unordered_set<string>{},
                         dataVec, indexArrays, true, containerLength);

        if (columnTypeMap.size() == 0) {
            for (unsigned int i = 0; i < names.size(); ++i) {
                columnTypeMap[names[i]] = types[i];
            }
        } else {
            for (unsigned int i = 0; i < names.size(); ++i) {
                DATA_TYPE dictType = DATA_TYPE(abs(int(columnTypeMap[names[i]])));
                if (types[i] == DT_BLOB) {
                    // TODO check string field type
                } else {
                    if (dictType != types[i]) {
                        throw RuntimeException(ENCODERDECODER_PREFIX + "Input schema is incompatible with proto. Column(" +
                                               std::to_string(i) + ") expect: " + Util::getDataTypeString(dictType) +
                                               ", but get:" + Util::getDataTypeString(types[i]));
                    }
                }
            }
        }
        std::map<string, bool> nameMap;
        for (auto &str : names) {
            if (nameMap[str]) {
                throw RuntimeException(ENCODERDECODER_PREFIX + "invalid proto syntax cause duplicate names.");
            }
            nameMap[str] = true;
        }
        // TODO check index array length
        string repeatDelayField;
        MsgUtilPack pack{names, types, dataVec, columnTypeMap};

        for (size_t i = 0; i < msgList.size(); ++i) {
            getMsgDataWithArrayVector(*(msgList[i].get()), pack, indexArrays, "", repeatDelayField, useZeroAsNull);
        }
        // create ddb table from data containers
        // TODO verify names & dataVec length.
        if (names.size() != dataVec.size()) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "parse failed.");
        }
        if (indexArrays.size() != indexArrays.size()) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "parse failed.");
        }
        for (int i = 0; i < int(names.size()); ++i) {
            DATA_TYPE type = DATA_TYPE(abs(int(columnTypeMap[names[i]])));
            if ((int)type < ARRAY_VECTOR_TYPE_BASE) {
                switch (type) {
                    case DT_INT:
                    case DT_LONG:
                    case DT_BOOL:
                    case DT_FLOAT:
                    case DT_DOUBLE:
                    case DT_STRING:
                    case DT_SYMBOL:
                    case DT_BLOB:
                        cols.push_back(dataVec[i]);
                        break;
                    default:
                        throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported data type " +
                                               Util::getDataTypeString(type) + " of field " + names[i] + ".");
                }
            } else {
                vector<int> &indexVec = indexArrays[i];
                VectorSP indexArray = Util::createVector(DT_INT, 0, indexVec.size());
                indexArray->appendInt(indexVec.data(), (int)indexVec.size());
                VectorSP valueArray;
                switch (type - ARRAY_VECTOR_TYPE_BASE) {
                    case DT_INT:
                    case DT_LONG:
                    case DT_BOOL:
                    case DT_FLOAT:
                    case DT_DOUBLE:
                        valueArray = dataVec[i];
                        break;
                    // EMPLACE_ARRAY_VECTOR_COL(DT_BLOB, string, appendString)
                    // arrayVector Unsupported string or blob
                    case DT_BLOB:
                    case DT_SYMBOL:
                    case DT_STRING: {
                        // use dict type, not types type to construct blob for string arrayVector
                        // TODO check dict type & types type.
                        valueArray = Util::createVector(DT_BLOB, 0, indexVec.size());
                        vector<string> vec;
                        vector<int> indexArray = indexArrays[i];

                        vector<char *> stringContainer(dataVec[i]->size());
                        char **stringBuf = dataVec[i]->getStringConst(0, dataVec[i]->size(), stringContainer.data());
                        int prev = 0;
                        for (unsigned int index = 0; index < indexArray.size(); ++index) {
                            string builder = "[";
                            for (int j = prev; j < indexArray[index] && j < dataVec[i]->size(); ++j) {
                                builder += "\"";
                                builder += stringBuf[j];
                                builder += "\",";
                            }
                            prev = indexArray[index];
                            builder.pop_back();
                            if(builder.empty()) { builder += "["; }
                            builder += "]";
                            vec.push_back(builder);
                        }
                        valueArray->appendString(vec.data(), vec.size());
                        // The string type will not execute the following logic.
                        cols.emplace_back(valueArray);
                        continue;
                    }
                    default:
                        throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported data type " +
                                               Util::getDataTypeString((DATA_TYPE)(type - ARRAY_VECTOR_TYPE_BASE)) +
                                               " of field " + names[i] + ".");
                }

                if (valueArray->size() == 0) {
                    vector<int> nullVec(vecSize, INT_MIN);
                    VectorSP vec = Util::createVector(DT_INT, 0, vecSize);
                    vec->appendInt(nullVec.data(), vecSize);
                    cols.push_back(vec);
                } else {
                    vector<ConstantSP> args{indexArray, valueArray};
                    ConstantSP arrayVector = heap->currentSession()->getFunctionDef("arrayVector")->call(heap, args);
                    cols.emplace_back(arrayVector);
                }
            }
        }
    } else {
        // prepare data frame for getMsg process
        vector<vector<int>> indexes = {};
        createTableFrame(messageDesc, names, types, "", columnTypeMap, repeatStatus, flagMap, unordered_set<string>{},
                         dataVec, indexes, false, containerLength);

        bool diff = false;
        if (columnTypeMap.size() == 0) {
            for (unsigned int i = 0; i < names.size(); ++i) {
                columnTypeMap[names[i]] = types[i];
            }
        } else {
            for (unsigned int i = 0; i < names.size(); ++i) {
                if (columnTypeMap[names[i]] != types[i]) {
                    diff = true;
                    columnTypeMap[names[i]] = types[i];
                }
            }
        }
        diff ? throw RuntimeException(ENCODERDECODER_PREFIX + "Input schema is incompatible with proto.") : 0;
        std::map<string, bool> nameMap;
        for (auto str : names) {
            if (nameMap[str]) {
                throw RuntimeException(ENCODERDECODER_PREFIX + "invalid proto syntax cause duplicate names.");
            }
            nameMap[str] = true;
        }
        MsgUtilPack pack{names, types, dataVec, columnTypeMap};
        for (size_t i = 0; i < msgList.size(); ++i) {
            getMsgData(*(msgList[i].get()), pack, "", useZeroAsNull);
        }

        for (int i = 0; i < int(names.size()); ++i) {
            switch (types[i]) {
                case DT_INT:
                case DT_LONG:
                case DT_BOOL:
                case DT_FLOAT:
                case DT_DOUBLE:
                case DT_BLOB:
                case DT_SYMBOL:
                case DT_STRING:
                    cols.push_back(dataVec[i]);
                    break;
                default:
                    throw RuntimeException(ENCODERDECODER_PREFIX + "Unsupported data type " +
                                           Util::getDataTypeString(types[i]) + " of field " + names[i] + ".");
            }
        }
    }

    for (ConstantSP col : cols) {
        col->setTemporary(true);
    }
    ConstantSP table = Util::createTable(names, cols);
    return table;
}

