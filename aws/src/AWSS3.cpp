/*
 * AWSS3.cpp
 *
 *  Created on: May 2, 2018
 *      Author: jccai
 */

#include "AWSS3.h"
#include "Concurrent.h"
#include "CoreConcept.h"
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/BucketLocationConstraint.h>
#include <aws/s3/model/CreateBucketConfiguration.h>
#include <aws/s3/model/DeleteBucketRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/DeleteObjectsRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/CopyObjectRequest.h>
#include <aws/s3/model/Object.h>
#include <aws/s3/model/ObjectStorageClass.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>

#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <cstdint>
#include <future>

void setS3account(DictionarySP& account, Aws::Auth::AWSCredentials& credential, Aws::Client::ClientConfiguration& config) {
    Aws::String keyId;
    Aws::String secretKey;
    Aws::String region;
    Aws::String caPath;
    Aws::String caFile;
    bool verifySSL = false;
    if (account->getMember("id")->isNull() || account->getMember("key")->isNull() || (account->getMember("region")->isNull())) {
        throw IllegalArgumentException("S3account", "s3account should have id, key, region");
    }
    keyId = Aws::String((account->getMember("id")->getString().c_str()));
    secretKey = Aws::String((account->getMember("key")->getString().c_str()));
    region = Aws::String((account->getMember("region")->getString().c_str()));
    credential.SetAWSAccessKeyId(keyId);
    credential.SetAWSSecretKey(secretKey);
    config.region = region;
    try {
        caPath = Aws::String(account->getMember("caPath")->getString().c_str());
        config.caPath = caPath;
        caFile = Aws::String(account->getMember("caFile")->getString().c_str());
        config.caFile = caFile;
        // auto ssl = account->getMember("verifySSL");
        config.verifySSL = false;
    } catch(...) {}
}

class S3InitGuard {
public:
    S3InitGuard(DictionarySP& account) {
        std::lock_guard<std::mutex> g{S3InitGuard::m_};
        LOG("[S3InitGuard] use count ", count_ + 1);
        try {
            if (S3InitGuard::count_++ == 0) {
                Init(account);
            } else {
                if (account.get() != S3InitGuard::init_dict_) {
                    RWLockGuard<RWLock> cli_g{&S3InitGuard::client_lk_, true};
                    client_ = newS3Client_(account);
                    init_dict_ = account.get();
                }
            }
        } catch(...) {
            LOG("[S3InitGuard] ctor exception: use count ", count_ - 1);
            if (--S3InitGuard::count_ == 0) {
                Shutdown();
            }
            throw;
        }
        S3InitGuard::client_lk_.acquireRead();
    }

    ~S3InitGuard() {
        S3InitGuard::client_lk_.releaseRead();
        {
            std::lock_guard<std::mutex> g{S3InitGuard::m_};
            LOG("[S3InitGuard] use count ", count_ - 1);
            if (--S3InitGuard::count_ == 0) {
                Shutdown();
            }
        }
    }

    static void Init(DictionarySP& account) {
        LOG("[S3InitGuard] start init");
        options_ = Aws::SDKOptions{};
        Aws::InitAPI(options_);
        Aws::Utils::Logging::InitializeAWSLogging(
            Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>("AWS Logging",Aws::Utils::Logging::LogLevel::Trace,"aws_sdk_"));
        RWLockGuard<RWLock> g{&S3InitGuard::client_lk_, true};
        client_ = newS3Client_(account);
        init_dict_ = account.get();
    }

    static void Shutdown() {
        LOG("[S3InitGuard] start shutdown");
        Aws::ShutdownAPI(options_);
        Aws::Utils::Logging::ShutdownAWSLogging();
        delete client_;
        client_ = nullptr;
    }

    static Aws::S3::S3Client* GetClient() {
        return client_;
    }
private:

    static std::mutex m_;
    static volatile size_t count_;
    static Aws::SDKOptions options_;
    static Aws::S3::S3Client* client_;
    static RWLock client_lk_;
    static Dictionary* init_dict_;

