// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// This boost/asio/io_context.hpp include is needless for no MinGW
// build.
//
// This is for including boost/asio/detail/socket_types.hpp before any
// "#include <windows.h>". boost/asio/detail/socket_types.hpp doesn't
// work if windows.h is already included. boost/process.h ->
// boost/process/args.hpp -> boost/process/detail/basic_cmd.hpp
// includes windows.h. boost/process/args.hpp is included before
// boost/process/async.h that includes
// boost/asio/detail/socket_types.hpp implicitly is included.
#include <boost/asio/io_context.hpp>
// We need BOOST_USE_WINDOWS_H definition with MinGW when we use
// boost/process.hpp. See ARROW_BOOST_PROCESS_COMPILE_DEFINITIONS in
// cpp/cmake_modules/BuildUtils.cmake for details.
#include <boost/process.hpp>

#include <gtest/gtest.h>

#ifdef _WIN32
// Undefine preprocessor macros that interfere with AWS function / method names
#ifdef GetMessage
#undef GetMessage
#endif
#ifdef GetObject
#undef GetObject
#endif
#endif

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/client/RetryStrategy.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/sts/STSClient.h>

#include "arrow/filesystem/filesystem.h"
#include "arrow/filesystem/s3_internal.h"
#include "arrow/filesystem/s3_test_util.h"
#include "arrow/filesystem/s3fs.h"
#include "arrow/filesystem/test_util.h"
#include "arrow/result.h"
#include "arrow/status.h"
#include "arrow/testing/gtest_util.h"
#include "arrow/testing/util.h"
#include "arrow/util/io_util.h"
#include "arrow/util/logging.h"
#include "arrow/util/macros.h"

namespace arrow {
namespace fs {

using ::arrow::internal::DelEnvVar;
using ::arrow::internal::PlatformFilename;
using ::arrow::internal::SetEnvVar;
using ::arrow::internal::UriEscape;

using ::arrow::fs::internal::ConnectRetryStrategy;
using ::arrow::fs::internal::ErrorToStatus;
using ::arrow::fs::internal::OutcomeToStatus;
using ::arrow::fs::internal::ToAwsString;

namespace bp = boost::process;

// NOTE: Connecting in Python:
// >>> fs = s3fs.S3FileSystem(key='minio', secret='miniopass',
// client_kwargs=dict(endpoint_url='http://127.0.0.1:9000'))
// >>> fs.ls('')
// ['bucket']
// or:
// >>> from fs_s3fs import S3FS
// >>> fs = S3FS('bucket', endpoint_url='http://127.0.0.1:9000',
// aws_access_key_id='minio', aws_secret_access_key='miniopass')

#define ARROW_AWS_ASSIGN_OR_FAIL_IMPL(outcome_name, lhs, rexpr) \
  auto outcome_name = (rexpr);                                  \
  if (!outcome_name.IsSuccess()) {                              \
    FAIL() << "'" ARROW_STRINGIFY(rexpr) "' failed with "       \
           << outcome_name.GetError().GetMessage();             \
  }                                                             \
  lhs = std::move(outcome_name).GetResultWithOwnership();

#define ARROW_AWS_ASSIGN_OR_FAIL_NAME(x, y) ARROW_CONCAT(x, y)

#define ARROW_AWS_ASSIGN_OR_FAIL(lhs, rexpr) \
  ARROW_AWS_ASSIGN_OR_FAIL_IMPL(             \
      ARROW_AWS_ASSIGN_OR_FAIL_NAME(_aws_error_or_value, __COUNTER__), lhs, rexpr);

class S3TestMixin : public ::testing::Test {
 public:
  void SetUp() override {
    ASSERT_OK(minio_.Start());

    client_config_.endpointOverride = ToAwsString(minio_.connect_string());
    client_config_.scheme = Aws::Http::Scheme::HTTP;
    client_config_.retryStrategy = std::make_shared<ConnectRetryStrategy>();
    credentials_ = {ToAwsString(minio_.access_key()), ToAwsString(minio_.secret_key())};
    bool use_virtual_addressing = false;
    client_.reset(
        new Aws::S3::S3Client(credentials_, client_config_,
                              Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never,
                              use_virtual_addressing));
  }

  void TearDown() override { ASSERT_OK(minio_.Stop()); }

