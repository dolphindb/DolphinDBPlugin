#ifndef PROTOBUF_UTIL_H
#define PROTOBUF_UTIL_H

#include "DolphinDBEverything.h"
#include "EncoderDecoder.h"
#include "ddbplugin/Plugin.h"
#include <CoreConcept.h>
#include <Exceptions.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Types.h>
#include <Util.h>
#if defined(_MSC_VER)
#elif defined(__clang__)
#else // gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include "google/protobuf/message.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/dynamic_message.h"

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#else // gcc
#pragma GCC diagnostic pop
#endif

using namespace ddb;
namespace protobufCoder {

enum {
    SINGLE = 0,
    MULTI = 1,
};

}

ConstantSP protobufSchema(string schemeName, bool needArrayVector, const unordered_set<string>& ignoredColumn,
                          const string& messageName);
ConstantSP parseProtobuf(Heap* heap, vector<ConstantSP>& arguments);

ConstantSP parseProtobufWrapper(Heap* heap, vector<ConstantSP>& arguments);
// ConstantSP formatProtobuf(Heap* heap, vector<ConstantSP>& arguments);

// dynamic function
ConstantSP getProtobufSchemaDynamic(string filePath, bool needArrayVector, const unordered_set<string>& ignoredColumn,
                                    const string& messageName);
ConstantSP parseProtobufDynamic(string filePath, VectorSP data, unordered_map<string, DATA_TYPE>& dict,
                                bool needArrayVector = false, Heap* heap = nullptr, const string& = "",
                                bool useZeroAsNull = true);
const google::protobuf::Message *getMessageFromProtoFile(const std::string& filePath, const std::string& protoName, google::protobuf::DescriptorPool& pool, google::protobuf::DynamicMessageFactory& factory);
#endif //PROTOBUF_UTIL_H
