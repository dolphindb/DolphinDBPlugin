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

#include <algorithm>
#include <chrono>
#include <string>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "arrow/filesystem/test_util.h"
#include "arrow/filesystem/util_internal.h"
#include "arrow/io/interfaces.h"
#include "arrow/status.h"
#include "arrow/testing/gtest_util.h"

using ::testing::ElementsAre;

namespace arrow {
namespace fs {

namespace {

std::vector<FileInfo> GetAllWithType(FileSystem* fs, FileType type) {
  FileSelector selector;
  selector.base_dir = "";
  selector.recursive = true;
  std::vector<FileInfo> infos = std::move(fs->GetFileInfo(selector)).ValueOrDie();
  std::vector<FileInfo> result;
  for (const auto& info : infos) {
    if (info.type() == type) {
      result.push_back(info);
    }
  }
  return result;
}

std::vector<FileInfo> GetAllDirs(FileSystem* fs) {
  return GetAllWithType(fs, FileType::Directory);
}

std::vector<FileInfo> GetAllFiles(FileSystem* fs) {
  return GetAllWithType(fs, FileType::File);
}

void AssertPaths(const std::vector<FileInfo>& infos,
                 const std::vector<std::string>& expected_paths) {
  auto sorted_infos = infos;
  SortInfos(&sorted_infos);
  std::vector<std::string> paths(sorted_infos.size());
  std::transform(sorted_infos.begin(), sorted_infos.end(), paths.begin(),
                 [&](const FileInfo& info) { return info.path(); });

  ASSERT_EQ(paths, expected_paths);
}

void AssertAllDirs(FileSystem* fs, const std::vector<std::string>& expected_paths) {
  AssertPaths(GetAllDirs(fs), expected_paths);
}

void AssertAllFiles(FileSystem* fs, const std::vector<std::string>& expected_paths) {
  AssertPaths(GetAllFiles(fs), expected_paths);
}

void ValidateTimePoint(TimePoint tp) { ASSERT_GE(tp.time_since_epoch().count(), 0); }

};  // namespace

void AssertFileContents(FileSystem* fs, const std::string& path,
                        const std::string& expected_data) {
  ASSERT_OK_AND_ASSIGN(FileInfo info, fs->GetFileInfo(path));
  ASSERT_EQ(info.type(), FileType::File) << "For path '" << path << "'";
  ASSERT_EQ(info.size(), static_cast<int64_t>(expected_data.length()))
      << "For path '" << path << "'";

  ASSERT_OK_AND_ASSIGN(auto stream, fs->OpenInputStream(path));
  ASSERT_OK_AND_ASSIGN(auto buffer, stream->Read(info.size()));
  AssertBufferEqual(*buffer, expected_data);
  // No data left in stream
  ASSERT_OK_AND_ASSIGN(auto leftover, stream->Read(1));
  ASSERT_EQ(leftover->size(), 0);

  ASSERT_OK(stream->Close());
}

void CreateFile(FileSystem* fs, const std::string& path, const std::string& data) {
  ASSERT_OK_AND_ASSIGN(auto stream, fs->OpenOutputStream(path));
  ASSERT_OK(stream->Write(data));
  ASSERT_OK(stream->Close());
}

void SortInfos(std::vector<FileInfo>* infos) {
  std::sort(infos->begin(), infos->end(),
            [](const FileInfo& left, const FileInfo& right) -> bool {
              return left.path() < right.path();
            });
}

void AssertFileInfo(const FileInfo& info, const std::string& path, FileType type) {
  ASSERT_EQ(info.path(), path);
  ASSERT_EQ(info.type(), type) << "For path '" << info.path() << "'";
}

void AssertFileInfo(const FileInfo& info, const std::string& path, FileType type,
                    TimePoint mtime) {
  AssertFileInfo(info, path, type);
  ASSERT_EQ(info.mtime(), mtime) << "For path '" << info.path() << "'";
}

void AssertFileInfo(const FileInfo& info, const std::string& path, FileType type,
                    TimePoint mtime, int64_t size) {
  AssertFileInfo(info, path, type, mtime);
  ASSERT_EQ(info.size(), size) << "For path '" << info.path() << "'";
}

void AssertFileInfo(const FileInfo& info, const std::string& path, FileType type,
                    int64_t size) {
  AssertFileInfo(info, path, type);
  ASSERT_EQ(info.size(), size) << "For path '" << info.path() << "'";
}

void AssertFileInfo(FileSystem* fs, const std::string& path, FileType type) {
  ASSERT_OK_AND_ASSIGN(FileInfo info, fs->GetFileInfo(path));
  AssertFileInfo(info, path, type);
}

void AssertFileInfo(FileSystem* fs, const std::string& path, FileType type,
                    TimePoint mtime) {
  ASSERT_OK_AND_ASSIGN(FileInfo info, fs->GetFileInfo(path));
  AssertFileInfo(info, path, type, mtime);
}

void AssertFileInfo(FileSystem* fs, const std::string& path, FileType type,
                    TimePoint mtime, int64_t size) {
  ASSERT_OK_AND_ASSIGN(FileInfo info, fs->GetFileInfo(path));
  AssertFileInfo(info, path, type, mtime, size);
}

void AssertFileInfo(FileSystem* fs, const std::string& path, FileType type,
                    int64_t size) {
  ASSERT_OK_AND_ASSIGN(FileInfo info, fs->GetFileInfo(path));
  AssertFileInfo(info, path, type, size);
}

////////////////////////////////////////////////////////////////////////////
// GenericFileSystemTest implementation

// XXX is there a way we can test mtimes reliably and precisely?

GenericFileSystemTest::~GenericFileSystemTest() {}

void GenericFileSystemTest::TestEmpty(FileSystem* fs) {
  auto dirs = GetAllDirs(fs);
  ASSERT_EQ(dirs.size(), 0);
  auto files = GetAllFiles(fs);
  ASSERT_EQ(files.size(), 0);
}

void GenericFileSystemTest::TestNormalizePath(FileSystem* fs) {
  // Canonical abstract paths should go through unchanged
  ASSERT_OK_AND_EQ("AB", fs->NormalizePath("AB"));
  ASSERT_OK_AND_EQ("AB/CD/efg", fs->NormalizePath("AB/CD/efg"));
}

void GenericFileSystemTest::TestCreateDir(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB"));
  ASSERT_OK(fs->CreateDir("AB/CD/EF"));  // Recursive
  if (!have_implicit_directories()) {
    // Non-recursive, parent doesn't exist
    ASSERT_RAISES(IOError, fs->CreateDir("AB/GH/IJ", false /* recursive */));
  }
  ASSERT_OK(fs->CreateDir("AB/GH", false /* recursive */));
  ASSERT_OK(fs->CreateDir("AB/GH/IJ", false /* recursive */));
  // Idempotency
  ASSERT_OK(fs->CreateDir("AB/GH/IJ", false /* recursive */));
  ASSERT_OK(fs->CreateDir("XY"));

  AssertAllDirs(fs, {"AB", "AB/CD", "AB/CD/EF", "AB/GH", "AB/GH/IJ", "XY"});
  AssertAllFiles(fs, {});

  // Cannot create a directory as child of a file
  CreateFile(fs, "AB/def", "");
  ASSERT_RAISES(IOError, fs->CreateDir("AB/def/EF/GH", true /* recursive */));
  ASSERT_RAISES(IOError, fs->CreateDir("AB/def/EF", false /* recursive */));

  AssertAllDirs(fs, {"AB", "AB/CD", "AB/CD/EF", "AB/GH", "AB/GH/IJ", "XY"});
  AssertAllFiles(fs, {"AB/def"});
}

void GenericFileSystemTest::TestDeleteDir(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB/CD/EF"));
  ASSERT_OK(fs->CreateDir("AB/GH/IJ"));
  CreateFile(fs, "AB/abc", "");
  CreateFile(fs, "AB/CD/def", "");
  CreateFile(fs, "AB/CD/EF/ghi", "");
  ASSERT_OK(fs->DeleteDir("AB/CD"));
  ASSERT_OK(fs->DeleteDir("AB/GH/IJ"));

  AssertAllDirs(fs, {"AB", "AB/GH"});
  AssertAllFiles(fs, {"AB/abc"});

  // File doesn't exist
  ASSERT_RAISES(IOError, fs->DeleteDir("AB/GH/IJ"));
  ASSERT_RAISES(IOError, fs->DeleteDir(""));

  AssertAllDirs(fs, {"AB", "AB/GH"});

  // Not a directory
  CreateFile(fs, "AB/def", "");
  ASSERT_RAISES(IOError, fs->DeleteDir("AB/def"));

  AssertAllDirs(fs, {"AB", "AB/GH"});
  AssertAllFiles(fs, {"AB/abc", "AB/def"});
}

void GenericFileSystemTest::TestDeleteDirContents(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB/CD/EF"));
  ASSERT_OK(fs->CreateDir("AB/GH/IJ"));
  CreateFile(fs, "AB/abc", "");
  CreateFile(fs, "AB/CD/def", "");
  CreateFile(fs, "AB/CD/EF/ghi", "");
  ASSERT_OK(fs->DeleteDirContents("AB/CD"));
  ASSERT_OK(fs->DeleteDirContents("AB/GH/IJ"));

  AssertAllDirs(fs, {"AB", "AB/CD", "AB/GH", "AB/GH/IJ"});
  AssertAllFiles(fs, {"AB/abc"});

  // Calling DeleteDirContents on root directory is disallowed
  ASSERT_RAISES(Invalid, fs->DeleteDirContents(""));
  ASSERT_RAISES(Invalid, fs->DeleteDirContents("/"));
  AssertAllDirs(fs, {"AB", "AB/CD", "AB/GH", "AB/GH/IJ"});
  AssertAllFiles(fs, {"AB/abc"});

  // Not a directory
  CreateFile(fs, "abc", "");
  ASSERT_RAISES(IOError, fs->DeleteDirContents("abc"));
  AssertAllFiles(fs, {"AB/abc", "abc"});
}

void GenericFileSystemTest::TestDeleteRootDirContents(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB/CD"));
  CreateFile(fs, "AB/abc", "");

  auto st = fs->DeleteRootDirContents();
  if (!st.ok()) {
    // Not all filesystems support deleting root directory contents
    ASSERT_TRUE(st.IsInvalid() || st.IsNotImplemented());
    AssertAllDirs(fs, {"AB", "AB/CD"});
    AssertAllFiles(fs, {"AB/abc"});
  } else {
    if (!have_flaky_directory_tree_deletion()) {
      AssertAllDirs(fs, {});
    }
    AssertAllFiles(fs, {});
  }
}

void GenericFileSystemTest::TestDeleteFile(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB"));
  CreateFile(fs, "AB/def", "");
  AssertAllDirs(fs, {"AB"});
  AssertAllFiles(fs, {"AB/def"});

  ASSERT_OK(fs->DeleteFile("AB/def"));
  AssertAllDirs(fs, {"AB"});
  AssertAllFiles(fs, {});

  CreateFile(fs, "abc", "data");
  AssertAllDirs(fs, {"AB"});
  AssertAllFiles(fs, {"abc"});

  ASSERT_OK(fs->DeleteFile("abc"));
  AssertAllDirs(fs, {"AB"});
  AssertAllFiles(fs, {});

  // File doesn't exist
  ASSERT_RAISES(IOError, fs->DeleteFile("abc"));
  ASSERT_RAISES(IOError, fs->DeleteFile("AB/def"));

  // Not a file
  ASSERT_RAISES(IOError, fs->DeleteFile("AB"));
  AssertAllDirs(fs, {"AB"});
  AssertAllFiles(fs, {});
}

void GenericFileSystemTest::TestDeleteFiles(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB"));
  CreateFile(fs, "abc", "");
  CreateFile(fs, "AB/def", "123");
  CreateFile(fs, "AB/ghi", "456");
  CreateFile(fs, "AB/jkl", "789");
  CreateFile(fs, "AB/mno", "789");
  AssertAllDirs(fs, {"AB"});
  AssertAllFiles(fs, {"AB/def", "AB/ghi", "AB/jkl", "AB/mno", "abc"});

  // All successful
  ASSERT_OK(fs->DeleteFiles({"abc", "AB/def"}));
  AssertAllDirs(fs, {"AB"});
  AssertAllFiles(fs, {"AB/ghi", "AB/jkl", "AB/mno"});

  // One error: file doesn't exist
  ASSERT_RAISES(IOError, fs->DeleteFiles({"xx", "AB/jkl"}));
  AssertAllDirs(fs, {"AB"});
  AssertAllFiles(fs, {"AB/ghi", "AB/mno"});

  // One error: not a file
  ASSERT_RAISES(IOError, fs->DeleteFiles({"AB", "AB/mno"}));
  AssertAllDirs(fs, {"AB"});
  AssertAllFiles(fs, {"AB/ghi"});
}

void GenericFileSystemTest::TestMoveFile(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB/CD"));
  ASSERT_OK(fs->CreateDir("EF"));
  CreateFile(fs, "abc", "data");
  std::vector<std::string> all_dirs{"AB", "AB/CD", "EF"};
  AssertAllDirs(fs, all_dirs);
  AssertAllFiles(fs, {"abc"});

  // Move inside root dir
  ASSERT_OK(fs->Move("abc", "def"));
  AssertAllDirs(fs, all_dirs);
  AssertAllFiles(fs, {"def"});
  AssertFileInfo(fs, "def", FileType::File, 4);
  AssertFileContents(fs, "def", "data");

  // Move out of root dir
  ASSERT_OK(fs->Move("def", "AB/CD/ghi"));
  AssertAllDirs(fs, all_dirs);
  AssertAllFiles(fs, {"AB/CD/ghi"});
  AssertFileInfo(fs, "AB/CD/ghi", FileType::File, 4);
  AssertFileContents(fs, "AB/CD/ghi", "data");

  ASSERT_OK(fs->Move("AB/CD/ghi", "EF/jkl"));
  AssertAllDirs(fs, all_dirs);
  AssertAllFiles(fs, {"EF/jkl"});
  AssertFileInfo(fs, "EF/jkl", FileType::File, 4);
  AssertFileContents(fs, "EF/jkl", "data");

  // Move back into root dir
  ASSERT_OK(fs->Move("EF/jkl", "mno"));
  AssertAllDirs(fs, all_dirs);
  AssertAllFiles(fs, {"mno"});
  AssertFileInfo(fs, "mno", FileType::File, 4);
  AssertFileContents(fs, "mno", "data");

  // Destination is a file => clobber
  CreateFile(fs, "AB/pqr", "other data");
  AssertAllFiles(fs, {"AB/pqr", "mno"});
  ASSERT_OK(fs->Move("mno", "AB/pqr"));
  AssertAllFiles(fs, {"AB/pqr"});
  AssertFileInfo(fs, "AB/pqr", FileType::File, 4);
  AssertFileContents(fs, "AB/pqr", "data");

  // Identical source and destination: allowed to succeed or raise IOError,
  // but should not lose data.
  auto err = fs->Move("AB/pqr", "AB/pqr");
  if (!err.ok()) {
    ASSERT_RAISES(IOError, err);
  }
  AssertAllFiles(fs, {"AB/pqr"});
  AssertFileInfo(fs, "AB/pqr", FileType::File, 4);
  AssertFileContents(fs, "AB/pqr", "data");

  // Source doesn't exist
  ASSERT_RAISES(IOError, fs->Move("abc", "def"));
  if (!have_implicit_directories()) {
    // Parent destination doesn't exist
    ASSERT_RAISES(IOError, fs->Move("AB/pqr", "XX/mno"));
  }
  // Parent destination is not a directory
  CreateFile(fs, "xxx", "");
  ASSERT_RAISES(IOError, fs->Move("AB/pqr", "xxx/mno"));
  if (!allow_write_file_over_dir()) {
    // Destination is a directory
    ASSERT_RAISES(IOError, fs->Move("AB/pqr", "EF"));
  }
  AssertAllDirs(fs, all_dirs);
  AssertAllFiles(fs, {"AB/pqr", "xxx"});
}

void GenericFileSystemTest::TestMoveDir(FileSystem* fs) {
  if (!allow_move_dir()) {
    // XXX skip
    return;
  }
  ASSERT_OK(fs->CreateDir("AB/CD"));
  ASSERT_OK(fs->CreateDir("EF"));
  CreateFile(fs, "AB/abc", "abc data");
  CreateFile(fs, "AB/CD/def", "def data");
  CreateFile(fs, "EF/ghi", "ghi data");
  AssertAllDirs(fs, {"AB", "AB/CD", "EF"});
  AssertAllFiles(fs, {"AB/CD/def", "AB/abc", "EF/ghi"});

  // Move inside root dir
  ASSERT_OK(fs->Move("AB", "GH"));
  AssertAllDirs(fs, {"EF", "GH", "GH/CD"});
  AssertAllFiles(fs, {"EF/ghi", "GH/CD/def", "GH/abc"});

  // Move out of root dir
  ASSERT_OK(fs->Move("GH", "EF/IJ"));
  AssertAllDirs(fs, {"EF", "EF/IJ", "EF/IJ/CD"});
  AssertAllFiles(fs, {"EF/IJ/CD/def", "EF/IJ/abc", "EF/ghi"});

  // Move back into root dir
  ASSERT_OK(fs->Move("EF/IJ", "KL"));
  AssertAllDirs(fs, {"EF", "KL", "KL/CD"});
  AssertAllFiles(fs, {"EF/ghi", "KL/CD/def", "KL/abc"});

  // Overwrite file with directory => untested (implementation-dependent)

  // Identical source and destination: allowed to succeed or raise IOError,
  // but should not lose data.
  Status st = fs->Move("KL", "KL");
  if (!st.ok()) {
    ASSERT_RAISES(IOError, st);
  }
  AssertAllDirs(fs, {"EF", "KL", "KL/CD"});
  AssertAllFiles(fs, {"EF/ghi", "KL/CD/def", "KL/abc"});

  // Destination is a non-empty directory
  ASSERT_RAISES(IOError, fs->Move("KL", "EF"));
  AssertAllDirs(fs, {"EF", "KL", "KL/CD"});
  AssertAllFiles(fs, {"EF/ghi", "KL/CD/def", "KL/abc"});

  // Cannot move directory inside itself
  ASSERT_RAISES(IOError, fs->Move("KL", "KL/ZZ"));

  // (other errors tested in TestMoveFile)

  // Contents didn't change
  AssertAllDirs(fs, {"EF", "KL", "KL/CD"});
  AssertFileContents(fs, "KL/abc", "abc data");
  AssertFileContents(fs, "KL/CD/def", "def data");
}

void GenericFileSystemTest::TestCopyFile(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB/CD"));
  ASSERT_OK(fs->CreateDir("EF"));
  CreateFile(fs, "AB/abc", "data");
  std::vector<std::string> all_dirs{"AB", "AB/CD", "EF"};

  // Copy into root dir
  ASSERT_OK(fs->CopyFile("AB/abc", "def"));
  AssertAllDirs(fs, all_dirs);
  AssertAllFiles(fs, {"AB/abc", "def"});

  // Copy out of root dir
  ASSERT_OK(fs->CopyFile("def", "EF/ghi"));
  AssertAllDirs(fs, all_dirs);
  AssertAllFiles(fs, {"AB/abc", "EF/ghi", "def"});

  // Overwrite contents for one file => other data shouldn't change
  CreateFile(fs, "def", "other data");
  AssertFileContents(fs, "AB/abc", "data");
  AssertFileContents(fs, "def", "other data");
  AssertFileContents(fs, "EF/ghi", "data");

  // Destination is a file => clobber
  ASSERT_OK(fs->CopyFile("def", "AB/abc"));
  AssertAllDirs(fs, all_dirs);
  AssertAllFiles(fs, {"AB/abc", "EF/ghi", "def"});
  AssertFileContents(fs, "AB/abc", "other data");
  AssertFileContents(fs, "def", "other data");
  AssertFileContents(fs, "EF/ghi", "data");

  // Identical source and destination: allowed to succeed or raise IOError,
  // but should not lose data.
  Status st = fs->CopyFile("def", "def");
  if (!st.ok()) {
    ASSERT_RAISES(IOError, st);
  }
  AssertAllFiles(fs, {"AB/abc", "EF/ghi", "def"});
  AssertFileContents(fs, "def", "other data");

  // Source doesn't exist
  ASSERT_RAISES(IOError, fs->CopyFile("abc", "xxx"));
  if (!allow_write_file_over_dir()) {
    // Destination is a non-empty directory
    ASSERT_RAISES(IOError, fs->CopyFile("def", "AB"));
  }
  if (!have_implicit_directories()) {
    // Parent destination doesn't exist
    ASSERT_RAISES(IOError, fs->CopyFile("AB/abc", "XX/mno"));
  }
  // Parent destination is not a directory
  ASSERT_RAISES(IOError, fs->CopyFile("AB/abc", "def/mno"));
  AssertAllDirs(fs, all_dirs);
  AssertAllFiles(fs, {"AB/abc", "EF/ghi", "def"});
}

void GenericFileSystemTest::TestGetFileInfo(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB/CD/EF"));
  CreateFile(fs, "AB/CD/ghi", "some data");
  CreateFile(fs, "AB/CD/jkl", "some other data");

  FileInfo info;
  TimePoint first_dir_time, first_file_time;

  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("AB"));
  AssertFileInfo(info, "AB", FileType::Directory);
  ASSERT_EQ(info.base_name(), "AB");
  ASSERT_EQ(info.size(), kNoSize);
  first_dir_time = info.mtime();
  if (have_directory_mtimes()) {
    ValidateTimePoint(first_dir_time);
  }

  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("AB/CD/EF"));
  AssertFileInfo(info, "AB/CD/EF", FileType::Directory);
  ASSERT_EQ(info.base_name(), "EF");
  ASSERT_EQ(info.size(), kNoSize);
  // AB/CD's creation can impact AB's modification time, however, AB/CD/EF's
  // creation doesn't, so AB/CD/EF's mtime should be after AB's.
  if (have_directory_mtimes()) {
    AssertDurationBetween(info.mtime() - first_dir_time, 0.0, kTimeSlack);
  }

  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("AB/CD/ghi"));
  AssertFileInfo(info, "AB/CD/ghi", FileType::File, 9);
  ASSERT_EQ(info.base_name(), "ghi");
  first_file_time = info.mtime();
  // AB/CD/ghi's creation doesn't impact AB's modification time,
  // so AB/CD/ghi's mtime should be after AB's.
  if (have_directory_mtimes()) {
    AssertDurationBetween(first_file_time - first_dir_time, 0.0, kTimeSlack);
  }
  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("AB/CD/jkl"));
  AssertFileInfo(info, "AB/CD/jkl", FileType::File, 15);
  // This file was created after the one above
  AssertDurationBetween(info.mtime() - first_file_time, 0.0, kTimeSlack);

  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("zz"));
  AssertFileInfo(info, "zz", FileType::NotFound);
  ASSERT_EQ(info.base_name(), "zz");
  ASSERT_EQ(info.size(), kNoSize);
  ASSERT_EQ(info.mtime(), kNoTime);
}