    static Aws::S3::S3Client* newS3Client_(DictionarySP s3account) {
        Aws::Auth::AWSCredentials credential;
        Aws::Client::ClientConfiguration config;
        setS3account(s3account, credential, config);
        return new Aws::S3::S3Client(credential, config);
    }
};

std::mutex S3InitGuard::m_;
size_t volatile S3InitGuard::count_ = 0;
Aws::SDKOptions S3InitGuard::options_;
Aws::S3::S3Client* S3InitGuard::client_ = nullptr;
RWLock S3InitGuard::client_lk_;
Dictionary* S3InitGuard::init_dict_ = nullptr;


template <typename R, typename E>
static const R& GetResult(const Aws::Utils::Outcome<R, E>& result, const Aws::String& errPrefix) {
    if (!result.IsSuccess()) {
        Aws::String error = errPrefix + ":\n" +
                result.GetError().GetExceptionName() + "\n" +
                result.GetError().GetMessage() + "\n";
        throw IOException(std::string(error.c_str()));
    } else {
        return result.GetResult();
    }
}

template <typename R, typename E>
static R&& GetResultWithOwnership(Aws::Utils::Outcome<R, E>& result, const Aws::String& errPrefix) {
    if (!result.IsSuccess()) {
        Aws::String error = errPrefix + ":\n" +
                result.GetError().GetExceptionName() + "\n" +
                result.GetError().GetMessage() + "\n";
        throw IOException(std::string(error.c_str()));
    } else {
        return result.GetResultWithOwnership();
    }
}

static void assertArg(bool cond, const string& funcName, const string& argName, const string& typeName) {
    if (!cond)
        throw IllegalArgumentException(funcName,
            "Invalid argument type, " + argName + " should be a " + typeName + ".");
}

static VectorSP retriveVecStrArg(ConstantSP arg) {
    if (arg->isVector()) {
        return arg;
    }
    VectorSP ret = Util::createVector(DT_ANY, 0);
    ret->append(arg);
    return ret;
}

