#include "s3_client.h"

#include "aws/core/utils/memory/stl/AWSStreamFwd.h"
#include <aws/s3/S3Client.h>
#include <aws/s3/S3ServiceClientModel.h>
#include <aws/s3/model/BucketLocationConstraint.h>
#include <aws/s3/model/CreateBucketConfiguration.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/CopyObjectRequest.h>
#include <aws/s3/model/DeleteBucketRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

#include <fstream>
#include <string>
#include <utility>
#include <vector>

#if __cplusplus >= 201703L
#include <optional>
#else
#include "ddbplugin/FixSTL.h"
#endif

using namespace Aws::S3::Model; // NOLINT(google-build-using-namespace)
using std::string;
using std::vector;

s3_client::s3_client(std::shared_ptr<Aws::S3::S3Client> client, Aws::S3::S3ClientConfiguration config)
    : client_(std::move(client)), config_(std::move(config))
{
}

auto s3_client::get_object(const s3_object &object, const s3_get_object_options &options) const -> GetObjectOutcome
{
    GetObjectRequest request;
    request.SetBucket(object.bucket);
    request.SetKey(object.key);
    if (options.response_stream_factory) {
        request.SetResponseStreamFactory(*options.response_stream_factory);
    }
    if (options.range) {
        request.SetRange("bytes=" + std::to_string(options.range->offset) + "-" +
                         std::to_string(options.range->offset + options.range->length - 1));
    }
    return client_->GetObject(request);
}

auto s3_client::list_objects(const string &bucket_name, const string &prefix, const ListObjectsOptions &options) const
    -> ListObjectsOutcome
{
    ListObjectsRequest request;
    request.SetBucket(bucket_name);
    request.SetPrefix(prefix);
    if (options.marker) {
        request.SetMarker(*options.marker);
    }
    if (options.delimiter) {
        request.SetDelimiter(*options.delimiter);
    }
    if (options.limit) {
        request.SetMaxKeys(*options.limit);
    }
    return client_->ListObjects(request);
}

auto s3_client::delete_object(const string &bucket_name, const vector<string> &key_names) const
    -> vector<DeleteObjectOutcome>
{
    DeleteObjectRequest request;
    request.SetBucket(bucket_name);
    vector<DeleteObjectOutcome> outcomes;
    outcomes.reserve(key_names.size());
    for (const auto &key_name : key_names) {
        request.SetKey(key_name);
        outcomes.push_back(client_->DeleteObject(request));
    }
    return outcomes;
}

auto s3_client::put_object(const string &bucket_name, const vector<s3_upload_object> &objects) const
    -> vector<PutObjectOutcomeCallable>
{
    PutObjectRequest request;
    request.SetBucket(bucket_name);
    vector<PutObjectOutcomeCallable> futures;
    futures.reserve(objects.size());
    for (const auto &object : objects) {
        request.SetKey(object.key);
        auto input_data = Aws::MakeShared<Aws::FStream>("PutObjectInputStream", object.input_file.c_str(),
                                                        std::ios_base::in | std::ios_base::binary);
        request.SetBody(input_data);
        futures.push_back(client_->PutObjectCallable(request));
    }
    return futures;
}

auto s3_client::list_buckets() const -> ListBucketsOutcome
{
    return client_->ListBuckets();
}

auto s3_client::delete_bucket(const string &bucket_name) const -> DeleteBucketOutcome
{
    DeleteBucketRequest request;
    request.SetBucket(bucket_name);
    return client_->DeleteBucket(request);
}

auto s3_client::create_bucket(const string &bucket_name) const -> CreateBucketOutcome
{
    CreateBucketRequest request;
    auto location_constraint = BucketLocationConstraintMapper::GetBucketLocationConstraintForName(config_.region);
    request.WithBucket(bucket_name)
        .WithCreateBucketConfiguration(CreateBucketConfiguration().WithLocationConstraint(location_constraint));
    return client_->CreateBucket(request);
}

auto s3_client::head_object(const s3_object &object) const -> HeadObjectOutcome
{
    HeadObjectRequest request;
    request.SetBucket(object.bucket);
    request.SetKey(object.key);
    return client_->HeadObject(request);
}

auto s3_client::copy_object(const string &bucket_name, const vector<s3_copy_object> &objects) const
    -> vector<CopyObjectOutcomeCallable>
{
    CopyObjectRequest request;
    request.SetBucket(bucket_name);
    vector<CopyObjectOutcomeCallable> futures;
    futures.reserve(objects.size());
    for (const auto &object : objects) {
        request.SetKey(object.dest);
        request.SetCopySource(bucket_name + "/" + object.src);
        futures.push_back(client_->CopyObjectCallable(request));
    }
    return futures;
}