void GenericFileSystemTest::TestGetFileInfoVector(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB/CD"));
  CreateFile(fs, "AB/CD/ghi", "some data");

  std::vector<FileInfo> infos;
  TimePoint dir_time, file_time;
  ASSERT_OK_AND_ASSIGN(
      infos, fs->GetFileInfo({"AB", "AB/CD", "AB/zz", "zz", "XX/zz", "AB/CD/ghi"}));
  ASSERT_EQ(infos.size(), 6);
  AssertFileInfo(infos[0], "AB", FileType::Directory);
  dir_time = infos[0].mtime();
  if (have_directory_mtimes()) {
    ValidateTimePoint(dir_time);
  }
  AssertFileInfo(infos[1], "AB/CD", FileType::Directory);
  AssertFileInfo(infos[2], "AB/zz", FileType::NotFound);
  AssertFileInfo(infos[3], "zz", FileType::NotFound);
  AssertFileInfo(infos[4], "XX/zz", FileType::NotFound);
  ASSERT_EQ(infos[4].size(), kNoSize);
  ASSERT_EQ(infos[4].mtime(), kNoTime);
  AssertFileInfo(infos[5], "AB/CD/ghi", FileType::File, 9);
  file_time = infos[5].mtime();
  if (have_directory_mtimes()) {
    AssertDurationBetween(file_time - dir_time, 0.0, kTimeSlack);
  } else {
    ValidateTimePoint(file_time);
  }

  // Check the mtime is the same from one call to the other
  FileInfo info;
  if (have_directory_mtimes()) {
    ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("AB"));
    AssertFileInfo(info, "AB", FileType::Directory);
    ASSERT_EQ(info.mtime(), dir_time);
  }
  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("AB/CD/ghi"));
  AssertFileInfo(info, "AB/CD/ghi", FileType::File, 9);
  ASSERT_EQ(info.mtime(), file_time);
}

