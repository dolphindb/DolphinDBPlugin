#include "plugin.h"
#include "s3_config.h"
#include "s3_client.h"

#include "Concurrent.h"
#include "CoreConcept.h"
#include "ddbplugin/Plugin.h"
#include "ddbplugin/PluginTypeUtils.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws/core/client/UserAgent.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Object.h>

#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#if __cplusplus >= 201703L
#include <optional>
#else
#include "ddbplugin/FixSTL.h"
#endif

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

using namespace ddb; // NOLINT(google-build-using-namespace)

namespace {

const std::string AWSS3_PLUGIN_PREFIX = "[PLUGIN AWS]:";

std::map<std::string, s3_client> s3Clients; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::mutex s3ClientsMutex; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

const std::string ACCOUNT_SHOULD_BE_DICTIONARY = "s3account should be a dictionary.";
const std::string BUCKET_SHOULD_BE_STRING = "bucket should be a string scalar.";
const std::string KEY_SHOULD_BE_STRING = "key should be a string scalar.";
const std::string KEY_SHOULD_BE_STRING_VECTOR = "key should be a string scalar or vector.";
const std::string PREFIX_SHOULD_BE_STRING = "prefix should be a string scalar.";
const std::string OUTPUT_FILE_SHOULD_BE_STRING = "outputFileName should be a string scalar.";
const std::string INPUT_FILE_SHOULD_BE_STRING_VECTOR = "input file name should be a string scalar or vector.";
const std::string SRC_PATH_SHOULD_BE_STRING_VECTOR = "srcPath should be a string scalar or vector.";
const std::string DEST_PATH_SHOULD_BE_STRING_VECTOR = "destPath should be a string scalar or vector.";
const std::string OBJECTS_SHOULD_BE_STRING_VECTOR = "objects should be a string scalar or vector.";
const std::string MARKER_SHOULD_BE_STRING = "marker should be a string scalar.";
const std::string DELIMITER_SHOULD_BE_STRING = "delimiter should be a string scalar.";
const std::string NEXT_MARKER_SHOULD_BE_STRING = "nextMarker should be a string scalar.";
const std::string LIMIT_SHOULD_BE_INTEGRAL = "limit should be an int or long scalar.";
const std::string THREAD_COUNT_SHOULD_BE_INTEGRAL = "threadCount should be an int or long scalar.";
const std::string OFFSET_LENGTH_SHOULD_BE_INTEGRAL = "offset or length should be an int or long scalar.";

template <typename Arg>
void checkArg(const Arg& arg, const std::string& func, const std::string& err) {
    if (!arg) {
        throw IllegalArgumentException(func, err);
    }
}

template <typename Outcome>
void checkOutcome(const Outcome& outcome, const std::string& action) {
    if (outcome.IsSuccess()) {
        return;
    }
    const auto& error = outcome.GetError();
    string errorMsg = "Failed to " + action + ": [" + error.GetExceptionName() + "] " + error.GetMessage();
    throw IOException(errorMsg);
}

VectorSP createStringVector(const std::vector<std::string>& values) {
    VectorSP result = Util::createVector(DT_STRING, 0);
    if (!values.empty()) {
        result->appendString(values.data(), values.size());
    }
    return result;
}

std::vector<std::string> getUserAgentFeatures(const Aws::Auth::CredentialsResolutionContext& context) {
    using Aws::Client::UserAgentFeature;
    static const std::map<UserAgentFeature, std::string> userAgentFeatureNames = {
        {Aws::Client::UserAgentFeature::RETRY_MODE_LEGACY, "RETRY_MODE_LEGACY"},
        {Aws::Client::UserAgentFeature::RETRY_MODE_STANDARD, "RETRY_MODE_STANDARD"},
        {Aws::Client::UserAgentFeature::RETRY_MODE_ADAPTIVE, "RETRY_MODE_ADAPTIVE"},
        {Aws::Client::UserAgentFeature::S3_TRANSFER, "S3_TRANSFER"},
        {Aws::Client::UserAgentFeature::S3_CRYPTO_V1N, "S3_CRYPTO_V1N"},
        {Aws::Client::UserAgentFeature::S3_CRYPTO_V2, "S3_CRYPTO_V2"},
        {Aws::Client::UserAgentFeature::S3_EXPRESS_BUCKET, "S3_EXPRESS_BUCKET"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_REQ_CRC32, "FLEXIBLE_CHECKSUMS_REQ_CRC32"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_REQ_CRC32C, "FLEXIBLE_CHECKSUMS_REQ_CRC32C"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_REQ_CRC64, "FLEXIBLE_CHECKSUMS_REQ_CRC64"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_REQ_SHA1, "FLEXIBLE_CHECKSUMS_REQ_SHA1"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_REQ_SHA256, "FLEXIBLE_CHECKSUMS_REQ_SHA256"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_REQ_WHEN_SUPPORTED, "FLEXIBLE_CHECKSUMS_REQ_WHEN_SUPPORTED"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_REQ_WHEN_REQUIRED, "FLEXIBLE_CHECKSUMS_REQ_WHEN_REQUIRED"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_RES_WHEN_SUPPORTED, "FLEXIBLE_CHECKSUMS_RES_WHEN_SUPPORTED"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_RES_WHEN_REQUIRED, "FLEXIBLE_CHECKSUMS_RES_WHEN_REQUIRED"},
        {Aws::Client::UserAgentFeature::ACCOUNT_ID_MODE_PREFERRED, "ACCOUNT_ID_MODE_PREFERRED"},
        {Aws::Client::UserAgentFeature::ACCOUNT_ID_MODE_DISABLED, "ACCOUNT_ID_MODE_DISABLED"},
        {Aws::Client::UserAgentFeature::ACCOUNT_ID_MODE_REQUIRED, "ACCOUNT_ID_MODE_REQUIRED"},
        {Aws::Client::UserAgentFeature::RESOLVED_ACCOUNT_ID, "RESOLVED_ACCOUNT_ID"},
        {Aws::Client::UserAgentFeature::GZIP_REQUEST_COMPRESSION, "GZIP_REQUEST_COMPRESSION"},
        {Aws::Client::UserAgentFeature::CREDENTIALS_ENV_VARS, "CREDENTIALS_ENV_VARS"},
        {Aws::Client::UserAgentFeature::CREDENTIALS_PROFILE, "CREDENTIALS_PROFILE"},
        {Aws::Client::UserAgentFeature::CREDENTIALS_PROFILE_PROCESS, "CREDENTIALS_PROFILE_PROCESS"},
        {Aws::Client::UserAgentFeature::CREDENTIALS_IMDS, "CREDENTIALS_IMDS"},
        {Aws::Client::UserAgentFeature::CREDENTIALS_STS_ASSUME_ROLE, "CREDENTIALS_STS_ASSUME_ROLE"},
        {Aws::Client::UserAgentFeature::CREDENTIALS_STS_WEB_IDENTITY_TOKEN, "CREDENTIALS_STS_WEB_IDENTITY_TOKEN"},
        {Aws::Client::UserAgentFeature::CREDENTIALS_HTTP, "CREDENTIALS_HTTP"},
        {Aws::Client::UserAgentFeature::CREDENTIALS_SSO, "CREDENTIALS_SSO"},
        {Aws::Client::UserAgentFeature::CREDENTIALS_SSO_LEGACY, "CREDENTIALS_SSO_LEGACY"},
        {Aws::Client::UserAgentFeature::CREDENTIALS_PROFILE_SOURCE_PROFILE, "CREDENTIALS_PROFILE_SOURCE_PROFILE"},
        {Aws::Client::UserAgentFeature::CREDENTIALS_LOGIN, "CREDENTIALS_LOGIN"},
        {Aws::Client::UserAgentFeature::PROTOCOL_RPC_V2_CBOR, "PROTOCOL_RPC_V2_CBOR"},
        {Aws::Client::UserAgentFeature::BEARER_SERVICE_ENV_VARS, "BEARER_SERVICE_ENV_VARS"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_REQ_SHA512, "FLEXIBLE_CHECKSUMS_REQ_SHA512"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_REQ_XXHASH64, "FLEXIBLE_CHECKSUMS_REQ_XXHASH64"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_REQ_XXHASH3, "FLEXIBLE_CHECKSUMS_REQ_XXHASH3"},
        {Aws::Client::UserAgentFeature::FLEXIBLE_CHECKSUMS_REQ_XXHASH128, "FLEXIBLE_CHECKSUMS_REQ_XXHASH128"},
        {Aws::Client::UserAgentFeature::PAGINATOR, "PAGINATOR"},
        {Aws::Client::UserAgentFeature::WAITER, "WAITER"},
    };
    std::vector<std::string> features;
    auto resolvedFeatures = context.GetUserAgentFeatures();
    for (auto feature : resolvedFeatures) {
        auto it = userAgentFeatureNames.find(feature);
        if (it != userAgentFeatureNames.end()) {
            features.push_back(it->second);
        }
    }
    return features;
}

s3_client createS3Client(DictionarySP& account, const Aws::S3::S3ClientConfiguration& clientConfig) {
    auto account_config = configure_s3_account(account, clientConfig);
    auto config = account_config.config;
    LOG("PluginAWS: requestTimeoutMs is set to ", config.requestTimeoutMs);
    LOG("PluginAWS: maxConnections is set to ", config.maxConnections);
    LOG("PluginAWS: verifySSL is set to ", config.verifySSL);
    std::shared_ptr<Aws::S3::S3Client> client;
    if (account_config.use_explicit_credentials) {
        client = std::make_shared<Aws::S3::S3Client>(account_config.credential, nullptr, config);
    } else {
        client = std::make_shared<Aws::S3::S3Client>(config);
    }
    return s3_client(std::move(client), std::move(config));
}

s3_client leaseS3Client(DictionarySP& account) {
    string accountKey=account->getString();
    auto config = get_s3_client_config();
    std::lock_guard<std::mutex> lock(s3ClientsMutex);
    auto client = s3Clients.find(accountKey);
    if (client == s3Clients.end()) {
        client = s3Clients.emplace(accountKey, createS3Client(account, config)).first;
    }
    return client->second;
}

} // namespace

