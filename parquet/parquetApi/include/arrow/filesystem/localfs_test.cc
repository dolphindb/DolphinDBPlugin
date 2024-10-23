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

#include <cerrno>
#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "arrow/filesystem/filesystem.h"
#include "arrow/filesystem/localfs.h"
#include "arrow/filesystem/path_util.h"
#include "arrow/filesystem/test_util.h"
#include "arrow/filesystem/util_internal.h"
#include "arrow/testing/gtest_util.h"
#include "arrow/util/io_util.h"

namespace arrow {
namespace fs {
namespace internal {

using ::arrow::internal::PlatformFilename;
using ::arrow::internal::TemporaryDir;

class LocalFSTestMixin : public ::testing::Test {
 public:
  void SetUp() override {
    ASSERT_OK_AND_ASSIGN(temp_dir_, TemporaryDir::Make("test-localfs-"));
  }

 protected:
  std::unique_ptr<TemporaryDir> temp_dir_;
};

struct CommonPathFormatter {
  std::string operator()(std::string fn) { return fn; }
  bool supports_uri() { return true; }
};

#ifdef _WIN32
struct ExtendedLengthPathFormatter {
  std::string operator()(std::string fn) { return "//?/" + fn; }
  // The path prefix conflicts with URI syntax
  bool supports_uri() { return false; }
};

using PathFormatters = ::testing::Types<CommonPathFormatter, ExtendedLengthPathFormatter>;
#else
using PathFormatters = ::testing::Types<CommonPathFormatter>;
#endif

// Non-overloaded version of FileSystemFromUri, for template resolution
// in CheckFileSystemFromUriFunc.
Result<std::shared_ptr<FileSystem>> FSFromUri(const std::string& uri,
                                              std::string* out_path = NULLPTR) {
  return FileSystemFromUri(uri, out_path);
}

////////////////////////////////////////////////////////////////////////////
// Misc tests

TEST(DetectAbsolutePath, Basics) {
  ASSERT_TRUE(DetectAbsolutePath("/"));
  ASSERT_TRUE(DetectAbsolutePath("/foo"));
  ASSERT_TRUE(DetectAbsolutePath("/foo/bar.txt"));
  ASSERT_TRUE(DetectAbsolutePath("//foo/bar/baz"));

#ifdef _WIN32
  constexpr bool is_win32 = true;
#else
  constexpr bool is_win32 = false;
#endif
  ASSERT_EQ(is_win32, DetectAbsolutePath("A:/"));
  ASSERT_EQ(is_win32, DetectAbsolutePath("z:/foo"));

  ASSERT_EQ(is_win32, DetectAbsolutePath("\\"));
  ASSERT_EQ(is_win32, DetectAbsolutePath("\\foo"));
  ASSERT_EQ(is_win32, DetectAbsolutePath("\\foo\\bar"));
  ASSERT_EQ(is_win32, DetectAbsolutePath("\\\\foo\\bar\\baz"));
  ASSERT_EQ(is_win32, DetectAbsolutePath("Z:\\"));
  ASSERT_EQ(is_win32, DetectAbsolutePath("z:\\foo"));

  ASSERT_FALSE(DetectAbsolutePath("A:"));
  ASSERT_FALSE(DetectAbsolutePath("z:foo"));
  ASSERT_FALSE(DetectAbsolutePath(""));
  ASSERT_FALSE(DetectAbsolutePath("AB:"));
  ASSERT_FALSE(DetectAbsolutePath(":"));
  ASSERT_FALSE(DetectAbsolutePath(""));
  ASSERT_FALSE(DetectAbsolutePath("@:"));
  ASSERT_FALSE(DetectAbsolutePath("à:"));
  ASSERT_FALSE(DetectAbsolutePath("0:"));
  ASSERT_FALSE(DetectAbsolutePath("A"));
  ASSERT_FALSE(DetectAbsolutePath("foo/bar"));
  ASSERT_FALSE(DetectAbsolutePath("foo\\bar"));
}

////////////////////////////////////////////////////////////////////////////
// Generic LocalFileSystem tests

template <typename PathFormatter>
class TestLocalFSGeneric : public LocalFSTestMixin, public GenericFileSystemTest {
 public:
  void SetUp() override {
    LocalFSTestMixin::SetUp();
    local_fs_ = std::make_shared<LocalFileSystem>(options());
    auto path = PathFormatter()(temp_dir_->path().ToString());
    fs_ = std::make_shared<SubTreeFileSystem>(path, local_fs_);
  }