void GenericFileSystemTest::TestGetFileInfoSelector(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB/CD"));
  CreateFile(fs, "abc", "data");
  CreateFile(fs, "AB/def", "some data");
  CreateFile(fs, "AB/CD/ghi", "some other data");
  CreateFile(fs, "AB/CD/jkl", "yet other data");

  TimePoint first_dir_time, first_file_time;
  FileSelector s;
  s.base_dir = "";
  std::vector<FileInfo> infos;
  ASSERT_OK_AND_ASSIGN(infos, fs->GetFileInfo(s));
  // Need to sort results to make testing deterministic
  SortInfos(&infos);
  ASSERT_EQ(infos.size(), 2);
  AssertFileInfo(infos[0], "AB", FileType::Directory);
  first_dir_time = infos[0].mtime();
  if (have_directory_mtimes()) {
    ValidateTimePoint(first_dir_time);
  }
  AssertFileInfo(infos[1], "abc", FileType::File, 4);

  s.base_dir = "AB";
  ASSERT_OK_AND_ASSIGN(infos, fs->GetFileInfo(s));
  SortInfos(&infos);
  ASSERT_EQ(infos.size(), 2);
  AssertFileInfo(infos[0], "AB/CD", FileType::Directory);
  AssertFileInfo(infos[1], "AB/def", FileType::File, 9);

  s.base_dir = "AB/CD";
  ASSERT_OK_AND_ASSIGN(infos, fs->GetFileInfo(s));
  SortInfos(&infos);
  ASSERT_EQ(infos.size(), 2);
  AssertFileInfo(infos[0], "AB/CD/ghi", FileType::File, 15);
  AssertFileInfo(infos[1], "AB/CD/jkl", FileType::File, 14);
  first_file_time = infos[0].mtime();
  if (have_directory_mtimes()) {
    AssertDurationBetween(first_file_time - first_dir_time, 0.0, kTimeSlack);
  }
  AssertDurationBetween(infos[1].mtime() - first_file_time, 0.0, kTimeSlack);

  // Recursive
  s.base_dir = "AB";
  s.recursive = true;
  ASSERT_OK_AND_ASSIGN(infos, fs->GetFileInfo(s));
  SortInfos(&infos);
  ASSERT_EQ(infos.size(), 4);
  AssertFileInfo(infos[0], "AB/CD", FileType::Directory);
  AssertFileInfo(infos[1], "AB/CD/ghi", FileType::File, first_file_time, 15);
  AssertFileInfo(infos[2], "AB/CD/jkl", FileType::File, 14);
  AssertFileInfo(infos[3], "AB/def", FileType::File, 9);

  // Check the mtime is the same from one call to the other
  FileInfo info;
  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("AB"));
  AssertFileInfo(info, "AB", FileType::Directory, first_dir_time);
  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("AB/CD/ghi"));
  AssertFileInfo(info, "AB/CD/ghi", FileType::File, first_file_time, 15);

  // Doesn't exist
  s.base_dir = "XX";
  ASSERT_RAISES(IOError, fs->GetFileInfo(s));
  s.allow_not_found = true;
  ASSERT_OK_AND_ASSIGN(infos, fs->GetFileInfo(s));
  ASSERT_EQ(infos.size(), 0);
  s.allow_not_found = false;

  // Not a dir
  s.base_dir = "abc";
  ASSERT_RAISES(IOError, fs->GetFileInfo(s));
}