 protected:
  MinioTestServer minio_;
  Aws::Client::ClientConfiguration client_config_;
  Aws::Auth::AWSCredentials credentials_;
  std::unique_ptr<Aws::S3::S3Client> client_;
};

void AssertGetObject(Aws::S3::Model::GetObjectResult& result,
                     const std::string& expected) {
  auto length = static_cast<int64_t>(expected.length());
  ASSERT_EQ(result.GetContentLength(), length);
  auto& stream = result.GetBody();
  std::string actual;
  actual.resize(length + 1);
  stream.read(&actual[0], length + 1);
  ASSERT_EQ(stream.gcount(), length);  // EOF was reached before length + 1
  actual.resize(length);
  ASSERT_EQ(actual.size(), expected.size());
  ASSERT_TRUE(actual == expected);  // Avoid ASSERT_EQ on large data
}

void AssertObjectContents(Aws::S3::S3Client* client, const std::string& bucket,
                          const std::string& key, const std::string& expected) {
  Aws::S3::Model::GetObjectRequest req;
  req.SetBucket(ToAwsString(bucket));
  req.SetKey(ToAwsString(key));
  ARROW_AWS_ASSIGN_OR_FAIL(auto result, client->GetObject(req));
  AssertGetObject(result, expected);
}

////////////////////////////////////////////////////////////////////////////
// S3Options tests

class S3OptionsTest : public ::testing::Test {
 public:
  void SetUp() {
    // we set this environment variable to speed up tests by ensuring
    // DefaultAWSCredentialsProviderChain does not query (inaccessible)
    // EC2 metadata endpoint
    ASSERT_OK(SetEnvVar("AWS_EC2_METADATA_DISABLED", "true"));
  }
  void TearDown() { ASSERT_OK(DelEnvVar("AWS_EC2_METADATA_DISABLED")); }
};

TEST_F(S3OptionsTest, FromUri) {
  std::string path;
  S3Options options;

  ASSERT_OK_AND_ASSIGN(options, S3Options::FromUri("s3://", &path));
  ASSERT_EQ(options.region, kS3DefaultRegion);
  ASSERT_EQ(options.scheme, "https");
  ASSERT_EQ(options.endpoint_override, "");
  ASSERT_EQ(path, "");

  ASSERT_OK_AND_ASSIGN(options, S3Options::FromUri("s3:", &path));
  ASSERT_EQ(path, "");

  ASSERT_OK_AND_ASSIGN(options, S3Options::FromUri("s3://access:secret@mybucket", &path));
  ASSERT_EQ(path, "mybucket");
  const auto creds = options.credentials_provider->GetAWSCredentials();
  ASSERT_EQ(creds.GetAWSAccessKeyId(), "access");
  ASSERT_EQ(creds.GetAWSSecretKey(), "secret");

  ASSERT_OK_AND_ASSIGN(options, S3Options::FromUri("s3://mybucket/", &path));
  ASSERT_EQ(options.region, kS3DefaultRegion);
  ASSERT_EQ(options.scheme, "https");
  ASSERT_EQ(options.endpoint_override, "");
  ASSERT_EQ(path, "mybucket");

  ASSERT_OK_AND_ASSIGN(options, S3Options::FromUri("s3://mybucket/foo/bar/", &path));
  ASSERT_EQ(options.region, kS3DefaultRegion);
  ASSERT_EQ(options.scheme, "https");
  ASSERT_EQ(options.endpoint_override, "");
  ASSERT_EQ(path, "mybucket/foo/bar");

  ASSERT_OK_AND_ASSIGN(
      options,
      S3Options::FromUri(
          "s3://mybucket/foo/bar/?region=utopia&endpoint_override=localhost&scheme=http",
          &path));
  ASSERT_EQ(options.region, "utopia");
  ASSERT_EQ(options.scheme, "http");
  ASSERT_EQ(options.endpoint_override, "localhost");
  ASSERT_EQ(path, "mybucket/foo/bar");

  // Missing bucket name
  ASSERT_RAISES(Invalid, S3Options::FromUri("s3:///foo/bar/", &path));
}

TEST_F(S3OptionsTest, FromAccessKey) {
  S3Options options;

  // session token is optional and should default to empty string
  options = S3Options::FromAccessKey("access", "secret");
  ASSERT_EQ(options.GetAccessKey(), "access");
  ASSERT_EQ(options.GetSecretKey(), "secret");
  ASSERT_EQ(options.GetSessionToken(), "");

  options = S3Options::FromAccessKey("access", "secret", "token");
  ASSERT_EQ(options.GetAccessKey(), "access");
  ASSERT_EQ(options.GetSecretKey(), "secret");
  ASSERT_EQ(options.GetSessionToken(), "token");
}

TEST_F(S3OptionsTest, FromAssumeRole) {
  S3Options options;

  // arn should be only required argument
  options = S3Options::FromAssumeRole("my_role_arn");
  options = S3Options::FromAssumeRole("my_role_arn", "session");
  options = S3Options::FromAssumeRole("my_role_arn", "session", "id");
  options = S3Options::FromAssumeRole("my_role_arn", "session", "id", 42);

  // test w/ custom STSClient (will not use DefaultAWSCredentialsProviderChain)
  Aws::Auth::AWSCredentials test_creds = Aws::Auth::AWSCredentials("access", "secret");
  std::shared_ptr<Aws::STS::STSClient> sts_client =
      std::make_shared<Aws::STS::STSClient>(Aws::STS::STSClient(test_creds));
  options = S3Options::FromAssumeRole("my_role_arn", "session", "id", 42, sts_client);
}

////////////////////////////////////////////////////////////////////////////
// Basic test for the Minio test server.

class TestMinioServer : public S3TestMixin {
 public:
  void SetUp() override { S3TestMixin::SetUp(); }

