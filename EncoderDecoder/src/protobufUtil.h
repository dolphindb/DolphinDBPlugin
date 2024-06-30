#ifndef PROTOBUF_UTIL_H
#define PROTOBUF_UTIL_H

#include "EncoderDecoder.h"
#include "ddbplugin/Plugin.h"
#include <CoreConcept.h>
#include <Exceptions.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Types.h>
#include <Util.h>

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
ConstantSP formatProtobuf(Heap* heap, vector<ConstantSP>& arguments);

// dynamic function
ConstantSP getProtobufSchemaDynamic(string filePath, bool needArrayVector, const unordered_set<string>& ignoredColumn,
                                    const string& messageName);
ConstantSP parseProtobufDynamic(string filePath, VectorSP data, unordered_map<string, DATA_TYPE>& dict,
                                bool needArrayVector = false, Heap* heap = nullptr, const string& = "",
                                bool useZeroAsNull = true);

#endif //PROTOBUF_UTIL_H