void GetSortedInfos(FileSystem* fs, FileSelector s, std::vector<FileInfo>& infos) {
  ASSERT_OK_AND_ASSIGN(infos, fs->GetFileInfo(s));
  // Clear mtime & size for easier testing.
  for_each(infos.begin(), infos.end(), [](FileInfo& info) {
    info.set_mtime(kNoTime);
    info.set_size(kNoSize);
  });
  SortInfos(&infos);
}

void GenericFileSystemTest::TestGetFileInfoSelectorWithRecursion(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("01/02/03/04"));
  ASSERT_OK(fs->CreateDir("AA"));
  CreateFile(fs, "00.file", "00");
  CreateFile(fs, "01/01.file", "01");
  CreateFile(fs, "AA/AA.file", "aa");
  CreateFile(fs, "01/02/02.file", "02");
  CreateFile(fs, "01/02/03/03.file", "03");
  CreateFile(fs, "01/02/03/04/04.file", "04");

  std::vector<FileInfo> infos;
  FileSelector s;

  s.base_dir = "";
  s.recursive = false;
  GetSortedInfos(fs, s, infos);
  EXPECT_THAT(infos, ElementsAre(File("00.file"), Dir("01"), Dir("AA")));

  // recursive should prevail on max_recursion
  s.max_recursion = 9000;
  GetSortedInfos(fs, s, infos);
  EXPECT_THAT(infos, ElementsAre(File("00.file"), Dir("01"), Dir("AA")));

  // recursive but no traversal
  s.recursive = true;
  s.max_recursion = 0;
  GetSortedInfos(fs, s, infos);
  EXPECT_THAT(infos, ElementsAre(File("00.file"), Dir("01"), Dir("AA")));

  s.recursive = true;
  s.max_recursion = 1;
  GetSortedInfos(fs, s, infos);
  EXPECT_THAT(infos, ElementsAre(File("00.file"), Dir("01"), File("01/01.file"),
                                 Dir("01/02"), Dir("AA"), File("AA/AA.file")));

  s.recursive = true;
  s.max_recursion = 2;
  GetSortedInfos(fs, s, infos);
  EXPECT_THAT(infos, ElementsAre(File("00.file"), Dir("01"), File("01/01.file"),
                                 Dir("01/02"), File("01/02/02.file"), Dir("01/02/03"),
                                 Dir("AA"), File("AA/AA.file")));

  s.base_dir = "01";
  s.recursive = false;
  GetSortedInfos(fs, s, infos);
  EXPECT_THAT(infos, ElementsAre(File("01/01.file"), Dir("01/02")));

  s.base_dir = "01";
  s.recursive = true;
  s.max_recursion = 1;
  GetSortedInfos(fs, s, infos);
  EXPECT_THAT(infos, ElementsAre(File("01/01.file"), Dir("01/02"), File("01/02/02.file"),
                                 Dir("01/02/03")));

  // All-in
  s.base_dir = "";
  s.recursive = true;
  s.max_recursion = INT32_MAX;
  GetSortedInfos(fs, s, infos);
  EXPECT_THAT(
      infos, ElementsAre(File("00.file"), Dir("01"), File("01/01.file"), Dir("01/02"),
                         File("01/02/02.file"), Dir("01/02/03"), File("01/02/03/03.file"),
                         Dir("01/02/03/04"), File("01/02/03/04/04.file"), Dir("AA"),
                         File("AA/AA.file")));
}