 protected:
  virtual LocalFileSystemOptions options() { return LocalFileSystemOptions::Defaults(); }

  std::shared_ptr<FileSystem> GetEmptyFileSystem() override { return fs_; }

  std::shared_ptr<LocalFileSystem> local_fs_;
  std::shared_ptr<FileSystem> fs_;
};

TYPED_TEST_SUITE(TestLocalFSGeneric, PathFormatters);

GENERIC_FS_TYPED_TEST_FUNCTIONS(TestLocalFSGeneric);

class TestLocalFSGenericMMap : public TestLocalFSGeneric<CommonPathFormatter> {
 protected:
  LocalFileSystemOptions options() override {
    auto options = LocalFileSystemOptions::Defaults();
    options.use_mmap = true;
    return options;
  }
};

GENERIC_FS_TEST_FUNCTIONS(TestLocalFSGenericMMap);

////////////////////////////////////////////////////////////////////////////
// Concrete LocalFileSystem tests

template <typename PathFormatter>
class TestLocalFS : public LocalFSTestMixin {
 public:
  void SetUp() {
    LocalFSTestMixin::SetUp();
    path_formatter_ = PathFormatter();
    local_fs_ = std::make_shared<LocalFileSystem>();
    local_path_ = EnsureTrailingSlash(path_formatter_(temp_dir_->path().ToString()));
    fs_ = std::make_shared<SubTreeFileSystem>(local_path_, local_fs_);
  }

  std::string UriFromAbsolutePath(const std::string& path) {
#ifdef _WIN32
    // Path is supposed to start with "X:/..."
    return "file:///" + path;
#else
    // Path is supposed to start with "/..."
    return "file://" + path;
#endif
  }

  template <typename FileSystemFromUriFunc>
  void CheckFileSystemFromUriFunc(const std::string& uri,
                                  FileSystemFromUriFunc&& fs_from_uri) {
    if (!path_formatter_.supports_uri()) {
      return;  // skip
    }
    std::string path;
    ASSERT_OK_AND_ASSIGN(fs_, fs_from_uri(uri, &path));
    ASSERT_EQ(path, local_path_);

    // Test that the right location on disk is accessed
    CreateFile(fs_.get(), local_path_ + "abc", "some data");
    CheckConcreteFile(this->temp_dir_->path().ToString() + "abc", 9);
  }

  void TestFileSystemFromUri(const std::string& uri) {
    CheckFileSystemFromUriFunc(uri, FSFromUri);
  }

  void TestFileSystemFromUriOrPath(const std::string& uri) {
    CheckFileSystemFromUriFunc(uri, FileSystemFromUriOrPath);
  }

  template <typename FileSystemFromUriFunc>
  void CheckLocalUri(const std::string& uri, const std::string& expected_path,
                     FileSystemFromUriFunc&& fs_from_uri) {
    if (!path_formatter_.supports_uri()) {
      return;  // skip
    }
    std::string path;
    ASSERT_OK_AND_ASSIGN(fs_, fs_from_uri(uri, &path));
    ASSERT_EQ(fs_->type_name(), "local");
    ASSERT_EQ(path, expected_path);
  }

  // Like TestFileSystemFromUri, but with an arbitrary non-existing path
  void TestLocalUri(const std::string& uri, const std::string& expected_path) {
    CheckLocalUri(uri, expected_path, FSFromUri);
  }

  void TestLocalUriOrPath(const std::string& uri, const std::string& expected_path) {
    CheckLocalUri(uri, expected_path, FileSystemFromUriOrPath);
  }