ConstantSP initialize(Heap* heap, std::vector<ConstantSP>& args) {
    (void)heap;
    (void)args;
    static std::once_flag flag;
    std::call_once(flag, []() {
        LOG("InitAPI");
        Aws::SDKOptions options;
        Aws::InitAPI(options);
        Aws::Utils::Logging::InitializeAWSLogging(Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
            "AWS Logging", Aws::Utils::Logging::LogLevel::Trace, "aws_sdk_"));
    });
    initialize_s3_client_config();
    return Util::createConstant(DT_VOID);
}

void setClientConfig(Heap* heap, std::vector<ConstantSP>& args) {
    (void)heap;
    checkArg(args[0]->isDictionary(), __func__, "config should be a dictionary.");
    update_s3_client_config(DictionarySP(args[0]));
    {
        std::lock_guard<std::mutex> lock(s3ClientsMutex);
        s3Clients.clear();
    }
}

ConstantSP getClientConfig(Heap* heap, std::vector<ConstantSP>& args) {
    (void)heap;

    auto config = get_s3_client_config();
    if (args.size() == 1) {
        checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
        DictionarySP s3account = DictionarySP(args[0]);
        config = leaseS3Client(s3account).config();
    }

    DictionarySP ret = Util::createDictionary(DT_STRING, SymbolBaseSP(), DT_ANY, SymbolBaseSP());
    ret->set("maxConnections", new Long(config.maxConnections));
    ret->set("requestTimeoutMs", new Long(config.requestTimeoutMs));
    ret->set("verifySSL", new Bool(config.verifySSL));
    return ret;
}

