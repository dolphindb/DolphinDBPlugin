/*
 * AWSS3.cpp
 *
 *  Created on: May 2, 2018
 *      Author: jccai
 */

#include "AWSS3.h"
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
#include <aws/s3/model/Object.h>
#include <aws/s3/model/ObjectStorageClass.h>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>

void setS3account(DictionarySP& account, Aws::Auth::AWSCredentials& credential, Aws::Client::ClientConfiguration& config) {
    Aws::String keyId;
    Aws::String secretKey;
    Aws::String region;
    Aws::String caPath;
    Aws::String caFile;
    bool verifySSL = false;
    try {
        keyId = Aws::String((account->getMember("id")->getString().c_str()));
        secretKey = Aws::String((account->getMember("key")->getString().c_str()));
        region = Aws::String((account->getMember("region")->getString().c_str()));
        credential.SetAWSAccessKeyId(keyId);
        credential.SetAWSSecretKey(secretKey);
        config.region = region;
        try {
            caPath = Aws::String(account->getMember("caPath")->getString().c_str());
            caFile = Aws::String(account->getMember("caFile")->getString().c_str());
            verifySSL = (bool)(account->getMember("verifySSL")->getBool());
            config.verifySSL = verifySSL;
            config.caPath = caPath;
            config.caFile = caFile;
        } catch(...) {}
    } catch(...) {
        throw IllegalArgumentException("S3account", 
            "s3account should have id, key, region");
    }
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
    Aws::Utils::Logging::InitializeAWSLogging(
        Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
            "AWS Logging",Aws::Utils::Logging::LogLevel::Trace,"aws_sdk_"));
    Aws::SDKOptions options;
    Aws::InitAPI(options);
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
        Aws::Auth::AWSCredentials credential;
        Aws::Client::ClientConfiguration config;
        setS3account(s3account, credential, config);
        Aws::S3::S3Client s3Client(credential, config);
        Aws::S3::Model::GetObjectRequest objectRequest;
        objectRequest.WithBucket(bucketName).WithKey(keyName)
                .SetResponseStreamFactory([&outputFileName](){
                    return Aws::New<Aws::FStream>("FStream to download file", outputFileName.c_str(), std::ios_base::out); });
        auto getObjectOutcome = s3Client.GetObject(objectRequest);
        if (!getObjectOutcome.IsSuccess()) {
            Aws::String error = "getS3Object cannot get object:\n" +
                getObjectOutcome.GetError().GetExceptionName() + "\n" +
                getObjectOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
        ret->setString(outputFileName.c_str());
    }
    Aws::ShutdownAPI(options);
    Aws::Utils::Logging::ShutdownAWSLogging();
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

    Aws::Utils::Logging::InitializeAWSLogging(
        Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
            "AWS Logging",Aws::Utils::Logging::LogLevel::Trace,"aws_sdk_"));
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
       
        const Aws::String bucketName(args[1]->getString().c_str());
        const Aws::String prefixName(args[2]->getString().c_str());

        DictionarySP s3account = (DictionarySP)args[0];
        Aws::Auth::AWSCredentials credential;
        Aws::Client::ClientConfiguration config;
        setS3account(s3account, credential, config);
        Aws::S3::S3Client s3Client(credential, config);
        Aws::S3::Model::ListObjectsRequest objectsRequest;
        objectsRequest.WithBucket(bucketName).WithPrefix(prefixName);

        auto listObjectsOutcome = s3Client.ListObjects(objectsRequest);
        if (listObjectsOutcome.IsSuccess()) {
            Aws::Vector<Aws::S3::Model::Object> objectList =
                listObjectsOutcome.GetResult().GetContents();
            long long i = 1;
            for (auto const &s3Object : objectList) {
                tblIdx.emplace_back(i++);
                tblBN.emplace_back(args[1]->getString().c_str());
                tblKN.emplace_back(std::string(s3Object.GetKey().c_str()));
                tblLM.emplace_back(std::string(s3Object.GetLastModified().ToLocalTimeString(Aws::Utils::DateFormat::ISO_8601).c_str()));
                tblLen.emplace_back(s3Object.GetSize());
                tblET.emplace_back(std::string(s3Object.GetETag().c_str()));
                tblOwner.emplace_back(std::string(s3Object.GetOwner().GetID().c_str()));
            }
        }
        else {
            Aws::String error = "listS3Object cannot list objects:\n" +
                listObjectsOutcome.GetError().GetExceptionName() + "\n" +
                listObjectsOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
    }
    Aws::ShutdownAPI(options);
    Aws::Utils::Logging::ShutdownAWSLogging();
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
    Aws::Utils::Logging::InitializeAWSLogging(
        Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
            "AWS Logging",Aws::Utils::Logging::LogLevel::Trace,"aws_sdk_"));
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        const Aws::String bucketName(args[1]->getString().c_str());
        const Aws::String keyName(args[2]->getString().c_str());
        Aws::StringStream range;
        range << "bytes=" << off << "-" << (off + len - 1);
        Aws::String objectRange(range.str());
        DictionarySP s3account = (DictionarySP)args[0];
        Aws::Auth::AWSCredentials credential;
        Aws::Client::ClientConfiguration config;
        setS3account(s3account, credential, config);
        Aws::S3::S3Client s3Client(credential, config);
        Aws::S3::Model::GetObjectRequest objectRequest;
        objectRequest.WithBucket(bucketName).WithKey(keyName).WithRange(objectRange);
        auto readObjectOutcome = s3Client.GetObject(objectRequest);
        if (readObjectOutcome.IsSuccess()) {
            std::stringstream buf;
            buf << readObjectOutcome.GetResult().GetBody().rdbuf();
            std::string tempFile(buf.str());
            ret->appendChar(const_cast<char *>(tempFile.c_str()), tempFile.size());
        }
        else {
            Aws::String error = "readS3Object cannot read object:\n" +
                readObjectOutcome.GetError().GetExceptionName() + "\n" +
                readObjectOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
    }
    Aws::ShutdownAPI(options);
    Aws::Utils::Logging::ShutdownAWSLogging();
    return ret;
}