void GenericFileSystemTest::TestOpenOutputStream(FileSystem* fs) {
  std::shared_ptr<io::OutputStream> stream;

  ASSERT_OK_AND_ASSIGN(stream, fs->OpenOutputStream("abc"));
  ASSERT_OK_AND_EQ(0, stream->Tell());
  ASSERT_FALSE(stream->closed());
  ASSERT_OK(stream->Close());
  ASSERT_TRUE(stream->closed());
  AssertAllDirs(fs, {});
  AssertAllFiles(fs, {"abc"});
  AssertFileContents(fs, "abc", "");

  // Parent does not exist
  if (!have_implicit_directories()) {
    ASSERT_RAISES(IOError, fs->OpenOutputStream("AB/def"));
  }
  AssertAllDirs(fs, {});
  AssertAllFiles(fs, {"abc"});

  // Several writes
  ASSERT_OK(fs->CreateDir("CD"));
  ASSERT_OK_AND_ASSIGN(stream, fs->OpenOutputStream("CD/ghi"));
  ASSERT_OK(stream->Write("some "));
  ASSERT_OK(stream->Write(Buffer::FromString("data")));
  ASSERT_OK_AND_EQ(9, stream->Tell());
  ASSERT_OK(stream->Close());
  AssertAllDirs(fs, {"CD"});
  AssertAllFiles(fs, {"CD/ghi", "abc"});
  AssertFileContents(fs, "CD/ghi", "some data");

  // Overwrite
  ASSERT_OK_AND_ASSIGN(stream, fs->OpenOutputStream("CD/ghi"));
  ASSERT_OK(stream->Write("overwritten"));
  ASSERT_OK(stream->Close());
  AssertAllDirs(fs, {"CD"});
  AssertAllFiles(fs, {"CD/ghi", "abc"});
  AssertFileContents(fs, "CD/ghi", "overwritten");

  ASSERT_RAISES(Invalid, stream->Write("x"));  // Stream is closed

  if (!allow_write_file_over_dir()) {
    // Cannot turn dir into file
    ASSERT_RAISES(IOError, fs->OpenOutputStream("CD"));
    AssertAllDirs(fs, {"CD"});
  }
}