ConstantSP getS3Object(Heap* heap, vector<ConstantSP>& args) {
    (void)heap;
    auto bucketArg = arg_to_string(args[1]);
    auto keyArg = arg_to_string(args[2]);
    std::optional<std::string> outputFileArg;
    if (args.size() == 4) {
        outputFileArg = arg_to_string(args[3]);
    }

    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
    checkArg(bucketArg, __func__, BUCKET_SHOULD_BE_STRING);
    checkArg(keyArg, __func__, KEY_SHOULD_BE_STRING);
    checkArg(args.size() != 4 || outputFileArg, __func__, OUTPUT_FILE_SHOULD_BE_STRING);

    ConstantSP ret = Util::createConstant(DT_STRING);
    const Aws::String& bucketName = *bucketArg;
    const Aws::String& keyName = *keyArg;
    Aws::String outputFileName = keyName;
    if (args.size() == 4) {
        outputFileName = *outputFileArg;
    }

    auto pos = outputFileName.find_last_of(R"(/\)");
    if (pos != string::npos && pos != 0) {
        auto dirName = outputFileName.substr(0, pos);
        string errMsg;
        if (!Util::createDirectoryRecursive(dirName, errMsg)) {
            throw IOException(errMsg);
        }
    }

    if (Util::existsDir(outputFileName)) {
        throw RuntimeException(std::string("already exist a dir with the same name ") + outputFileName);
    }

    DictionarySP s3account = DictionarySP(args[0]);
    s3_client client = leaseS3Client(s3account);
    auto response = [&outputFileName]() {
        auto *fptr = Aws::New<Aws::FStream>("FStream to download file", outputFileName.c_str(),
                                            std::ios_base::out | std::ios_base::binary);
        if (!fptr->good()) {
            Aws::Delete<Aws::FStream>(fptr);
            throw IOException(AWSS3_PLUGIN_PREFIX + ": open output file [" + outputFileName + "] failed");
        }
        return fptr;
    };
    s3_get_object_options getOptions;
    getOptions.response_stream_factory = response;
    auto outcome = client.get_object({bucketName, keyName}, getOptions);
    checkOutcome(outcome, "get object");
    ret->setString(outputFileName.c_str());
    return ret;
}