void deleteS3Object(Heap* heap, vector<ConstantSP>& args) {
    if(!args[0]->isDictionary()) {
        throw IllegalArgumentException("deleteS3Object",
            "Invalid argument type, s3account should be a dictionary.");
    }
    if(args[1]->getType() != DT_STRING || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException("deleteS3Object",
            "Invalid argument type, bucket or key should be a string");
    }

    Aws::Utils::Logging::InitializeAWSLogging(
        Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
            "AWS Logging",Aws::Utils::Logging::LogLevel::Trace,"aws_sdk_"));
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        const Aws::String bucketName(args[1]->getString().c_str());
        const Aws::String keyName(args[2]->getString().c_str());

        DictionarySP s3account = (DictionarySP)args[0];
        Aws::Auth::AWSCredentials credential;
        Aws::Client::ClientConfiguration config;
        setS3account(s3account, credential, config);
        Aws::S3::S3Client s3Client(credential, config);
        Aws::S3::Model::DeleteObjectRequest objectRequest;
        objectRequest.WithBucket(bucketName).WithKey(keyName);
        auto deleteObjectOutcome = s3Client.DeleteObject(objectRequest);
        if (!deleteObjectOutcome.IsSuccess()) {
            Aws::String error = "deleteS3Object cannot delete object:\n" +
                deleteObjectOutcome.GetError().GetExceptionName() + "\n" +
                deleteObjectOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
    }
    Aws::ShutdownAPI(options);
    Aws::Utils::Logging::ShutdownAWSLogging();
}