 protected:
};

TEST_F(TestMinioServer, Connect) {
  // Just a dummy connection test.  Check that we can list buckets,
  // and that there are none (the server is launched in an empty temp dir).
  ARROW_AWS_ASSIGN_OR_FAIL(auto bucket_list, client_->ListBuckets());
  ASSERT_EQ(bucket_list.GetBuckets().size(), 0);
}

////////////////////////////////////////////////////////////////////////////
// Concrete S3 tests

class TestS3FS : public S3TestMixin {
 public:
  void SetUp() override {
    S3TestMixin::SetUp();
    MakeFileSystem();
    // Set up test bucket
    {
      Aws::S3::Model::CreateBucketRequest req;
      req.SetBucket(ToAwsString("bucket"));
      ASSERT_OK(OutcomeToStatus(client_->CreateBucket(req)));
      req.SetBucket(ToAwsString("empty-bucket"));
      ASSERT_OK(OutcomeToStatus(client_->CreateBucket(req)));
    }
    {
      Aws::S3::Model::PutObjectRequest req;
      req.SetBucket(ToAwsString("bucket"));
      req.SetKey(ToAwsString("emptydir/"));
      ASSERT_OK(OutcomeToStatus(client_->PutObject(req)));
      // NOTE: no need to create intermediate "directories" somedir/ and
      // somedir/subdir/
      req.SetKey(ToAwsString("somedir/subdir/subfile"));
      req.SetBody(std::make_shared<std::stringstream>("sub data"));
      ASSERT_OK(OutcomeToStatus(client_->PutObject(req)));
      req.SetKey(ToAwsString("somefile"));
      req.SetBody(std::make_shared<std::stringstream>("some data"));
      ASSERT_OK(OutcomeToStatus(client_->PutObject(req)));
    }
  }

  void MakeFileSystem() {
    options_.ConfigureAccessKey(minio_.access_key(), minio_.secret_key());
    options_.scheme = "http";
    options_.endpoint_override = minio_.connect_string();
    ASSERT_OK_AND_ASSIGN(fs_, S3FileSystem::Make(options_));
  }

  void TestOpenOutputStream() {
    std::shared_ptr<io::OutputStream> stream;

    // Nonexistent
    ASSERT_RAISES(IOError, fs_->OpenOutputStream("nonexistent-bucket/somefile"));

    // Create new empty file
    ASSERT_OK_AND_ASSIGN(stream, fs_->OpenOutputStream("bucket/newfile1"));
    ASSERT_OK(stream->Close());
    AssertObjectContents(client_.get(), "bucket", "newfile1", "");

    // Create new file with 1 small write
    ASSERT_OK_AND_ASSIGN(stream, fs_->OpenOutputStream("bucket/newfile2"));
    ASSERT_OK(stream->Write("some data"));
    ASSERT_OK(stream->Close());
    AssertObjectContents(client_.get(), "bucket", "newfile2", "some data");

    // Create new file with 3 small writes
    ASSERT_OK_AND_ASSIGN(stream, fs_->OpenOutputStream("bucket/newfile3"));
    ASSERT_OK(stream->Write("some "));
    ASSERT_OK(stream->Write(""));
    ASSERT_OK(stream->Write("new data"));
    ASSERT_OK(stream->Close());
    AssertObjectContents(client_.get(), "bucket", "newfile3", "some new data");

    // Create new file with some large writes
    std::string s1, s2, s3, s4, s5, expected;
    s1 = random_string(6000000, /*seed =*/42);  // More than the 5 MB minimum part upload
    s2 = "xxx";
    s3 = random_string(6000000, 43);
    s4 = "zzz";
    s5 = random_string(600000, 44);
    expected = s1 + s2 + s3 + s4 + s5;
    ASSERT_OK_AND_ASSIGN(stream, fs_->OpenOutputStream("bucket/newfile4"));
    for (auto input : {s1, s2, s3, s4, s5}) {
      ASSERT_OK(stream->Write(input));
      // Clobber source contents.  This shouldn't reflect in the data written.
      input.front() = 'x';
      input.back() = 'x';
    }
    ASSERT_OK(stream->Close());
    AssertObjectContents(client_.get(), "bucket", "newfile4", expected);

    // Overwrite
    ASSERT_OK_AND_ASSIGN(stream, fs_->OpenOutputStream("bucket/newfile1"));
    ASSERT_OK(stream->Write("overwritten data"));
    ASSERT_OK(stream->Close());
    AssertObjectContents(client_.get(), "bucket", "newfile1", "overwritten data");

    // Overwrite and make empty
    ASSERT_OK_AND_ASSIGN(stream, fs_->OpenOutputStream("bucket/newfile1"));
    ASSERT_OK(stream->Close());
    AssertObjectContents(client_.get(), "bucket", "newfile1", "");

    // Open file and then lose filesystem reference
    ASSERT_EQ(fs_.use_count(), 1);  // needed for test to work
    std::weak_ptr<S3FileSystem> weak_fs(fs_);
    ASSERT_OK_AND_ASSIGN(stream, fs_->OpenOutputStream("bucket/newfile5"));
    fs_.reset();
    ASSERT_FALSE(weak_fs.expired());
    ASSERT_OK(stream->Write("some data"));
    ASSERT_OK(stream->Close());
    ASSERT_TRUE(weak_fs.expired());
  }