ConstantSP listS3Object(Heap* heap, vector<ConstantSP>& args) {
    (void)heap;
    auto bucketArg = arg_to_string(args[1]);
    auto prefixArg = arg_to_string(args[2]);
    std::optional<std::string> markerArg;
    std::optional<std::string> delimiterArg;
    std::optional<std::string> nextMarkerArg;
    std::optional<int64_t> limitArg;
    if (args.size() >= 4) {
        markerArg = arg_to_string(args[3]);
    }
    if (args.size() >= 5) {
        delimiterArg = arg_to_string(args[4]);
    }
    if (args.size() >= 6) {
        nextMarkerArg = arg_to_string(args[5]);
    }
    if (args.size() >= 7) {
        limitArg = arg_to_int64(args[6]);
    }

    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
    checkArg(bucketArg, __func__, BUCKET_SHOULD_BE_STRING);
    checkArg(prefixArg, __func__, PREFIX_SHOULD_BE_STRING);
    ConstantSP nextMarker;
    checkArg(args.size() < 4 || markerArg, __func__, MARKER_SHOULD_BE_STRING);
    checkArg(args.size() < 5 || delimiterArg, __func__, DELIMITER_SHOULD_BE_STRING);
    if (args.size() >= 6) {
        checkArg(nextMarkerArg, __func__, NEXT_MARKER_SHOULD_BE_STRING);
        nextMarker = args[5];
    }
    checkArg(args.size() < 7 || (limitArg && *limitArg >= 0 && *limitArg <= std::numeric_limits<int>::max()),
              __func__, LIMIT_SHOULD_BE_INTEGRAL);
    
    std::vector<std::string> colName{"index", "bucket name", "key name", "last modified", "length", "ETag" , "owner"};

    std::vector<long long> tblIdx;
    VectorSP tableIndex = Util::createVector(DT_LONG, 0);
    std::vector<std::string> tblBN;
    VectorSP tableBucketName = Util::createVector(DT_STRING, 0);
    std::vector<std::string> tblKN;
    VectorSP tableKeyName = Util::createVector(DT_STRING, 0);
    std::vector<std::string> tblLM;
    VectorSP tableLastModified = Util::createVector(DT_STRING, 0);
    std::vector<long long> tblLen;
    VectorSP tableLength = Util::createVector(DT_LONG, 0);
    std::vector<std::string> tblET;
    VectorSP tableETag = Util::createVector(DT_STRING, 0);
    std::vector<std::string> tblOwner;
    VectorSP tableOwner = Util::createVector(DT_STRING, 0);

    const Aws::String& bucketName = *bucketArg;
    const Aws::String& prefixName = *prefixArg;

    auto s3account = DictionarySP(args[0]);
    s3_client client = leaseS3Client(s3account);
    ListObjectsOptions options;
    if (markerArg && !args[3]->isNull()) {
        options.marker = *markerArg;
    }
    if (delimiterArg && !args[4]->isNull()) {
        options.delimiter = *delimiterArg;
    }
    if (limitArg) {
        options.limit = static_cast<int>(*limitArg);
    }

    auto outcome = client.list_objects(bucketName, prefixName, options);
    checkOutcome(outcome, "list objects");
    const auto &result = outcome.GetResult();
    Aws::Vector<Aws::S3::Model::Object> objectList = result.GetContents();
    long long i = 1;
    // set nextmarker
    if (!nextMarker.isNull()) {
        if (result.GetIsTruncated()) {
            nextMarker->setString(result.GetNextMarker().c_str());
            if (nextMarker->isNull() && objectList.size() > 0) {
                nextMarker->setString(objectList.back().GetKey().c_str());
            }
        } else {
            nextMarker->setString("");
        }
    }
    LOG("[listS3Object] marker ", markerArg ? *markerArg : " ", " turncated ", result.GetIsTruncated());
    for (auto const &s3Object : objectList) {
        tblIdx.emplace_back(i++);
        tblBN.emplace_back(*bucketArg);
        tblKN.emplace_back(s3Object.GetKey());
        tblLM.emplace_back(s3Object.GetLastModified().ToLocalTimeString(Aws::Utils::DateFormat::ISO_8601));
        tblLen.emplace_back(s3Object.GetSize());
        tblET.emplace_back(s3Object.GetETag());
        tblOwner.emplace_back(s3Object.GetOwner().GetID());
    }
    for (auto const &prefix : result.GetCommonPrefixes()) {
        auto dirName = prefix.GetPrefix();
        tblIdx.emplace_back(i++);
        tblBN.emplace_back(*bucketArg);
        tblKN.emplace_back(dirName);
        tblLM.emplace_back("");
        tblLen.emplace_back(0);
        tblET.emplace_back("");
        tblOwner.emplace_back("");
    }
    int rowNumber = static_cast<int>(tblIdx.size());
    tableIndex->appendLong(tblIdx.data(), rowNumber);
    tableBucketName->appendString(tblBN.data(), rowNumber);
    tableKeyName->appendString(tblKN.data(), rowNumber);
    tableLastModified->appendString(tblLM.data(), rowNumber);
    tableLength->appendLong(tblLen.data(), rowNumber);
    tableETag->appendString(tblET.data(), rowNumber);
    tableOwner->appendString(tblOwner.data(), rowNumber);
    return Util::createTable(colName, 
        {tableIndex,tableBucketName,tableKeyName,tableLastModified,tableLength,tableETag,tableOwner});
}