void GenericFileSystemTest::TestOpenAppendStream(FileSystem* fs) {
  if (!allow_append_to_file()) {
    // XXX skip
    return;
  }
  std::shared_ptr<io::OutputStream> stream;

  ASSERT_OK_AND_ASSIGN(stream, fs->OpenAppendStream("abc"));
  ASSERT_OK_AND_EQ(0, stream->Tell());
  ASSERT_OK(stream->Write("some "));
  ASSERT_OK(stream->Write(Buffer::FromString("data")));
  ASSERT_OK_AND_EQ(9, stream->Tell());
  ASSERT_OK(stream->Close());
  AssertAllDirs(fs, {});
  AssertAllFiles(fs, {"abc"});
  AssertFileContents(fs, "abc", "some data");

  ASSERT_OK_AND_ASSIGN(stream, fs->OpenAppendStream("abc"));
  ASSERT_OK_AND_EQ(9, stream->Tell());
  ASSERT_OK(stream->Write(" appended"));
  ASSERT_OK(stream->Close());
  AssertAllDirs(fs, {});
  AssertAllFiles(fs, {"abc"});
  AssertFileContents(fs, "abc", "some data appended");

  ASSERT_RAISES(Invalid, stream->Write("x"));  // Stream is closed
}

void GenericFileSystemTest::TestOpenInputStream(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB"));
  CreateFile(fs, "AB/abc", "some data");

  std::shared_ptr<io::InputStream> stream;
  std::shared_ptr<Buffer> buffer;
  ASSERT_OK_AND_ASSIGN(stream, fs->OpenInputStream("AB/abc"));
  ASSERT_OK_AND_ASSIGN(buffer, stream->Read(4));
  AssertBufferEqual(*buffer, "some");
  ASSERT_OK_AND_ASSIGN(buffer, stream->Read(6));
  AssertBufferEqual(*buffer, " data");
  ASSERT_OK_AND_ASSIGN(buffer, stream->Read(1));
  AssertBufferEqual(*buffer, "");
  ASSERT_OK(stream->Close());
  ASSERT_RAISES(Invalid, stream->Read(1));  // Stream is closed

  // File does not exist
  ASSERT_RAISES(IOError, fs->OpenInputStream("AB/def"));
  ASSERT_RAISES(IOError, fs->OpenInputStream("def"));

  // Cannot open directory
  ASSERT_RAISES(IOError, fs->OpenInputStream("AB"));
}