  void TestOpenOutputStreamAbort() {
    std::shared_ptr<io::OutputStream> stream;
    ASSERT_OK_AND_ASSIGN(stream, fs_->OpenOutputStream("bucket/somefile"));
    ASSERT_OK(stream->Write("new data"));
    // Abort() cancels the multipart upload.
    ASSERT_OK(stream->Abort());
    ASSERT_EQ(stream->closed(), true);
    AssertObjectContents(client_.get(), "bucket", "somefile", "some data");
  }

  void TestOpenOutputStreamDestructor() {
    std::shared_ptr<io::OutputStream> stream;
    ASSERT_OK_AND_ASSIGN(stream, fs_->OpenOutputStream("bucket/somefile"));
    ASSERT_OK(stream->Write("new data"));
    // Destructor implicitly closes stream and completes the multipart upload.
    stream.reset();
    AssertObjectContents(client_.get(), "bucket", "somefile", "new data");
  }

 protected:
  S3Options options_;
  std::shared_ptr<S3FileSystem> fs_;
};

TEST_F(TestS3FS, GetFileInfoRoot) { AssertFileInfo(fs_.get(), "", FileType::Directory); }

TEST_F(TestS3FS, GetFileInfoBucket) {
  AssertFileInfo(fs_.get(), "bucket", FileType::Directory);
  AssertFileInfo(fs_.get(), "empty-bucket", FileType::Directory);
  AssertFileInfo(fs_.get(), "nonexistent-bucket", FileType::NotFound);
  // Trailing slashes
  AssertFileInfo(fs_.get(), "bucket/", FileType::Directory);
  AssertFileInfo(fs_.get(), "empty-bucket/", FileType::Directory);
  AssertFileInfo(fs_.get(), "nonexistent-bucket/", FileType::NotFound);
}

TEST_F(TestS3FS, GetFileInfoObject) {
  // "Directories"
  AssertFileInfo(fs_.get(), "bucket/emptydir", FileType::Directory, kNoSize);
  AssertFileInfo(fs_.get(), "bucket/somedir", FileType::Directory, kNoSize);
  AssertFileInfo(fs_.get(), "bucket/somedir/subdir", FileType::Directory, kNoSize);

  // "Files"
  AssertFileInfo(fs_.get(), "bucket/somefile", FileType::File, 9);
  AssertFileInfo(fs_.get(), "bucket/somedir/subdir/subfile", FileType::File, 8);

  // Nonexistent
  AssertFileInfo(fs_.get(), "bucket/emptyd", FileType::NotFound);
  AssertFileInfo(fs_.get(), "bucket/somed", FileType::NotFound);
  AssertFileInfo(fs_.get(), "non-existent-bucket/somed", FileType::NotFound);

  // Trailing slashes
  AssertFileInfo(fs_.get(), "bucket/emptydir/", FileType::Directory, kNoSize);
  AssertFileInfo(fs_.get(), "bucket/somefile/", FileType::File, 9);
  AssertFileInfo(fs_.get(), "bucket/emptyd/", FileType::NotFound);
  AssertFileInfo(fs_.get(), "non-existent-bucket/somed/", FileType::NotFound);
}

TEST_F(TestS3FS, GetFileInfoSelector) {
  FileSelector select;
  std::vector<FileInfo> infos;

  // Root dir
  select.base_dir = "";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 2);
  SortInfos(&infos);
  AssertFileInfo(infos[0], "bucket", FileType::Directory);
  AssertFileInfo(infos[1], "empty-bucket", FileType::Directory);