ConstantSP readS3Object(Heap* heap, vector<ConstantSP>& args) {
    (void)heap;
    auto bucketArg = arg_to_string(args[1]);
    auto keyArg = arg_to_string(args[2]);
    auto offsetArg = arg_to_int64(args[3]);
    auto lengthArg = arg_to_int64(args[4]);

    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
    checkArg(bucketArg, __func__, BUCKET_SHOULD_BE_STRING);
    checkArg(keyArg, __func__, KEY_SHOULD_BE_STRING);
    checkArg(offsetArg && lengthArg, __func__, OFFSET_LENGTH_SHOULD_BE_INTEGRAL);
    int64_t off = *offsetArg;
    int64_t len = *lengthArg;
    if (off < 0 || len <= 0) {
        throw IllegalArgumentException("readS3Object", "Invalid range, offset should >= 0, length should > 0.");
    }

    DictionarySP s3account = DictionarySP(args[0]);

    s3_client client = leaseS3Client(s3account);
    const Aws::String& bucketName = *bucketArg;
    const Aws::String& keyName = *keyArg;
    s3_get_object_options getOptions;
    getOptions.range = s3_object_range{off, len};
    auto outcome = client.get_object({bucketName, keyName}, getOptions);
    checkOutcome(outcome, "read object");
    auto objectResult = outcome.GetResultWithOwnership();
    const auto contentLength = objectResult.GetContentLength();
    if (contentLength < 0 || contentLength > std::numeric_limits<int>::max()) {
        throw RuntimeException("readS3Object result is too large.");
    }

    int resultSize = static_cast<int>(contentLength);
    VectorSP ret = Util::asContiguous(Util::createVector(DT_CHAR, resultSize, resultSize));
    if (resultSize == 0) {
        LOG("[readS3Object] got 0 bytes");
        return ret;
    }

    char* data = ret->getCharBuffer(0, resultSize, nullptr);
    objectResult.GetBody().read(data, resultSize);
    resultSize = static_cast<int>(objectResult.GetBody().gcount());
    if (resultSize != contentLength) {
        ret->resize(resultSize);
    }
    LOG("[readS3Object] got ", resultSize, " bytes");
    return ret;
}

void deleteS3Object(Heap* heap, vector<ConstantSP>& args) {
    (void)heap;
    auto bucketArg = arg_to_string(args[1]);
    auto keyArg = arg_to_string_vector(args[2]);

    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
    checkArg(bucketArg, __func__, BUCKET_SHOULD_BE_STRING);
    checkArg(keyArg, __func__, KEY_SHOULD_BE_STRING_VECTOR);

    DictionarySP s3account = DictionarySP(args[0]);
    s3_client client = leaseS3Client(s3account);
    auto outcomes = client.delete_object(*bucketArg, *keyArg);
    for (const auto &outcome : outcomes) {
        checkOutcome(outcome, "delete object");
    }
}

void uploadS3Object(Heap* heap, vector<ConstantSP>& args) {
    (void)heap;
    auto bucketArg = arg_to_string(args[1]);
    auto keyArg = arg_to_string_vector(args[2]);
    auto inputFileArg = arg_to_string_vector(args[3]);

    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
    checkArg(bucketArg, __func__, BUCKET_SHOULD_BE_STRING);
    checkArg(keyArg, __func__, KEY_SHOULD_BE_STRING_VECTOR);
    checkArg(inputFileArg, __func__, INPUT_FILE_SHOULD_BE_STRING_VECTOR);

    const auto& keys = *keyArg;
    const auto& from = *inputFileArg;
    if (keys.size() != from.size()) {
        throw IllegalArgumentException(__func__, "input files and keys are mismatched");
    }
    std::vector<s3_upload_object> objects;
    objects.reserve(keys.size());
    for (size_t i = 0; i < keys.size(); ++i) {
        objects.push_back({keys[i], from[i]});
    }

    DictionarySP s3account = DictionarySP(args[0]);
    s3_client client = leaseS3Client(s3account);
    auto futures = client.put_object(*bucketArg, objects);
    auto upload = objects.begin();
    for (auto& future : futures) {
        checkOutcome(future.get(), "upload object " + (upload++)->key);
    }
}