void GenericFileSystemTest::TestOpenInputStreamWithFileInfo(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB"));
  CreateFile(fs, "AB/abc", "some data");

  ASSERT_OK_AND_ASSIGN(auto info, fs->GetFileInfo("AB/abc"));

  ASSERT_OK_AND_ASSIGN(auto stream, fs->OpenInputStream(info));
  ASSERT_OK_AND_ASSIGN(auto buffer, stream->Read(9));
  AssertBufferEqual(*buffer, "some data");

  // Passing an incomplete FileInfo should also work
  info.set_type(FileType::Unknown);
  info.set_size(kNoSize);
  info.set_mtime(kNoTime);
  ASSERT_OK_AND_ASSIGN(stream, fs->OpenInputStream(info));
  ASSERT_OK_AND_ASSIGN(buffer, stream->Read(4));
  AssertBufferEqual(*buffer, "some");

  // File does not exist
  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("zzzzt"));
  ASSERT_RAISES(IOError, fs->OpenInputStream(info));
  // (same, with incomplete FileInfo)
  info.set_type(FileType::Unknown);
  ASSERT_RAISES(IOError, fs->OpenInputStream(info));

  // Cannot open directory
  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("AB"));
  ASSERT_RAISES(IOError, fs->OpenInputStream(info));
}