  // Empty bucket
  select.base_dir = "empty-bucket";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 0);
  // Nonexistent bucket
  select.base_dir = "nonexistent-bucket";
  ASSERT_RAISES(IOError, fs_->GetFileInfo(select));
  select.allow_not_found = true;
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 0);
  select.allow_not_found = false;
  // Non-empty bucket
  select.base_dir = "bucket";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  SortInfos(&infos);
  ASSERT_EQ(infos.size(), 3);
  AssertFileInfo(infos[0], "bucket/emptydir", FileType::Directory);
  AssertFileInfo(infos[1], "bucket/somedir", FileType::Directory);
  AssertFileInfo(infos[2], "bucket/somefile", FileType::File, 9);

  // Empty "directory"
  select.base_dir = "bucket/emptydir";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 0);
  // Non-empty "directories"
  select.base_dir = "bucket/somedir";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 1);
  AssertFileInfo(infos[0], "bucket/somedir/subdir", FileType::Directory);
  select.base_dir = "bucket/somedir/subdir";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 1);
  AssertFileInfo(infos[0], "bucket/somedir/subdir/subfile", FileType::File, 8);
  // Nonexistent
  select.base_dir = "bucket/nonexistent";
  ASSERT_RAISES(IOError, fs_->GetFileInfo(select));
  select.allow_not_found = true;
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 0);
  select.allow_not_found = false;

  // Trailing slashes
  select.base_dir = "empty-bucket/";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 0);
  select.base_dir = "nonexistent-bucket/";
  ASSERT_RAISES(IOError, fs_->GetFileInfo(select));
  select.base_dir = "bucket/";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  SortInfos(&infos);
  ASSERT_EQ(infos.size(), 3);
}

TEST_F(TestS3FS, GetFileInfoSelectorRecursive) {
  FileSelector select;
  std::vector<FileInfo> infos;
  select.recursive = true;

  // Root dir
  select.base_dir = "";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 7);
  SortInfos(&infos);
  AssertFileInfo(infos[0], "bucket", FileType::Directory);
  AssertFileInfo(infos[1], "bucket/emptydir", FileType::Directory);
  AssertFileInfo(infos[2], "bucket/somedir", FileType::Directory);
  AssertFileInfo(infos[3], "bucket/somedir/subdir", FileType::Directory);
  AssertFileInfo(infos[4], "bucket/somedir/subdir/subfile", FileType::File, 8);
  AssertFileInfo(infos[5], "bucket/somefile", FileType::File, 9);
  AssertFileInfo(infos[6], "empty-bucket", FileType::Directory);

  // Empty bucket
  select.base_dir = "empty-bucket";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 0);

  // Non-empty bucket
  select.base_dir = "bucket";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  SortInfos(&infos);
  ASSERT_EQ(infos.size(), 5);
  AssertFileInfo(infos[0], "bucket/emptydir", FileType::Directory);
  AssertFileInfo(infos[1], "bucket/somedir", FileType::Directory);
  AssertFileInfo(infos[2], "bucket/somedir/subdir", FileType::Directory);
  AssertFileInfo(infos[3], "bucket/somedir/subdir/subfile", FileType::File, 8);
  AssertFileInfo(infos[4], "bucket/somefile", FileType::File, 9);

  // Empty "directory"
  select.base_dir = "bucket/emptydir";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 0);

  // Non-empty "directories"
  select.base_dir = "bucket/somedir";
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  SortInfos(&infos);
  ASSERT_EQ(infos.size(), 2);
  AssertFileInfo(infos[0], "bucket/somedir/subdir", FileType::Directory);
  AssertFileInfo(infos[1], "bucket/somedir/subdir/subfile", FileType::File, 8);
}