ConstantSP listS3Bucket(Heap* heap, vector<ConstantSP>& args) {
    (void)heap;
    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
    std::vector<std::string> colName{"index", "bucket name", "creation date"};
    std::vector<long long> tblIdx;
    VectorSP tableIndex = Util::createVector(DT_LONG, 0);
    std::vector<std::string> tblBN;
    VectorSP tableBucketName = Util::createVector(DT_STRING, 0);
    std::vector<std::string> tblCD;
    VectorSP tableCreationDate = Util::createVector(DT_STRING, 0);
    {
        DictionarySP s3account = DictionarySP(args[0]);
        s3_client client = leaseS3Client(s3account);
        auto outcome = client.list_buckets();
        checkOutcome(outcome, "list buckets");
        const auto &result = outcome.GetResult();
        Aws::Vector<Aws::S3::Model::Bucket> bucketList = result.GetBuckets();
        long long i = 1;
        for (auto const &s3Bucket : bucketList) {
            tblIdx.emplace_back(i++);
            tblBN.emplace_back(s3Bucket.GetName().c_str());
            tblCD.emplace_back(s3Bucket.GetCreationDate().ToLocalTimeString(Aws::Utils::DateFormat::ISO_8601).c_str());
        }
    }
    int rowNumber = static_cast<int>(tblIdx.size());
    tableIndex->appendLong(tblIdx.data(), rowNumber);
    tableBucketName->appendString(tblBN.data(), rowNumber);
    tableCreationDate->appendString(tblCD.data(), rowNumber);
    return Util::createTable(colName, {tableIndex, tableBucketName, tableCreationDate});
}

ConstantSP getUserAgentFeature(Heap* heap, vector<ConstantSP>& args) {
    (void)heap;
    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);

    DictionarySP s3account = DictionarySP(args[0]);
    std::vector<std::string> features;
    auto account_config = configure_s3_account(s3account, get_s3_client_config());
    if (account_config.use_explicit_credentials) {
        features.emplace_back("CREDENTIALS_EXPLICIT");
    } else {
        Aws::Auth::DefaultAWSCredentialsProviderChain provider(account_config.config.credentialProviderConfig);
        auto credentials = provider.GetAWSCredentials();
        features = getUserAgentFeatures(credentials.GetContext());
    }

    return createStringVector(features);
}

void deleteS3Bucket(Heap* heap, vector<ConstantSP>& args) {
    (void)heap;
    auto bucketArg = arg_to_string(args[1]);

    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
    checkArg(bucketArg, __func__, BUCKET_SHOULD_BE_STRING);

    DictionarySP s3account = DictionarySP(args[0]);
    s3_client client = leaseS3Client(s3account);
    auto outcome = client.delete_bucket(*bucketArg);
    checkOutcome(outcome, "delete bucket");
}

void createS3Bucket(Heap* heap, vector<ConstantSP>& args) {
    (void)heap;
    auto bucketArg = arg_to_string(args[1]);

    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
    checkArg(bucketArg, __func__, BUCKET_SHOULD_BE_STRING);

    DictionarySP s3account = DictionarySP(args[0]);
    s3_client client = leaseS3Client(s3account);
    auto outcome = client.create_bucket(*bucketArg);
    checkOutcome(outcome, "create bucket");
}

ConstantSP headS3Object(Heap* heap, vector<ConstantSP>& args) {
    (void)heap;
    auto bucketArg = arg_to_string(args[1]);
    auto keyArg = arg_to_string(args[2]);

    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
    checkArg(bucketArg, __func__, BUCKET_SHOULD_BE_STRING);
    checkArg(keyArg, __func__, KEY_SHOULD_BE_STRING);
    DictionarySP s3account = DictionarySP(args[0]);
    s3_client client = leaseS3Client(s3account);

    auto outcome = client.head_object({*bucketArg, *keyArg});
    checkOutcome(outcome, "get object information");
    const auto& result = outcome.GetResult();
    DictionarySP ret = Util::createDictionary(DT_STRING, SymbolBaseSP(), DT_ANY, SymbolBaseSP());
    ret->set("bucket name", new String(*bucketArg));
    ret->set("key name", new String(*keyArg));
    ret->set("length", new Long(result.GetContentLength()));
    ret->set("last modified", new String(result.GetLastModified().ToLocalTimeString(Aws::Utils::DateFormat::ISO_8601).c_str()));
    ret->set("ETag", new String(result.GetETag().c_str()));
    ret->set("content type", new String(result.GetContentType().c_str()));
    return ret;
}