void uploadS3Object(Heap* heap, vector<ConstantSP>& args) {
    if(!args[0]->isDictionary()) {
        throw IllegalArgumentException("uploadS3Object",
            "Invalid argument type, s3account should be a dictionary.");
    }
    if(args[1]->getType() != DT_STRING || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException("uploadS3Object",
            "Invalid argument type, bucket or key should be a string");
    }
    if(args[3]->getType() != DT_STRING) {
        throw IllegalArgumentException("uploadS3Object",
            "Invalid argument type, input file name should be a string");
    }

    Aws::Utils::Logging::InitializeAWSLogging(
        Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
            "AWS Logging",Aws::Utils::Logging::LogLevel::Trace,"aws_sdk_"));
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        const Aws::String bucketName(args[1]->getString().c_str());
        const Aws::String keyName(args[2]->getString().c_str());
        const Aws::String inputFileName(args[3]->getString().c_str());
        DictionarySP s3account = (DictionarySP)args[0];
        Aws::Auth::AWSCredentials credential;
        Aws::Client::ClientConfiguration config;
        setS3account(s3account, credential, config);
        Aws::S3::S3Client s3Client(credential, config);
        Aws::S3::Model::PutObjectRequest objectRequest;
        objectRequest.WithBucket(bucketName).WithKey(keyName);
        auto inputData = Aws::MakeShared<Aws::FStream>("PutObjectInputStream",
            inputFileName.c_str(), std::ios_base::in | std::ios_base::binary);

        objectRequest.SetBody(inputData);
        auto uploadObjectOutcome = s3Client.PutObject(objectRequest);
        if (!uploadObjectOutcome.IsSuccess()) {
            Aws::String error = "uploadS3Object cannot upload object:\n" +
                uploadObjectOutcome.GetError().GetExceptionName() + "\n" +
                uploadObjectOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
    }
    Aws::ShutdownAPI(options);
    Aws::Utils::Logging::ShutdownAWSLogging();
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
    Aws::Utils::Logging::InitializeAWSLogging(
        Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
            "AWS Logging",Aws::Utils::Logging::LogLevel::Trace,"aws_sdk_"));
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        DictionarySP s3account = (DictionarySP)args[0];
        Aws::Auth::AWSCredentials credential;
        Aws::Client::ClientConfiguration config;
        setS3account(s3account, credential, config);
        Aws::S3::S3Client s3Client(credential, config);

        auto listBucketsOutcome = s3Client.ListBuckets();
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
    Aws::ShutdownAPI(options);
    Aws::Utils::Logging::ShutdownAWSLogging();
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

    Aws::Utils::Logging::InitializeAWSLogging(
        Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
            "AWS Logging",Aws::Utils::Logging::LogLevel::Trace,"aws_sdk_"));
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        const Aws::String bucketName(args[1]->getString().c_str());

        DictionarySP s3account = (DictionarySP)args[0];
        Aws::Auth::AWSCredentials credential;
        Aws::Client::ClientConfiguration config;
        setS3account(s3account, credential, config);
        Aws::S3::S3Client s3Client(credential, config);
        Aws::S3::Model::DeleteBucketRequest BucketRequest;
        BucketRequest.SetBucket(bucketName);
        auto deleteBucketOutcome = s3Client.DeleteBucket(BucketRequest);
        if (!deleteBucketOutcome.IsSuccess()) {
            Aws::String error = "deleteS3Bucket cannot delete bucket:\n" +
                deleteBucketOutcome.GetError().GetExceptionName() + "\n" +
                deleteBucketOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
    }
    Aws::ShutdownAPI(options);
    Aws::Utils::Logging::ShutdownAWSLogging();
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

    Aws::Utils::Logging::InitializeAWSLogging(
        Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
            "AWS Logging",Aws::Utils::Logging::LogLevel::Trace,"aws_sdk_"));
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        const Aws::String bucketName(args[1]->getString().c_str());

        DictionarySP s3account = (DictionarySP)args[0];
        Aws::Auth::AWSCredentials credential;
        Aws::Client::ClientConfiguration config;
        setS3account(s3account, credential, config);
        Aws::S3::S3Client s3Client(credential, config);
        Aws::S3::Model::CreateBucketRequest BucketRequest;
        auto locationConstraint = Aws::S3::Model::BucketLocationConstraintMapper::
            GetBucketLocationConstraintForName(s3account->getMember("region")->getString().c_str());
        BucketRequest.WithBucket(bucketName).WithCreateBucketConfiguration(
            Aws::S3::Model::CreateBucketConfiguration().WithLocationConstraint(locationConstraint));
        auto createBucketOutcome = s3Client.CreateBucket(BucketRequest);
        if (!createBucketOutcome.IsSuccess()) {
            Aws::String error = "createS3Bucket cannot create bucket:\n" +
                createBucketOutcome.GetError().GetExceptionName() + "\n" +
                createBucketOutcome.GetError().GetMessage() + "\n";
            throw IOException(std::string(error.c_str()));
        }
    }
    Aws::ShutdownAPI(options);
    Aws::Utils::Logging::ShutdownAWSLogging();
}

// ConstantSP createS3InputStream(Heap* heap, vector<ConstantSP>& args) {
//     if(!args[0]->isDictionary()) {
//         throw IllegalArgumentException("createS3InputStream",
//                                        "Invalid argument type, s3account should be a dictionary.");
//     }
//     if(args[1]->getType() != DT_STRING || args[2]->getType() != DT_STRING) {
//         throw IllegalArgumentException("createS3InputStream",
//                                        "Invalid argument type, bucket or key should be a string");
//     }
//     Aws::SDKOptions options;
//     Aws::InitAPI(options);
//     const Aws::String bucketName(args[1]->getString().c_str());
//     const Aws::String keyName(args[2]->getString().c_str());
//     DictionarySP s3account = (DictionarySP)args[0];
//     Aws::Auth::AWSCredentials credential;
//     Aws::Client::ClientConfiguration config;
//     setS3account(s3account, credential, config);
//     Aws::S3::S3Client s3Client(credential, config);
//     Aws::S3::Model::GetObjectRequest objectRequest;
//     objectRequest.WithBucket(bucketName).WithKey(keyName);

//     DataInputStreamSP ret = new S3InputStream(s3Client, objectRequest, options);
//     return ret;
// }