TEST_F(TestS3FS, CreateDir) {
  FileInfo st;

  // Existing bucket
  ASSERT_OK(fs_->CreateDir("bucket"));
  AssertFileInfo(fs_.get(), "bucket", FileType::Directory);

  // New bucket
  AssertFileInfo(fs_.get(), "new-bucket", FileType::NotFound);
  ASSERT_OK(fs_->CreateDir("new-bucket"));
  AssertFileInfo(fs_.get(), "new-bucket", FileType::Directory);

  // Existing "directory"
  AssertFileInfo(fs_.get(), "bucket/somedir", FileType::Directory);
  ASSERT_OK(fs_->CreateDir("bucket/somedir"));
  AssertFileInfo(fs_.get(), "bucket/somedir", FileType::Directory);

  AssertFileInfo(fs_.get(), "bucket/emptydir", FileType::Directory);
  ASSERT_OK(fs_->CreateDir("bucket/emptydir"));
  AssertFileInfo(fs_.get(), "bucket/emptydir", FileType::Directory);

  // New "directory"
  AssertFileInfo(fs_.get(), "bucket/newdir", FileType::NotFound);
  ASSERT_OK(fs_->CreateDir("bucket/newdir"));
  AssertFileInfo(fs_.get(), "bucket/newdir", FileType::Directory);

  // New "directory", recursive
  ASSERT_OK(fs_->CreateDir("bucket/newdir/newsub/newsubsub", /*recursive=*/true));
  AssertFileInfo(fs_.get(), "bucket/newdir/newsub", FileType::Directory);
  AssertFileInfo(fs_.get(), "bucket/newdir/newsub/newsubsub", FileType::Directory);

  // Existing "file", should fail
  ASSERT_RAISES(IOError, fs_->CreateDir("bucket/somefile"));
}

TEST_F(TestS3FS, DeleteFile) {
  // Bucket
  ASSERT_RAISES(IOError, fs_->DeleteFile("bucket"));
  ASSERT_RAISES(IOError, fs_->DeleteFile("empty-bucket"));
  ASSERT_RAISES(IOError, fs_->DeleteFile("nonexistent-bucket"));

  // "File"
  ASSERT_OK(fs_->DeleteFile("bucket/somefile"));
  AssertFileInfo(fs_.get(), "bucket/somefile", FileType::NotFound);
  ASSERT_RAISES(IOError, fs_->DeleteFile("bucket/somefile"));
  ASSERT_RAISES(IOError, fs_->DeleteFile("bucket/nonexistent"));

  // "Directory"
  ASSERT_RAISES(IOError, fs_->DeleteFile("bucket/somedir"));
  AssertFileInfo(fs_.get(), "bucket/somedir", FileType::Directory);
}

TEST_F(TestS3FS, DeleteDir) {
  FileSelector select;
  select.base_dir = "bucket";
  std::vector<FileInfo> infos;

  // Empty "directory"
  ASSERT_OK(fs_->DeleteDir("bucket/emptydir"));
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 2);
  SortInfos(&infos);
  AssertFileInfo(infos[0], "bucket/somedir", FileType::Directory);
  AssertFileInfo(infos[1], "bucket/somefile", FileType::File);

  // Non-empty "directory"
  ASSERT_OK(fs_->DeleteDir("bucket/somedir"));
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 1);
  AssertFileInfo(infos[0], "bucket/somefile", FileType::File);

  // Leaving parent "directory" empty
  ASSERT_OK(fs_->CreateDir("bucket/newdir/newsub/newsubsub"));
  ASSERT_OK(fs_->DeleteDir("bucket/newdir/newsub"));
  ASSERT_OK_AND_ASSIGN(infos, fs_->GetFileInfo(select));
  ASSERT_EQ(infos.size(), 2);
  SortInfos(&infos);
  AssertFileInfo(infos[0], "bucket/newdir", FileType::Directory);  // still exists
  AssertFileInfo(infos[1], "bucket/somefile", FileType::File);

  // Bucket
  ASSERT_OK(fs_->DeleteDir("bucket"));
  AssertFileInfo(fs_.get(), "bucket", FileType::NotFound);
}

TEST_F(TestS3FS, CopyFile) {
  // "File"
  ASSERT_OK(fs_->CopyFile("bucket/somefile", "bucket/newfile"));
  AssertFileInfo(fs_.get(), "bucket/newfile", FileType::File, 9);
  AssertObjectContents(client_.get(), "bucket", "newfile", "some data");
  AssertFileInfo(fs_.get(), "bucket/somefile", FileType::File, 9);  // still exists
  // Overwrite
  ASSERT_OK(fs_->CopyFile("bucket/somedir/subdir/subfile", "bucket/newfile"));
  AssertFileInfo(fs_.get(), "bucket/newfile", FileType::File, 8);
  AssertObjectContents(client_.get(), "bucket", "newfile", "sub data");

  // Nonexistent
  ASSERT_RAISES(IOError, fs_->CopyFile("bucket/nonexistent", "bucket/newfile2"));
  ASSERT_RAISES(IOError, fs_->CopyFile("nonexistent-bucket/somefile", "bucket/newfile2"));
  ASSERT_RAISES(IOError, fs_->CopyFile("bucket/somefile", "nonexistent-bucket/newfile2"));
  AssertFileInfo(fs_.get(), "bucket/newfile2", FileType::NotFound);
}

