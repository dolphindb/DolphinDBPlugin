#pragma once

#include <aws/core/utils/memory/stl/AWSStreamFwd.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/S3ServiceClientModel.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#if __cplusplus >= 201703L
#include <optional>
#else
#include "ddbplugin/FixSTL.h"
#endif

struct ListObjectsOptions {
    std::optional<std::string> marker;
    std::optional<std::string> delimiter;
    std::optional<int> limit;
};

struct s3_object {
    std::string bucket;
    std::string key;
};

struct s3_upload_object {
    std::string key;
    std::string input_file;
};

struct s3_copy_object {
    std::string src;
    std::string dest;
};

struct s3_object_range {
    int64_t offset;
    int64_t length;
};

struct s3_get_object_options {
    std::optional<Aws::IOStreamFactory> response_stream_factory;
    std::optional<s3_object_range> range;
};

class s3_client
{
    using string = std::string;

  public:
    s3_client(std::shared_ptr<Aws::S3::S3Client> client, Aws::S3::S3ClientConfiguration config);

    auto list_buckets() const -> Aws::S3::Model::ListBucketsOutcome;
    auto list_objects(const string &bucket_name, const string &prefix, const ListObjectsOptions &options) const
        -> Aws::S3::Model::ListObjectsOutcome;
    auto head_object(const s3_object &object) const -> Aws::S3::Model::HeadObjectOutcome;
    auto get_object(const s3_object &object, const s3_get_object_options &options = {}) const
        -> Aws::S3::Model::GetObjectOutcome;

    auto create_bucket(const string &bucket_name) const -> Aws::S3::Model::CreateBucketOutcome;
    auto put_object(const string &bucket_name, const std::vector<s3_upload_object> &objects) const
        -> std::vector<Aws::S3::Model::PutObjectOutcomeCallable>;
    auto copy_object(const string &bucket_name, const std::vector<s3_copy_object> &objects) const
        -> std::vector<Aws::S3::Model::CopyObjectOutcomeCallable>;

    auto delete_bucket(const string &bucket_name) const -> Aws::S3::Model::DeleteBucketOutcome;
    auto delete_object(const string &bucket_name, const std::vector<string> &key_names) const
        -> std::vector<Aws::S3::Model::DeleteObjectOutcome>;
    auto config() const -> const Aws::S3::S3ClientConfiguration& { return config_; }

  private:
    std::shared_ptr<Aws::S3::S3Client> client_;
    Aws::S3::S3ClientConfiguration config_;
};