// S3InputStream::S3InputStream(Aws::S3::S3Client s3Client, Aws::S3::Model::GetObjectRequest objectRequest, Aws::SDKOptions options)
//         : DataInputStream(FILE_STREAM), s3Client_(s3Client), objectRequest_(objectRequest), options_(options)  {
//     Aws::Utils::Logging::InitializeAWSLogging(
//             Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
//                     "AWS Logging",Aws::Utils::Logging::LogLevel::Trace,"aws_sdk_"));
//     if(objectRequest_.GetKey().find(".gz") != Aws::String::npos)
//     {
//         zStruct = new zlibStruct;
//         zStruct->strm.zalloc = Z_NULL;
//         zStruct->strm.zfree = Z_NULL;
//         zStruct->strm.opaque = Z_NULL;
//         zStruct->strm.avail_in = 0;
//         zStruct->strm.next_in = Z_NULL;
//         int ret = inflateInit2(&zStruct->strm, 15 + 16);  // windowsBits = 15
//         if (ret != Z_OK) {
//             throw IOException("zlib: decompress init error.");
//         }
//     }
// }

// IO_ERR S3InputStream::internalStreamRead(char* buf, size_t length, size_t& actualLength) {
//     if (zStruct == nullptr)
//         return fileStream(buf, length, actualLength);
//     else
//         return fileStreamZlib(buf, length, actualLength);
// }

// IO_ERR S3InputStream::internalClose() {
//     Aws::ShutdownAPI(options_);
//     Aws::Utils::Logging::ShutdownAWSLogging();
//     if(zStruct != nullptr) {
//         (void)inflateEnd(&zStruct->strm);
//         delete zStruct;
//     }

// }

// IO_ERR S3InputStream::fileStream(char *buf, size_t length, size_t& actualLength) {
//     actualLength = 0;
//     if(eof_)
//         return END_OF_STREAM;
//     if(err_)
//         return CORRUPT;
//     Aws::StringStream range;
//     range << "bytes=" << begin_ << "-" << (begin_ + length - 1);
//     objectRequest_.WithRange(range.str());
//     auto readObjectOutcome = s3Client_.GetObject(objectRequest_);
//     if (readObjectOutcome.IsSuccess()) {
//         auto &download = readObjectOutcome.GetResult().GetBody();
//         download.read(buf, length);
//         actualLength = download.gcount();
//         begin_ += actualLength;
//         if(download.eof())
//         {
//             eof_ = true;
//             return END_OF_STREAM;
//         }
//     }
//     else {
//         err_ = true;
//         Aws::String error = "deleteS3Bucket cannot delete bucket:\n" +
//                 readObjectOutcome.GetError().GetExceptionName() + "\n" +
//                 readObjectOutcome.GetError().GetMessage() + "\n";
//         throw IOException(std::string(error.c_str()));
//     }
//     return OK;
// }
// IO_ERR S3InputStream::fileStreamZlib(char *buf, size_t length, size_t& actualLength) {
//     int ret;
//     actualLength = 0;
//     if(eof_) {
//         ret = Z_STREAM_END;
//         goto Result;
//     }
//     else if(!eof_ && zStruct->buffer.in_avail() >= length) {
//         ret = Z_OK;
//         goto Result;
//     }
//     else {
//         while(!eof_ && zStruct->buffer.in_avail() < length) {
//             size_t gcount = 0;
//             fileStream(reinterpret_cast<char *>(zStruct->in), CHUNK, gcount);
//             //src_.read(reinterpret_cast<char *>(zStruct->in), CHUNK);
//             zStruct->strm.avail_in = gcount;
//             if (err_) {
//                 ret = Z_ERRNO;
//                 goto Result;
//             }
//             zStruct->strm.next_in = zStruct->in;
//             do {
//                 zStruct->strm.avail_out = CHUNK;
//                 zStruct->strm.next_out = zStruct->out;
//                 ret = inflate(&zStruct->strm, eof_ ? Z_FINISH : Z_NO_FLUSH);
//                 if(ret != Z_STREAM_END && ret != Z_OK) {
//                     ret = Z_ERRNO;
//                     goto Result;
//                 }
//                 zStruct->have = CHUNK - zStruct->strm.avail_out;
//                 zStruct->buffer.sputn(reinterpret_cast<const char*>(zStruct->out), zStruct->have);
//             } while(zStruct->strm.avail_out == 0);
//             if (eof_) {
//                 ret = Z_STREAM_END;
//                 goto Result;
//             }
//             if (zStruct->buffer.in_avail() >= length) {
//                 ret = Z_OK;
//                 break;
//             }
//         }
//     }
//     Result:
//     if(ret == Z_OK) {
//         actualLength = length;
//         zStruct->buffer.sgetn(buf, actualLength);
//         return OK;
//     }
//     else if (ret == Z_STREAM_END) {
//         actualLength = std::min(length, zStruct->buffer.in_avail());
//         zStruct->buffer.sgetn(buf, actualLength);
//         return zStruct->buffer.in_avail() ? OK : END_OF_STREAM;
//     }
//     else {
//         return CORRUPT;
//     }
// }