TEST_F(TestS3FS, Move) {
  // "File"
  ASSERT_OK(fs_->Move("bucket/somefile", "bucket/newfile"));
  AssertFileInfo(fs_.get(), "bucket/newfile", FileType::File, 9);
  AssertObjectContents(client_.get(), "bucket", "newfile", "some data");
  // Source was deleted
  AssertFileInfo(fs_.get(), "bucket/somefile", FileType::NotFound);

  // Overwrite
  ASSERT_OK(fs_->Move("bucket/somedir/subdir/subfile", "bucket/newfile"));
  AssertFileInfo(fs_.get(), "bucket/newfile", FileType::File, 8);
  AssertObjectContents(client_.get(), "bucket", "newfile", "sub data");
  // Source was deleted
  AssertFileInfo(fs_.get(), "bucket/somedir/subdir/subfile", FileType::NotFound);

  // Nonexistent
  ASSERT_RAISES(IOError, fs_->Move("bucket/non-existent", "bucket/newfile2"));
  ASSERT_RAISES(IOError, fs_->Move("nonexistent-bucket/somefile", "bucket/newfile2"));
  ASSERT_RAISES(IOError, fs_->Move("bucket/somefile", "nonexistent-bucket/newfile2"));
  AssertFileInfo(fs_.get(), "bucket/newfile2", FileType::NotFound);
}

TEST_F(TestS3FS, OpenInputStream) {
  std::shared_ptr<io::InputStream> stream;
  std::shared_ptr<Buffer> buf;

  // Nonexistent
  ASSERT_RAISES(IOError, fs_->OpenInputStream("nonexistent-bucket/somefile"));
  ASSERT_RAISES(IOError, fs_->OpenInputStream("bucket/zzzt"));

  // "Files"
  ASSERT_OK_AND_ASSIGN(stream, fs_->OpenInputStream("bucket/somefile"));
  ASSERT_OK_AND_ASSIGN(buf, stream->Read(2));
  AssertBufferEqual(*buf, "so");
  ASSERT_OK_AND_ASSIGN(buf, stream->Read(5));
  AssertBufferEqual(*buf, "me da");
  ASSERT_OK_AND_ASSIGN(buf, stream->Read(5));
  AssertBufferEqual(*buf, "ta");
  ASSERT_OK_AND_ASSIGN(buf, stream->Read(5));
  AssertBufferEqual(*buf, "");

  ASSERT_OK_AND_ASSIGN(stream, fs_->OpenInputStream("bucket/somedir/subdir/subfile"));
  ASSERT_OK_AND_ASSIGN(buf, stream->Read(100));
  AssertBufferEqual(*buf, "sub data");
  ASSERT_OK_AND_ASSIGN(buf, stream->Read(100));
  AssertBufferEqual(*buf, "");
  ASSERT_OK(stream->Close());

  // "Directories"
  ASSERT_RAISES(IOError, fs_->OpenInputStream("bucket/emptydir"));
  ASSERT_RAISES(IOError, fs_->OpenInputStream("bucket/somedir"));
  ASSERT_RAISES(IOError, fs_->OpenInputStream("bucket"));

  // Open file and then lose filesystem reference
  ASSERT_EQ(fs_.use_count(), 1);  // needed for test to work
  std::weak_ptr<S3FileSystem> weak_fs(fs_);
  ASSERT_OK_AND_ASSIGN(stream, fs_->OpenInputStream("bucket/somefile"));
  fs_.reset();
  ASSERT_FALSE(weak_fs.expired());
  ASSERT_OK_AND_ASSIGN(buf, stream->Read(10));
  AssertBufferEqual(*buf, "some data");
  ASSERT_OK(stream->Close());
  ASSERT_TRUE(weak_fs.expired());
}