ConstantSP getS3Object(Heap* heap, vector<ConstantSP>& args) {
    if(!args[0]->isDictionary()) {
        throw IllegalArgumentException("getS3Object",
            "Invalid argument type, s3account should be a dictionary.");
    }
    if(args[1]->getType() != DT_STRING || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException("getS3Object",
            "Invalid argument type, bucket or key should be a string");
    }

    ConstantSP ret = Util::createConstant(DT_STRING);
    {
        const Aws::String bucketName(args[1]->getString().c_str());
        const Aws::String keyName(args[2]->getString().c_str());
        Aws::String outputFileName = keyName;
        if(args.size() == 4) {
            if(args[3]->getType() != DT_STRING) {
                throw IllegalArgumentException("getS3Object",
                    "Invalid argument type, outputFileName should be a string");
            }
            outputFileName = Aws::String(args[3]->getString().c_str());
        }
        DictionarySP s3account = (DictionarySP)args[0];
        S3InitGuard g{s3account};
        Aws::S3::Model::GetObjectRequest objectRequest;
        objectRequest.WithBucket(bucketName).WithKey(keyName)
                .SetResponseStreamFactory([&outputFileName](){
                    return Aws::New<Aws::FStream>("FStream to download file", outputFileName.c_str(), std::ios_base::out); });
        auto getObjectOutcome = g.GetClient()->GetObject(objectRequest);
        if (!getObjectOutcome.IsSuccess()) {
            Aws::String error = "getS3Object cannot get object:\n" +
                getObjectOutcome.GetError().GetExceptionName() + "\n" +
                getObjectOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
        ret->setString(outputFileName.c_str());
    }
    return ret;
}

ConstantSP listS3Object(Heap* heap, vector<ConstantSP>& args) {
    if(!args[0]->isDictionary()) {
        throw IllegalArgumentException("listS3Object",
            "Invalid argument type, s3account should be a dictionary.");
    }
    if(args[1]->getType() != DT_STRING || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException("listS3Object",
            "Invalid argument type, bucket or prefix should be a string");
    }
    SmartPointer<String> marker;
    SmartPointer<String> delimiter;
    SmartPointer<String> nextMarker;
    SmartPointer<Long> limit;
    if (args.size() >= 4) {
        assertArg(args[3]->getType() == DT_STRING, __func__, "marker", "string");
        marker = args[3];
    }
    if (args.size() >= 5) {
        assertArg(args[4]->getType() == DT_STRING, __func__, "delimiter", "string");
        delimiter = args[4];
    }
    if (args.size() >= 6) {
        assertArg(args[5]->getType() == DT_STRING, __func__, "nextMarker", "string");
        nextMarker = args[5];
    }
    if (args.size() >= 7) {
        assertArg(args[6]->getType() == DT_LONG, __func__, "limit", "long");
        limit = args[6];
    }
    
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

    {
       
        const Aws::String bucketName(args[1]->getString().c_str());
        const Aws::String prefixName(args[2]->getString().c_str());

        DictionarySP s3account = (DictionarySP)args[0];
        S3InitGuard g{s3account};
        Aws::S3::Model::ListObjectsRequest objectsRequest;
        objectsRequest.WithBucket(bucketName).WithPrefix(prefixName);
        // ptr is not null and str is not empty
        if (!marker.isNull() && !marker->isNull()) {
            objectsRequest.WithMarker(marker->getString().c_str());
        }
        if (!delimiter.isNull() && !delimiter->isNull()) {
            objectsRequest.WithDelimiter(delimiter->getString().c_str());
        }
        if (!limit.isNull()) {
            objectsRequest.WithMaxKeys(limit->getLong());
        }

        auto listObjectsOutcome = g.GetClient()->ListObjects(objectsRequest);
        if (listObjectsOutcome.IsSuccess()) {
            Aws::Vector<Aws::S3::Model::Object> objectList =
                listObjectsOutcome.GetResult().GetContents();
            long long i = 1;
            // set nextmarker
            if (!nextMarker.isNull()) {
                if (listObjectsOutcome.GetResult().GetIsTruncated()) {
                    nextMarker->setString(listObjectsOutcome.GetResult().GetNextMarker().c_str());
                    if (nextMarker->isNull() && objectList.size() > 0) {
                        nextMarker->setString(objectList.back().GetKey().c_str());
                    }
                } else {
                    nextMarker->setString("");
                }
            }
            LOG("[listS3Object] marker ", marker.isNull() ? " " : marker->getString(), " turncated ", listObjectsOutcome.GetResult().GetIsTruncated());
            for (auto const &s3Object : objectList) {
                tblIdx.emplace_back(i++);
                tblBN.emplace_back(args[1]->getString().c_str());
                tblKN.emplace_back(std::string(s3Object.GetKey().c_str()));
                tblLM.emplace_back(std::string(s3Object.GetLastModified().ToLocalTimeString(Aws::Utils::DateFormat::ISO_8601).c_str()));
                tblLen.emplace_back(s3Object.GetSize());
                tblET.emplace_back(std::string(s3Object.GetETag().c_str()));
                tblOwner.emplace_back(std::string(s3Object.GetOwner().GetID().c_str()));
            }
            for (auto const& prefix: listObjectsOutcome.GetResult().GetCommonPrefixes()) {
                auto dirName = prefix.GetPrefix();
                tblIdx.emplace_back(i++);
                tblBN.emplace_back(args[1]->getString().c_str());
                tblKN.emplace_back(std::string(dirName.c_str()));
                tblLM.emplace_back("");
                tblLen.emplace_back(0);
                tblET.emplace_back(std::string(""));
                tblOwner.emplace_back(std::string(""));
            }
        }
        else {
            Aws::String error = "listS3Object cannot list objects:\n" +
                listObjectsOutcome.GetError().GetExceptionName() + "\n" +
                listObjectsOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
    }
    size_t rowNumber = tblIdx.size();
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
    if(!args[0]->isDictionary()) {
        throw IllegalArgumentException("readS3Object",
            "Invalid argument type, s3account should be a dictionary.");
    }
    if(args[1]->getType() != DT_STRING || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException("readS3Object",
            "Invalid argument type, bucket or key should be a string");
    }
    long long off = 0, len = 0;
    if((args[3]->getType() != DT_LONG && args[3]->getType() != DT_INT) ||
       (args[4]->getType() != DT_LONG && args[4]->getType() != DT_INT)) {
        throw IllegalArgumentException("readS3Object",
            "Invalid argument type, offset or length should be a int or long");
    }
    if(args[3]->getType() == DT_INT)
	off = args[3]->getInt();
    else
	off = args[3]->getLong();
    if(args[4]->getType() == DT_INT)
	len = args[4]->getInt();
    else
	len = args[4]->getLong();
    if(off < 0 || len <= 0) {
	throw IllegalArgumentException("readS3Object", "Invalid range, offset should >= 0, length should > 0.");
    }

    VectorSP ret = Util::createVector(DT_CHAR, 0);
    DictionarySP s3account = (DictionarySP)args[0];

    S3InitGuard g{s3account};
    {
        const Aws::String bucketName(args[1]->getString().c_str());
        const Aws::String keyName(args[2]->getString().c_str());
        Aws::StringStream range;
        range << "bytes=" << off << "-" << (off + len - 1);
        Aws::String objectRange(range.str());

        Aws::S3::Model::GetObjectRequest objectRequest;
        objectRequest.WithBucket(bucketName).WithKey(keyName).WithRange(objectRange);
        auto readObjectOutcome = g.GetClient()->GetObject(objectRequest);
        std::stringstream buf;
        buf << GetResultWithOwnership(readObjectOutcome, "readS3Object cannot read object").GetBody().rdbuf();
        std::string tempFile(buf.str());
        LOG_INFO("[readS3Object] got ", tempFile.size(), " bytes");
        ret->appendChar(const_cast<char *>(tempFile.c_str()), tempFile.size());
    }
    return ret;
}

void deleteS3Object(Heap* heap, vector<ConstantSP>& args) {
    if(!args[0]->isDictionary()) {
        throw IllegalArgumentException("deleteS3Object",
            "Invalid argument type, s3account should be a dictionary.");
    }
    if(args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException("deleteS3Object",
            "Invalid argument type, bucket should be a string");
    }
    assertArg(args[2]->getType() == DT_STRING || args[2]->isVector(), __func__, "key", "string");
    
    DictionarySP s3account = (DictionarySP)args[0];
    S3InitGuard g{s3account};
    const Aws::String bucketName(args[1]->getString().c_str());
    VectorSP keyNames = retriveVecStrArg(args[2]);
    std::vector<std::future<Aws::S3::Model::DeleteObjectsOutcome>> futures;
    Aws::S3::Model::DeleteObjectsRequest r;
    r.SetBucket(bucketName);
    for (int i = 0; i < keyNames->size(); ) {
        // DeleteObjects request delete 1000 objects at most
        int upper = std::min(i + 990, keyNames->size());
        Aws::Vector<Aws::S3::Model::ObjectIdentifier> objects;
        for (int j = i; j < upper; ++j) {
            objects.push_back(Aws::S3::Model::ObjectIdentifier().WithKey(keyNames->getString(j).c_str()));
        }
        r.SetDelete(Aws::S3::Model::Delete().WithObjects(std::move(objects)));
        futures.push_back(g.GetClient()->DeleteObjectsCallable(r));
        i = upper;
    }
    for (size_t i = 0; i < futures.size(); ++i) {
        futures[i].wait();
        if (!futures[i].valid()) {
            throw IOException(string(__func__) + string(" cannot delete object ") + keyNames->getString(i));
        }
        GetResult(futures[i].get(), Aws::String(__func__) + Aws::String(" cannot delete object ") + Aws::String(keyNames->getString(i).c_str()));
    }
}

void uploadS3Object(Heap* heap, vector<ConstantSP>& args) {
    if(!args[0]->isDictionary()) {
        throw IllegalArgumentException("uploadS3Object",
            "Invalid argument type, s3account should be a dictionary.");
    }
    if(args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException("uploadS3Object",
            "Invalid argument type, bucket should be a string");
    }
    if(args[2]->getType() != DT_STRING && !args[2]->isVector()) {
        throw IllegalArgumentException("uploadS3Object",
            "Invalid argument type, key should be a string");
    }
    if(args[3]->getType() != DT_STRING && !args[3]->isVector()) {
        throw IllegalArgumentException("uploadS3Object",
            "Invalid argument type, input file name should be a string ");
    }
    VectorSP keys;
    VectorSP from;
    if (args[2]->isVector()) {
        keys = args[2];
    } else {
        keys = Util::createVector(DT_STRING, 0);
        keys->append(args[2]);
    }

    if (args[3]->isVector()) {
        from = args[3];
    } else {
        from = Util::createVector(DT_STRING, 0);
        from->append(args[3]);
    }

    if (keys->size() != from->size()) {
        throw IllegalArgumentException(__func__, "Invalid argument type, input files and keys are mismatched");
    }
    
    DictionarySP s3account = (DictionarySP)args[0];
    S3InitGuard g{s3account};
    {
        const Aws::String bucketName(args[1]->getString().c_str());
        std::vector<Aws::S3::Model::PutObjectOutcomeCallable> futures;
        for (int i = 0; i < keys->size(); ++i) {
            const Aws::String keyName(keys->getString(i).c_str());
            const Aws::String inputFileName(from->getString(i).c_str());
            Aws::S3::Model::PutObjectRequest objectRequest;
            objectRequest.WithBucket(bucketName).WithKey(keyName);
            auto inputData = Aws::MakeShared<Aws::FStream>("PutObjectInputStream",
                inputFileName.c_str(), std::ios_base::in | std::ios_base::binary);
            objectRequest.SetBody(inputData);
            futures.push_back(g.GetClient()->PutObjectCallable(objectRequest));
        }
        for (size_t i = 0; i < futures.size(); ++i) {
            auto& future = futures[i];
            future.wait();
            if (!future.valid()) {
                throw IOException("uploadS3Object cannot upload object " + keys->getString(i));
            }
            GetResult(future.get(), Aws::String("uploadS3Object cannot upload object ") + Aws::String(keys->getString(i).c_str()));
        }
    }
}

ConstantSP listS3Bucket(Heap* heap, vector<ConstantSP>& args) {
    if(!args[0]->isDictionary()) {
        throw IllegalArgumentException("listS3Bucket",
            "Invalid argument type, s3account should be a dictionary.");
    }
    std::vector<std::string> colName{"index", "bucket name", "creation date"};
    std::vector<long long> tblIdx;
    VectorSP tableIndex = Util::createVector(DT_LONG, 0);
    std::vector<std::string> tblBN;
    VectorSP tableBucketName = Util::createVector(DT_STRING, 0);
    std::vector<std::string> tblCD;
    VectorSP tableCreationDate = Util::createVector(DT_STRING, 0);
    {
        DictionarySP s3account = (DictionarySP)args[0];
        S3InitGuard g{s3account};
        auto listBucketsOutcome = g.GetClient()->ListBuckets();
        if (listBucketsOutcome.IsSuccess()) {
            Aws::Vector<Aws::S3::Model::Bucket> bucketList =
                listBucketsOutcome.GetResult().GetBuckets();
            long long i = 1;
            for (auto const &s3Bucket : bucketList) {
                tblIdx.emplace_back(i++);
                tblBN.emplace_back(std::string(s3Bucket.GetName().c_str()));
                tblCD.emplace_back(std::string(s3Bucket.GetCreationDate().ToLocalTimeString(Aws::Utils::DateFormat::ISO_8601).c_str()));
            }
        }
        else {
            Aws::String error = "listS3Bucket cannot list buckets:\n" +
                listBucketsOutcome.GetError().GetExceptionName() + "\n" +
                listBucketsOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
    }
    size_t rowNumber = tblIdx.size();
    tableIndex->appendLong(tblIdx.data(), rowNumber);
    tableBucketName->appendString(tblBN.data(), rowNumber);
    tableCreationDate->appendString(tblCD.data(), rowNumber);
    return Util::createTable(colName, {tableIndex, tableBucketName, tableCreationDate});
}

void deleteS3Bucket(Heap* heap, vector<ConstantSP>& args) {
    if(!args[0]->isDictionary()) {
        throw IllegalArgumentException("deleteS3Bucket",
            "Invalid argument type, s3account should be a dictionary.");
    }
    if(args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException("deleteS3Bucket",
            "Invalid argument type, bucket should be a string");
    }

    {
        const Aws::String bucketName(args[1]->getString().c_str());
        DictionarySP s3account = (DictionarySP)args[0];
        S3InitGuard g{s3account};
        Aws::S3::Model::DeleteBucketRequest BucketRequest;
        BucketRequest.SetBucket(bucketName);
        auto deleteBucketOutcome = g.GetClient()->DeleteBucket(BucketRequest);
        if (!deleteBucketOutcome.IsSuccess()) {
            Aws::String error = "deleteS3Bucket cannot delete bucket:\n" +
                deleteBucketOutcome.GetError().GetExceptionName() + "\n" +
                deleteBucketOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
    }
}

void createS3Bucket(Heap* heap, vector<ConstantSP>& args) {
    if(!args[0]->isDictionary()) {
        throw IllegalArgumentException("createS3Bucket",
            "Invalid argument type, s3account should be a dictionary.");
    }
    if(args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException("createS3Bucket",
            "Invalid argument type, bucket should be a string");
    }

    {
        const Aws::String bucketName(args[1]->getString().c_str());
        DictionarySP s3account = (DictionarySP)args[0];
        S3InitGuard g{s3account};
        Aws::S3::Model::CreateBucketRequest BucketRequest;
        auto locationConstraint = Aws::S3::Model::BucketLocationConstraintMapper::
            GetBucketLocationConstraintForName(s3account->getMember("region")->getString().c_str());
        BucketRequest.WithBucket(bucketName).WithCreateBucketConfiguration(
            Aws::S3::Model::CreateBucketConfiguration().WithLocationConstraint(locationConstraint));
        auto createBucketOutcome = g.GetClient()->CreateBucket(BucketRequest);
        if (!createBucketOutcome.IsSuccess()) {
            Aws::String error = "createS3Bucket cannot create bucket:\n" +
                createBucketOutcome.GetError().GetExceptionName() + "\n" +
                createBucketOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
    }
}

ConstantSP headS3Object(Heap* heap, vector<ConstantSP>& args) {
    assertArg(args[0]->isDictionary(), "HeadS3Object", "s3account", "Dictionary");
    assertArg(args[1]->getType() == DT_STRING, "HeadS3Object", "bucketName", "string");
    assertArg(args[2]->getType() == DT_STRING, "HeadS3Object", "key", "string");
    const Aws::String bucketName(args[1]->getString().c_str());
    const Aws::String keyName(args[2]->getString().c_str());
    DictionarySP s3account = (DictionarySP)args[0];
    S3InitGuard g{s3account};

    Aws::S3::Model::HeadObjectRequest request;
    request.WithBucket(bucketName).WithKey(keyName);
    auto result = GetResult(g.GetClient()->HeadObject(request), "HeadS3Object cannot get object information");
    DictionarySP ret = Util::createDictionary(DT_STRING, SymbolBaseSP(), DT_ANY, SymbolBaseSP());
    ret->set("bucket name", new String(bucketName.c_str()));
    ret->set("key name", new String(keyName.c_str()));
    ret->set("length", new Long(result.GetContentLength()));
    ret->set("last modified", new String(result.GetLastModified().ToLocalTimeString(Aws::Utils::DateFormat::ISO_8601).c_str()));
    ret->set("ETag", new String(result.GetETag().c_str()));
    ret->set("content type", new String(result.GetContentType().c_str()));
    return ret;
}

void copyS3Object(Heap* heap, vector<ConstantSP>& args) {
    assertArg(args[0]->isDictionary(), __func__, "s3account", "Dictionary");
    assertArg(args[1]->getType() == DT_STRING, __func__, "bucketName", "string");
    assertArg(args[2]->getType() == DT_STRING || args[2]->isVector(), __func__, "srcPath", "string");
    assertArg(args[3]->getType() == DT_STRING || args[3]->isVector(), __func__, "destPath", "string");

    VectorSP srcPaths = retriveVecStrArg(args[2]);
    VectorSP destPaths = retriveVecStrArg(args[3]);
    if (srcPaths->size() != destPaths->size()) {
        throw IllegalArgumentException(__func__, " srcPath and destPath are mismatched");
    }
    const Aws::String bucket(args[1]->getString().c_str());
    DictionarySP s3account = (DictionarySP)args[0];
    S3InitGuard g{s3account};
    std::vector<Aws::S3::Model::CopyObjectOutcomeCallable> futures;
    for (int i = 0; i < srcPaths->size(); ++i) {
        const Aws::String srcPath(srcPaths->getString(i).c_str());
        const Aws::String destPath(destPaths->getString(i).c_str());
        Aws::S3::Model::CopyObjectRequest request;
        // details at: https://docs.aws.amazon.com/AmazonS3/latest/API/API_CopyObject.html
        request.WithBucket(bucket).WithKey(destPath).WithCopySource(bucket + "/" + srcPath);
        futures.push_back(g.GetClient()->CopyObjectCallable(request)); 
    }
    for (size_t i = 0; i < futures.size(); ++i) {
        futures[i].wait();
        if (!futures[i].valid()) {
            throw IOException(string(__func__) + string(" cannot copy object ") + srcPaths->getString(i));
        }
        GetResult(futures[i].get(), Aws::String(__func__) + Aws::String(" cannot copy object ") + Aws::String(srcPaths->getString(i).c_str()));
    }
}

#if 0
ConstantSP createS3InputStream(Heap* heap, vector<ConstantSP>& args) {
    if(!args[0]->isDictionary()) {
        throw IllegalArgumentException("createS3InputStream",
                                       "Invalid argument type, s3account should be a dictionary.");
    }
    if(args[1]->getType() != DT_STRING || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException("createS3InputStream",
                                       "Invalid argument type, bucket or key should be a string");
    }
    const Aws::String bucketName(args[1]->getString().c_str());
    const Aws::String keyName(args[2]->getString().c_str());
    DictionarySP s3account = (DictionarySP)args[0];
    Aws::Auth::AWSCredentials credential;
    Aws::Client::ClientConfiguration config;
    setS3account(s3account, credential, config);
    Aws::S3::S3Client s3Client(credential, config);
    Aws::S3::Model::GetObjectRequest objectRequest;
    objectRequest.WithBucket(bucketName).WithKey(keyName);

    DataInputStreamSP ret = new S3InputStream(s3Client, objectRequest, options);
    return ret;
}

S3InputStream::S3InputStream(Aws::S3::S3Client s3Client, Aws::S3::Model::GetObjectRequest objectRequest, Aws::SDKOptions options)
        : DataInputStream(FILE_STREAM), s3Client_(s3Client), objectRequest_(objectRequest), options_(options)  {
    Aws::Utils::Logging::InitializeAWSLogging(
            Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
                    "AWS Logging",Aws::Utils::Logging::LogLevel::Trace,"aws_sdk_"));
    if(objectRequest_.GetKey().find(".gz") != Aws::String::npos)
    {
        zStruct = new zlibStruct;
        zStruct->strm.zalloc = Z_NULL;
        zStruct->strm.zfree = Z_NULL;
        zStruct->strm.opaque = Z_NULL;
        zStruct->strm.avail_in = 0;
        zStruct->strm.next_in = Z_NULL;
        int ret = inflateInit2(&zStruct->strm, 15 + 16);  // windowsBits = 15
        if (ret != Z_OK) {
            throw IOException("zlib: decompress init error.");
        }
    }
}

IO_ERR S3InputStream::internalStreamRead(char* buf, size_t length, size_t& actualLength) {
    if (zStruct == nullptr)
        return fileStream(buf, length, actualLength);
    else
        return fileStreamZlib(buf, length, actualLength);
}

IO_ERR S3InputStream::internalClose() {
    Aws::ShutdownAPI(options_);
    Aws::Utils::Logging::ShutdownAWSLogging();
    if(zStruct != nullptr) {
        (void)inflateEnd(&zStruct->strm);
        delete zStruct;
    }

}

IO_ERR S3InputStream::fileStream(char *buf, size_t length, size_t& actualLength) {
    actualLength = 0;
    if(eof_)
        return END_OF_STREAM;
    if(err_)
        return CORRUPT;
    Aws::StringStream range;
    range << "bytes=" << begin_ << "-" << (begin_ + length - 1);
    objectRequest_.WithRange(range.str());
    auto readObjectOutcome = s3Client_.GetObject(objectRequest_);
    if (readObjectOutcome.IsSuccess()) {
        auto &download = readObjectOutcome.GetResult().GetBody();
        download.read(buf, length);
        actualLength = download.gcount();
        begin_ += actualLength;
        if(download.eof())
        {
            eof_ = true;
            return END_OF_STREAM;
        }
    }
    else {
        err_ = true;
        Aws::String error = "deleteS3Bucket cannot delete bucket:\n" +
                readObjectOutcome.GetError().GetExceptionName() + "\n" +
                readObjectOutcome.GetError().GetMessage() + "\n";
        throw IOException(std::string(error.c_str()));
    }
    return OK;
}
IO_ERR S3InputStream::fileStreamZlib(char *buf, size_t length, size_t& actualLength) {
    int ret;
    actualLength = 0;
    if(eof_) {
        ret = Z_STREAM_END;
        goto Result;
    }
    else if(!eof_ && zStruct->buffer.in_avail() >= length) {
        ret = Z_OK;
        goto Result;
    }
    else {
        while(!eof_ && zStruct->buffer.in_avail() < length) {
            size_t gcount = 0;
            fileStream(reinterpret_cast<char *>(zStruct->in), CHUNK, gcount);
            //src_.read(reinterpret_cast<char *>(zStruct->in), CHUNK);
            zStruct->strm.avail_in = gcount;
            if (err_) {
                ret = Z_ERRNO;
                goto Result;
            }
            zStruct->strm.next_in = zStruct->in;
            do {
                zStruct->strm.avail_out = CHUNK;
                zStruct->strm.next_out = zStruct->out;
                ret = inflate(&zStruct->strm, eof_ ? Z_FINISH : Z_NO_FLUSH);
                if(ret != Z_STREAM_END && ret != Z_OK) {
                    ret = Z_ERRNO;
                    goto Result;
                }
                zStruct->have = CHUNK - zStruct->strm.avail_out;
                zStruct->buffer.sputn(reinterpret_cast<const char*>(zStruct->out), zStruct->have);
            } while(zStruct->strm.avail_out == 0);
            if (eof_) {
                ret = Z_STREAM_END;
                goto Result;
            }
            if (zStruct->buffer.in_avail() >= length) {
                ret = Z_OK;
                break;
            }
        }
    }
    Result:
    if(ret == Z_OK) {
        actualLength = length;
        zStruct->buffer.sgetn(buf, actualLength);
        return OK;
    }
    else if (ret == Z_STREAM_END) {
        actualLength = std::min(length, zStruct->buffer.in_avail());
        zStruct->buffer.sgetn(buf, actualLength);
        return zStruct->buffer.in_avail() ? OK : END_OF_STREAM;
    }
    else {
        return CORRUPT;
    }
}
#endif