void copyS3Object(Heap* heap, vector<ConstantSP>& args) {
    (void)heap;
    auto bucketArg = arg_to_string(args[1]);
    auto srcPathArg = arg_to_string_vector(args[2]);
    auto destPathArg = arg_to_string_vector(args[3]);

    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
    checkArg(bucketArg, __func__, BUCKET_SHOULD_BE_STRING);
    checkArg(srcPathArg, __func__, SRC_PATH_SHOULD_BE_STRING_VECTOR);
    checkArg(destPathArg, __func__, DEST_PATH_SHOULD_BE_STRING_VECTOR);

    const auto& srcPaths = *srcPathArg;
    const auto& destPaths = *destPathArg;
    if (srcPaths.size() != destPaths.size()) {
        throw IllegalArgumentException(__func__, " srcPath and destPath are mismatched");
    }
    std::vector<s3_copy_object> objects;
    objects.reserve(srcPaths.size());
    for (size_t i = 0; i < srcPaths.size(); ++i) {
        objects.push_back({srcPaths[i], destPaths[i]});
    }
    DictionarySP s3account = DictionarySP(args[0]);
    s3_client client = leaseS3Client(s3account);
    auto futures = client.copy_object(*bucketArg, objects);
    auto copy = objects.begin();
    for (auto& future : futures) {
        checkOutcome(future.get(), "copy object " + (copy++)->src);
    }
}