TEST_F(TestS3FS, OpenInputFile) {
  std::shared_ptr<io::RandomAccessFile> file;
  std::shared_ptr<Buffer> buf;

  // Nonexistent
  ASSERT_RAISES(IOError, fs_->OpenInputFile("nonexistent-bucket/somefile"));
  ASSERT_RAISES(IOError, fs_->OpenInputFile("bucket/zzzt"));

  // "Files"
  ASSERT_OK_AND_ASSIGN(file, fs_->OpenInputFile("bucket/somefile"));
  ASSERT_OK_AND_EQ(9, file->GetSize());
  ASSERT_OK_AND_ASSIGN(buf, file->Read(4));
  AssertBufferEqual(*buf, "some");
  ASSERT_OK_AND_EQ(9, file->GetSize());
  ASSERT_OK_AND_EQ(4, file->Tell());

  ASSERT_OK_AND_ASSIGN(buf, file->ReadAt(2, 5));
  AssertBufferEqual(*buf, "me da");
  ASSERT_OK_AND_EQ(4, file->Tell());
  ASSERT_OK_AND_ASSIGN(buf, file->ReadAt(5, 20));
  AssertBufferEqual(*buf, "data");
  ASSERT_OK_AND_ASSIGN(buf, file->ReadAt(9, 20));
  AssertBufferEqual(*buf, "");

  char result[10];
  ASSERT_OK_AND_EQ(5, file->ReadAt(2, 5, &result));
  ASSERT_OK_AND_EQ(4, file->ReadAt(5, 20, &result));
  ASSERT_OK_AND_EQ(0, file->ReadAt(9, 0, &result));

  // Reading past end of file
  ASSERT_RAISES(IOError, file->ReadAt(10, 20));

  ASSERT_OK(file->Seek(5));
  ASSERT_OK_AND_ASSIGN(buf, file->Read(2));
  AssertBufferEqual(*buf, "da");
  ASSERT_OK(file->Seek(9));
  ASSERT_OK_AND_ASSIGN(buf, file->Read(2));
  AssertBufferEqual(*buf, "");
  // Seeking past end of file
  ASSERT_RAISES(IOError, file->Seek(10));
}

TEST_F(TestS3FS, OpenOutputStreamBackgroundWrites) { TestOpenOutputStream(); }

TEST_F(TestS3FS, OpenOutputStreamSyncWrites) {
  options_.background_writes = false;
  MakeFileSystem();
  TestOpenOutputStream();
}

TEST_F(TestS3FS, OpenOutputStreamAbortBackgroundWrites) { TestOpenOutputStreamAbort(); }

TEST_F(TestS3FS, OpenOutputStreamAbortSyncWrites) {
  options_.background_writes = false;
  MakeFileSystem();
  TestOpenOutputStreamAbort();
}

TEST_F(TestS3FS, OpenOutputStreamDestructorBackgroundWrites) {
  TestOpenOutputStreamDestructor();
}

TEST_F(TestS3FS, OpenOutputStreamDestructorSyncWrite) {
  options_.background_writes = false;
  MakeFileSystem();
  TestOpenOutputStreamDestructor();
}

TEST_F(TestS3FS, FileSystemFromUri) {
  std::stringstream ss;
  ss << "s3://" << minio_.access_key() << ":" << minio_.secret_key()
     << "@bucket/somedir/subdir/subfile"
     << "?scheme=http&endpoint_override=" << UriEscape(minio_.connect_string());

  std::string path;
  ASSERT_OK_AND_ASSIGN(auto fs, FileSystemFromUri(ss.str(), &path));
  ASSERT_EQ(path, "bucket/somedir/subdir/subfile");

  // Check the filesystem has the right connection parameters
  AssertFileInfo(fs.get(), path, FileType::File, 8);
}

////////////////////////////////////////////////////////////////////////////
// Generic S3 tests

class TestS3FSGeneric : public S3TestMixin, public GenericFileSystemTest {
 public:
  void SetUp() override {
    S3TestMixin::SetUp();
    // Set up test bucket
    {
      Aws::S3::Model::CreateBucketRequest req;
      req.SetBucket(ToAwsString("s3fs-test-bucket"));
      ASSERT_OK(OutcomeToStatus(client_->CreateBucket(req)));
    }

    options_.ConfigureAccessKey(minio_.access_key(), minio_.secret_key());
    options_.scheme = "http";
    options_.endpoint_override = minio_.connect_string();
    ASSERT_OK_AND_ASSIGN(s3fs_, S3FileSystem::Make(options_));
    fs_ = std::make_shared<SubTreeFileSystem>("s3fs-test-bucket", s3fs_);
  }

 protected:
  std::shared_ptr<FileSystem> GetEmptyFileSystem() override { return fs_; }

  bool have_implicit_directories() const override { return true; }
  bool allow_write_file_over_dir() const override { return true; }
  bool allow_move_dir() const override { return false; }
  bool allow_append_to_file() const override { return false; }
  bool have_directory_mtimes() const override { return false; }
  bool have_flaky_directory_tree_deletion() const override {
#ifdef _WIN32
    // Recent Minio versions on Windows may not register deletion of all
    // directories in a tree when doing a bulk delete.
    return true;
#else
    return false;
#endif
  }

  S3Options options_;
  std::shared_ptr<S3FileSystem> s3fs_;
  std::shared_ptr<FileSystem> fs_;
};

GENERIC_FS_TEST_FUNCTIONS(TestS3FSGeneric);

}  // namespace fs
}  // namespace arrow