  void TestInvalidUri(const std::string& uri) {
    if (!path_formatter_.supports_uri()) {
      return;  // skip
    }
    ASSERT_RAISES(Invalid, FileSystemFromUri(uri));
  }

  void TestInvalidUriOrPath(const std::string& uri) {
    if (!path_formatter_.supports_uri()) {
      return;  // skip
    }
    ASSERT_RAISES(Invalid, FileSystemFromUriOrPath(uri));
  }

  void CheckConcreteFile(const std::string& path, int64_t expected_size) {
    ASSERT_OK_AND_ASSIGN(auto fn, PlatformFilename::FromString(path));
    ASSERT_OK_AND_ASSIGN(int fd, ::arrow::internal::FileOpenReadable(fn));
    auto result = ::arrow::internal::FileGetSize(fd);
    ASSERT_OK(::arrow::internal::FileClose(fd));
    ASSERT_OK_AND_ASSIGN(int64_t size, result);
    ASSERT_EQ(size, expected_size);
  }

  static void CheckNormalizePath(const std::shared_ptr<FileSystem>& fs) {}

 protected:
  PathFormatter path_formatter_;
  std::shared_ptr<LocalFileSystem> local_fs_;
  std::shared_ptr<FileSystem> fs_;
  std::string local_path_;
};

TYPED_TEST_SUITE(TestLocalFS, PathFormatters);

TYPED_TEST(TestLocalFS, CorrectPathExists) {
  // Test that the right location on disk is accessed
  std::shared_ptr<io::OutputStream> stream;
  ASSERT_OK_AND_ASSIGN(stream, this->fs_->OpenOutputStream("abc"));
  std::string data = "some data";
  auto data_size = static_cast<int64_t>(data.size());
  ASSERT_OK(stream->Write(data.data(), data_size));
  ASSERT_OK(stream->Close());

  // Now check the file's existence directly, bypassing the FileSystem abstraction
  this->CheckConcreteFile(this->temp_dir_->path().ToString() + "abc", data_size);
}

TYPED_TEST(TestLocalFS, NormalizePath) {
#ifdef _WIN32
  ASSERT_OK_AND_EQ("AB/CD", this->local_fs_->NormalizePath("AB\\CD"));
  ASSERT_OK_AND_EQ("/AB/CD", this->local_fs_->NormalizePath("\\AB\\CD"));
  ASSERT_OK_AND_EQ("C:DE/fgh", this->local_fs_->NormalizePath("C:DE\\fgh"));
  ASSERT_OK_AND_EQ("C:/DE/fgh", this->local_fs_->NormalizePath("C:\\DE\\fgh"));
  ASSERT_OK_AND_EQ("//some/share/AB",
                   this->local_fs_->NormalizePath("\\\\some\\share\\AB"));
#else
  ASSERT_OK_AND_EQ("AB\\CD", this->local_fs_->NormalizePath("AB\\CD"));
#endif
}

TYPED_TEST(TestLocalFS, NormalizePathThroughSubtreeFS) {
#ifdef _WIN32
  ASSERT_OK_AND_EQ("AB/CD", this->fs_->NormalizePath("AB\\CD"));
#else
  ASSERT_OK_AND_EQ("AB\\CD", this->fs_->NormalizePath("AB\\CD"));
#endif
}

TYPED_TEST(TestLocalFS, FileSystemFromUriFile) {
  // Concrete test with actual file
  const auto uri_string = this->UriFromAbsolutePath(this->local_path_);
  this->TestFileSystemFromUri(uri_string);
  this->TestFileSystemFromUriOrPath(uri_string);

  // Variations
  this->TestLocalUri("file:/foo/bar", "/foo/bar");
  this->TestLocalUri("file:///foo/bar", "/foo/bar");
#ifdef _WIN32
  this->TestLocalUri("file:/C:/foo/bar", "C:/foo/bar");
  this->TestLocalUri("file:///C:/foo/bar", "C:/foo/bar");
#endif

  // Non-empty authority
#ifdef _WIN32
  this->TestLocalUri("file://server/share/foo/bar", "//server/share/foo/bar");
#else
  this->TestInvalidUri("file://server/share/foo/bar");
#endif

  // Relative paths
  this->TestInvalidUri("file:");
  this->TestInvalidUri("file:foo/bar");
}

TYPED_TEST(TestLocalFS, FileSystemFromUriNoScheme) {
  // Concrete test with actual file
  this->TestFileSystemFromUriOrPath(this->local_path_);
  this->TestInvalidUri(this->local_path_);  // Not actually an URI

  // Variations
  this->TestLocalUriOrPath(this->path_formatter_("/foo/bar"), "/foo/bar");

#ifdef _WIN32
  this->TestLocalUriOrPath(this->path_formatter_("C:/foo/bar/"), "C:/foo/bar/");
#endif

  // Relative paths
  this->TestInvalidUriOrPath("C:foo/bar");
  this->TestInvalidUriOrPath("foo/bar");
}

TYPED_TEST(TestLocalFS, FileSystemFromUriNoSchemeBackslashes) {
  const auto uri_string = ToBackslashes(this->local_path_);
#ifdef _WIN32
  this->TestFileSystemFromUriOrPath(uri_string);

  // Variations
  this->TestLocalUriOrPath(this->path_formatter_("C:\\foo\\bar"), "C:/foo/bar");
#else
  this->TestInvalidUri(uri_string);
#endif

  // Relative paths
  this->TestInvalidUriOrPath("C:foo\\bar");
  this->TestInvalidUriOrPath("foo\\bar");
}

TYPED_TEST(TestLocalFS, DirectoryMTime) {
  TimePoint t1 = CurrentTimePoint();
  ASSERT_OK(this->fs_->CreateDir("AB/CD/EF"));
  TimePoint t2 = CurrentTimePoint();

  std::vector<FileInfo> infos;
  ASSERT_OK_AND_ASSIGN(infos, this->fs_->GetFileInfo({"AB", "AB/CD/EF", "xxx"}));
  ASSERT_EQ(infos.size(), 3);
  AssertFileInfo(infos[0], "AB", FileType::Directory);
  AssertFileInfo(infos[1], "AB/CD/EF", FileType::Directory);
  AssertFileInfo(infos[2], "xxx", FileType::NotFound);

  // NOTE: creating AB/CD updates AB's modification time, but creating
  // AB/CD/EF doesn't.  So AB/CD/EF's modification time should always be
  // the same as or after AB's modification time.
  AssertDurationBetween(infos[1].mtime() - infos[0].mtime(), 0, kTimeSlack);
  // Depending on filesystem time granularity, the recorded time could be
  // before the system time when doing the modification.
  AssertDurationBetween(infos[0].mtime() - t1, -kTimeSlack, kTimeSlack);
  AssertDurationBetween(t2 - infos[1].mtime(), -kTimeSlack, kTimeSlack);
}

TYPED_TEST(TestLocalFS, FileMTime) {
  TimePoint t1 = CurrentTimePoint();
  ASSERT_OK(this->fs_->CreateDir("AB/CD"));
  CreateFile(this->fs_.get(), "AB/CD/ab", "data");
  TimePoint t2 = CurrentTimePoint();

  std::vector<FileInfo> infos;
  ASSERT_OK_AND_ASSIGN(infos, this->fs_->GetFileInfo({"AB", "AB/CD/ab", "xxx"}));
  ASSERT_EQ(infos.size(), 3);
  AssertFileInfo(infos[0], "AB", FileType::Directory);
  AssertFileInfo(infos[1], "AB/CD/ab", FileType::File, 4);
  AssertFileInfo(infos[2], "xxx", FileType::NotFound);

  AssertDurationBetween(infos[1].mtime() - infos[0].mtime(), 0, kTimeSlack);
  AssertDurationBetween(infos[0].mtime() - t1, -kTimeSlack, kTimeSlack);
  AssertDurationBetween(t2 - infos[1].mtime(), -kTimeSlack, kTimeSlack);
}

// TODO Should we test backslash paths on Windows?
// SubTreeFileSystem isn't compatible with them.

}  // namespace internal
}  // namespace fs
}  // namespace arrow