ConstantSP loadS3Object(Heap* heap, vector<ConstantSP>& args){
    //4+3+4 = 7-11 parameters
    auto bucketArg = arg_to_string(args[1]);
    auto objectsArg = arg_to_string_vector(args[2]);
    auto threadCountArg = arg_to_int64(args[3]);

    checkArg(args[0]->isDictionary(), __func__, ACCOUNT_SHOULD_BE_DICTIONARY);
    checkArg(bucketArg, __func__, BUCKET_SHOULD_BE_STRING);
    checkArg(objectsArg, __func__, OBJECTS_SHOULD_BE_STRING_VECTOR);
    checkArg(threadCountArg, __func__, THREAD_COUNT_SHOULD_BE_INTEGRAL);
    const int loadTextExBeginIndex = 4;
    DictionarySP s3account = DictionarySP(args[0]);
    std::string bucketName(*bucketArg);
    //next 3 parameter are loadTextEx(dbHandle, tableName, partitionColumns)
    //missing filename parameter in loadTextEx
    //next 4 parameter are loadTextEx(..., [delimiter], [schema], [skipRows=0], [transform])
    const auto& objects = *objectsArg;
    if(objects.empty()){
        throw RuntimeException("Object count 0 must be equal or greater than 1.");
    }
    if(*threadCountArg < 1){
        throw RuntimeException("Thread count "+std::to_string(*threadCountArg)+" must be equal or greater than 1.");
    }
    if(*threadCountArg > 10){
        throw RuntimeException("Thread count "+std::to_string(*threadCountArg)+" must be equal or less than 10.");
    }
    int threadCount = static_cast<int>(*threadCountArg);

    //prepare loadTextEx
    auto loadTextExFunc=Util::getFuncDefFromHeap(heap, "loadTextEx");
    if(loadTextExFunc.isNull()){
        throw RuntimeException("Can't find function loadTextEx");
    }
    //loadTextEx(dbHandle, tableName, partitionColumns, filename, [delimiter], [schema], [skipRows=0], [transform])
    vector<ConstantSP> loadTextExInputArgs;
    {
        loadTextExInputArgs.insert(loadTextExInputArgs.end(),args.begin()+loadTextExBeginIndex,args.begin()+loadTextExBeginIndex+3);
        loadTextExInputArgs.push_back(nullptr);
        loadTextExInputArgs.insert(loadTextExInputArgs.end(),args.begin()+loadTextExBeginIndex+3,args.end());
    }
    
    vector<string> colObject;
    vector<string> colMsg;
    vector<int> colError;
    std::string tempFolder;
    {
        static std::atomic<long long> lastTmpFileIndex(Util::getEpochTime());
#ifdef __linux__
        tempFolder = "/tmp/DDB_S3Plugin_loadS3Object_"+std::to_string(lastTmpFileIndex.fetch_add(1));
#else
        tempFolder = Util::getExecutableDirectory() + "/DDB_S3Plugin_loadS3Object_"+std::to_string(lastTmpFileIndex.fetch_add(1));
#endif
        std::string msg;
        if(!Util::createDirectory(tempFolder, msg)){
            throw RuntimeException("Create temp directory "+tempFolder+" error "+msg);
        }
    }
    struct ObjectFile{
        string filePath;
        string objectInfo;
    };
    std::vector<std::thread> threads(threadCount);
    //load text thread
    SynchronizedQueue<ObjectFile> fileQueue;
    std::mutex objectMutex;
    Thread loadTextThread(new Executor([&]() {
        ObjectFile objectFile;
        vector<ConstantSP> loadTextExArgs(loadTextExInputArgs);
        loadTextExArgs[3] = new String;
        ConstantSP result;
        string msg;
        while(true){
            fileQueue.blockingPop(objectFile);
            if(objectFile.filePath.empty()) {
                break;
            }
            loadTextExArgs[3]->setString(objectFile.filePath);
            msg.clear();
            try{
                result = loadTextExFunc->call(heap, loadTextExArgs);
            }catch(std::exception &e){
                msg=e.what();
            }catch(...){
                msg="Unknow exception";
            }
            {//insert errorcode
                std::lock_guard<std::mutex> _(objectMutex);
                colObject.push_back(objectFile.objectInfo);
                int errCode = 0;
                if(!msg.empty()){
                    errCode=2;
                }
                colError.push_back(errCode);
                colMsg.push_back(msg);
            }
        }
    }));
    loadTextThread.start();
    /*std::thread loadTextThread = std::thread(;*/
    int objectIndex=0;
    try{//start download thread
        for(auto &thread : threads){
            thread = std::thread([&]() {
                s3_client client = leaseS3Client(s3account);
                std::string object;
                std::string objectFileName;
                std::string outputFilePath;
                std::string unzipFolder;
                string errMsg;
                int errCode = 0;
                while(true){
                    {//get next key in srcPaths
                        std::lock_guard<std::mutex> _(objectMutex);
                        if(objectIndex >= static_cast<int>(objects.size())) {
                            break;
                        }
                        object = objects[objectIndex++];
                    }
                    errMsg.clear();
                    errCode = 1;
                    {
                        objectFileName = object;
                        objectFileName = Util::replace(objectFileName,'/','-');
                        objectFileName = Util::replace(objectFileName,'\\','-');
                        objectFileName = Util::replace(objectFileName,"--","-");
                    }
                    outputFilePath = tempFolder + "/" + objectFileName;
                    try{
                        auto response = [&outputFilePath](){
                            return Aws::New<Aws::FStream>("FStream to download file", outputFilePath.c_str(),
                                                          std::ios_base::out | std::ios_base::binary);
                        };
                        s3_get_object_options getOptions;
                        getOptions.response_stream_factory = response;
                        auto outcome = client.get_object({bucketName, object}, getOptions);
                        if (!outcome.IsSuccess()) {
                            errCode = 3;
                            checkOutcome(outcome, "get object");
                        }
                        if(object.find(".zip")!=string::npos||
                            object.find(".ZIP")!=string::npos){
                            unzipFolder = outputFilePath + "_";
                            string cmd="unzip "+outputFilePath+" -d "+unzipFolder;
                            if(system(cmd.data()) != 0){
                                errCode = 4;
                                throw RuntimeException("unzip "+object+" failed, please install unzip package or check file format.");
                            }
                            string msg;
                            Util::removeFile(outputFilePath, msg);
                            std::function<void(const string &,const string &)> processDir=[&](const string &dir, const string &objectInfo){
                                vector<FileAttributes> files;
                                if(!Util::getDirectoryContent(dir, files, msg)){
                                    errCode = 5;
                                    throw IOException("Find unzip files in "+dir+" error "+msg);
                                }
                                string dirPath = dir + "/";
                                string object= objectInfo + "/";
                                for(auto &file : files){
                                    if(!file.isDir){
                                        fileQueue.push(ObjectFile{dirPath + file.name, object + file.name});
                                    }else{
                                        processDir(dirPath + file.name, object + file.name);
                                    }
                                }
                            };
                            processDir(unzipFolder, object);
                        }else{
                            fileQueue.push(ObjectFile{outputFilePath, object});
                        }
                        errCode = 0;
                    }catch(std::exception &e){
                        if(errCode == 0) {
                            errCode=6;
                        }
                        errMsg = e.what();
                    }catch(...){
                        errCode = 7;
                        errMsg = "Unknown exception.";
                    }
                    if(errCode != 0){
                        std::lock_guard<std::mutex> _(objectMutex);
                        colObject.push_back(object);
                        colMsg.push_back(errMsg);
                        colError.push_back(errCode);
                    }
                }
            });
        }
    } catch(std::exception &e){
        throw RuntimeException(AWSS3_PLUGIN_PREFIX+": creating thread error:"+e.what());
    }
    try{//do some clean job
        for(auto &one : threads){
            one.join();
        }
        fileQueue.push(ObjectFile());
        loadTextThread.join();
        string msg;
        if(!Util::removeDirectoryRecursive(tempFolder, msg)){
            LOG_ERR("remove dir content failed ",msg);
        }
    } catch(std::exception &e){
        throw RuntimeException(AWSS3_PLUGIN_PREFIX+": join thread error:"+e.what());
    }
    TableSP resultTable = Util::createTable({"object","errorCode","errorInfo"},{DT_STRING,DT_INT,DT_STRING}, colObject.size(), 0);
    ((Vector*)resultTable->getColumn(0).get())->setString(0, colObject.size(), colObject.data());
    ((Vector*)resultTable->getColumn(1).get())->setInt(0, colError.size(), colError.data());
    ((Vector*)resultTable->getColumn(2).get())->setString(0, colMsg.size(), colMsg.data());
    return resultTable;
}