void GenericFileSystemTest::TestOpenInputFile(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB"));
  CreateFile(fs, "AB/abc", "some other data");

  std::shared_ptr<io::RandomAccessFile> file;
  std::shared_ptr<Buffer> buffer;
  ASSERT_OK_AND_ASSIGN(file, fs->OpenInputFile("AB/abc"));
  ASSERT_OK_AND_ASSIGN(buffer, file->ReadAt(5, 6));
  AssertBufferEqual(*buffer, "other ");
  ASSERT_OK_AND_EQ(15, file->GetSize());
  ASSERT_OK(file->Close());
  ASSERT_RAISES(Invalid, file->ReadAt(1, 1));  // Stream is closed

  // File does not exist
  ASSERT_RAISES(IOError, fs->OpenInputFile("AB/def"));
  ASSERT_RAISES(IOError, fs->OpenInputFile("def"));

  // Cannot open directory
  ASSERT_RAISES(IOError, fs->OpenInputFile("AB"));
}

void GenericFileSystemTest::TestOpenInputFileWithFileInfo(FileSystem* fs) {
  ASSERT_OK(fs->CreateDir("AB"));
  CreateFile(fs, "AB/abc", "some data");

  ASSERT_OK_AND_ASSIGN(auto info, fs->GetFileInfo("AB/abc"));

  ASSERT_OK_AND_ASSIGN(auto file, fs->OpenInputFile(info));
  ASSERT_OK_AND_EQ(9, file->GetSize());
  ASSERT_OK_AND_ASSIGN(auto buffer, file->Read(9));
  AssertBufferEqual(*buffer, "some data");

  // Passing an incomplete FileInfo should also work
  info.set_type(FileType::Unknown);
  info.set_size(kNoSize);
  info.set_mtime(kNoTime);
  ASSERT_OK_AND_ASSIGN(file, fs->OpenInputFile(info));
  ASSERT_OK_AND_EQ(9, file->GetSize());
  ASSERT_OK_AND_ASSIGN(buffer, file->Read(4));
  AssertBufferEqual(*buffer, "some");

  // File does not exist
  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("zzzzt"));
  ASSERT_RAISES(IOError, fs->OpenInputFile(info));
  // (same, with incomplete FileInfo)
  info.set_type(FileType::Unknown);
  ASSERT_RAISES(IOError, fs->OpenInputFile(info));

  // Cannot open directory
  ASSERT_OK_AND_ASSIGN(info, fs->GetFileInfo("AB"));
  ASSERT_RAISES(IOError, fs->OpenInputFile(info));
}

#define GENERIC_FS_TEST_DEFINE(FUNC_NAME) \
  void GenericFileSystemTest::FUNC_NAME() { FUNC_NAME(GetEmptyFileSystem().get()); }

GENERIC_FS_TEST_DEFINE(TestEmpty)
GENERIC_FS_TEST_DEFINE(TestNormalizePath)
GENERIC_FS_TEST_DEFINE(TestCreateDir)
GENERIC_FS_TEST_DEFINE(TestDeleteDir)
GENERIC_FS_TEST_DEFINE(TestDeleteDirContents)
GENERIC_FS_TEST_DEFINE(TestDeleteRootDirContents)
GENERIC_FS_TEST_DEFINE(TestDeleteFile)
GENERIC_FS_TEST_DEFINE(TestDeleteFiles)
GENERIC_FS_TEST_DEFINE(TestMoveFile)
GENERIC_FS_TEST_DEFINE(TestMoveDir)
GENERIC_FS_TEST_DEFINE(TestCopyFile)
GENERIC_FS_TEST_DEFINE(TestGetFileInfo)
GENERIC_FS_TEST_DEFINE(TestGetFileInfoVector)
GENERIC_FS_TEST_DEFINE(TestGetFileInfoSelector)
GENERIC_FS_TEST_DEFINE(TestGetFileInfoSelectorWithRecursion)
GENERIC_FS_TEST_DEFINE(TestOpenOutputStream)
GENERIC_FS_TEST_DEFINE(TestOpenAppendStream)
GENERIC_FS_TEST_DEFINE(TestOpenInputStream)
GENERIC_FS_TEST_DEFINE(TestOpenInputStreamWithFileInfo)
GENERIC_FS_TEST_DEFINE(TestOpenInputFile)
GENERIC_FS_TEST_DEFINE(TestOpenInputFileWithFileInfo)

#undef GENERIC_FS_TEST_DEFINE

}  // namespace fs
}  // namespace arrow
