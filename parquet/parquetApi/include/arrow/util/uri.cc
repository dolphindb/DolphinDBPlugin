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

#include "arrow/util/uri.h"

#include <cstring>
#include <sstream>
#include <vector>

#include "arrow/util/string_view.h"
#include "arrow/util/value_parsing.h"
#include "arrow/vendored/uriparser/Uri.h"

namespace arrow {
namespace internal {

namespace {

util::string_view TextRangeToView(const UriTextRangeStructA& range) {
  if (range.first == nullptr) {
    return "";
  } else {
    return {range.first, static_cast<size_t>(range.afterLast - range.first)};
  }
}

std::string TextRangeToString(const UriTextRangeStructA& range) {
  return std::string(TextRangeToView(range));
}

// There can be a difference between an absent field and an empty field.
// For example, in "unix:/tmp/foo", the host is absent, while in
// "unix:///tmp/foo", the host is empty but present.
// This function helps distinguish.
bool IsTextRangeSet(const UriTextRangeStructA& range) { return range.first != nullptr; }

#ifdef _WIN32
bool IsDriveSpec(util::string_view s) {
  return (s.length() >= 2 && s[1] == ':' &&
          ((s[0] >= 'A' && s[0] <= 'Z') || (s[0] >= 'a' && s[0] <= 'z')));
}
#endif

}  // namespace

std::string UriEscape(const std::string& s) {
  if (s.empty()) {
    // Avoid passing null pointer to uriEscapeExA
    return s;
  }
  std::string escaped;
  escaped.resize(3 * s.length());

  auto end = uriEscapeExA(s.data(), s.data() + s.length(), &escaped[0],
                          /*spaceToPlus=*/URI_FALSE, /*normalizeBreaks=*/URI_FALSE);
  escaped.resize(end - &escaped[0]);
  return escaped;
}

struct Uri::Impl {
  Impl() : string_rep_(""), port_(-1) { memset(&uri_, 0, sizeof(uri_)); }

  ~Impl() { uriFreeUriMembersA(&uri_); }

  void Reset() {
    uriFreeUriMembersA(&uri_);
    memset(&uri_, 0, sizeof(uri_));
    data_.clear();
    string_rep_.clear();
    path_segments_.clear();
    port_ = -1;
  }

  const std::string& KeepString(const std::string& s) {
    data_.push_back(s);
    return data_.back();
  }

  UriUriA uri_;
  // Keep alive strings that uriparser stores pointers to
  std::vector<std::string> data_;
  std::string string_rep_;
  int32_t port_;
  std::vector<util::string_view> path_segments_;
  bool is_file_uri_;
  bool is_absolute_path_;
};

Uri::Uri() : impl_(new Impl) {}

Uri::~Uri() {}

Uri::Uri(Uri&& u) : impl_(std::move(u.impl_)) {}

Uri& Uri::operator=(Uri&& u) {
  impl_ = std::move(u.impl_);
  return *this;
}

std::string Uri::scheme() const { return TextRangeToString(impl_->uri_.scheme); }

std::string Uri::host() const { return TextRangeToString(impl_->uri_.hostText); }

bool Uri::has_host() const { return IsTextRangeSet(impl_->uri_.hostText); }

std::string Uri::port_text() const { return TextRangeToString(impl_->uri_.portText); }

int32_t Uri::port() const { return impl_->port_; }

std::string Uri::username() const {
  auto userpass = TextRangeToView(impl_->uri_.userInfo);
  auto sep_pos = userpass.find_first_of(':');
  if (sep_pos == util::string_view::npos) {
    return std::string(userpass);
  } else {
    return std::string(userpass.substr(0, sep_pos));
  }
}

std::string Uri::password() const {
  auto userpass = TextRangeToView(impl_->uri_.userInfo);
  auto sep_pos = userpass.find_first_of(':');
  if (sep_pos == util::string_view::npos) {
    return std::string();
  } else {
    return std::string(userpass.substr(sep_pos + 1));
  }
}

std::string Uri::path() const {
  const auto& segments = impl_->path_segments_;

  bool must_prepend_slash = impl_->is_absolute_path_;
#ifdef _WIN32
  // On Windows, "file:///C:/foo" should have path "C:/foo", not "/C:/foo",
  // despite it being absolute.
  // (see https://tools.ietf.org/html/rfc8089#page-13)
  if (impl_->is_absolute_path_ && impl_->is_file_uri_ && segments.size() > 0 &&
      IsDriveSpec(segments[0])) {
    must_prepend_slash = false;
  }
#endif

  std::stringstream ss;
  if (must_prepend_slash) {
    ss << "/";
  }
  bool first = true;
  for (const auto seg : segments) {
    if (!first) {
      ss << "/";
    }
    first = false;
    ss << seg;
  }
  return std::move(ss).str();
}

std::string Uri::query_string() const { return TextRangeToString(impl_->uri_.query); }

Result<std::vector<std::pair<std::string, std::string>>> Uri::query_items() const {
  const auto& query = impl_->uri_.query;
  UriQueryListA* query_list;
  int item_count;
  std::vector<std::pair<std::string, std::string>> items;

  if (query.first == nullptr) {
    return items;
  }
  if (uriDissectQueryMallocA(&query_list, &item_count, query.first, query.afterLast) !=
      URI_SUCCESS) {
    return Status::Invalid("Cannot parse query string: '", query_string(), "'");
  }
  std::unique_ptr<UriQueryListA, decltype(&uriFreeQueryListA)> query_guard(
      query_list, uriFreeQueryListA);

  items.reserve(item_count);
  while (query_list != nullptr) {
    if (query_list->value != nullptr) {
      items.emplace_back(query_list->key, query_list->value);
    } else {
      items.emplace_back(query_list->key, "");
    }
    query_list = query_list->next;
  }
  return items;
}

const std::string& Uri::ToString() const { return impl_->string_rep_; }

Status Uri::Parse(const std::string& uri_string) {
  impl_->Reset();

  const auto& s = impl_->KeepString(uri_string);
  impl_->string_rep_ = s;
  const char* error_pos;
  if (uriParseSingleUriExA(&impl_->uri_, s.data(), s.data() + s.size(), &error_pos) !=
      URI_SUCCESS) {
    return Status::Invalid("Cannot parse URI: '", uri_string, "'");
  }

  const auto scheme = TextRangeToView(impl_->uri_.scheme);
  if (scheme.empty()) {
    return Status::Invalid("URI has empty scheme: '", uri_string, "'");
  }
  impl_->is_file_uri_ = (scheme == "file");

  // Gather path segments
  auto path_seg = impl_->uri_.pathHead;
  while (path_seg != nullptr) {
    impl_->path_segments_.push_back(TextRangeToView(path_seg->text));
    path_seg = path_seg->next;
  }

  // Decide whether URI path is absolute
  impl_->is_absolute_path_ = false;
  if (impl_->uri_.absolutePath == URI_TRUE) {
    impl_->is_absolute_path_ = true;
  } else if (has_host() && impl_->path_segments_.size() > 0) {
    // When there's a host (even empty), uriparser considers the path relative.
    // Several URI parsers for Python all consider it absolute, though.
    // For example, the path for "file:///tmp/foo" is "/tmp/foo", not "tmp/foo".
    // Similarly, the path for "file://localhost/" is "/".
    // However, the path for "file://localhost" is "".
    impl_->is_absolute_path_ = true;
  }
#ifdef _WIN32
  // There's an exception on Windows: "file:/C:foo/bar" is relative.
  if (impl_->is_file_uri_ && impl_->path_segments_.size() > 0) {
    const auto& first_seg = impl_->path_segments_[0];
    if (IsDriveSpec(first_seg) && (first_seg.length() >= 3 && first_seg[2] != '/')) {
      impl_->is_absolute_path_ = false;
    }
  }
#endif

  if (impl_->is_file_uri_ && !impl_->is_absolute_path_) {
    return Status::Invalid("File URI cannot be relative: '", uri_string, "'");
  }

  // Parse port number
  auto port_text = TextRangeToView(impl_->uri_.portText);
  if (port_text.size()) {
    uint16_t port_num;
    if (!ParseValue<UInt16Type>(port_text.data(), port_text.size(), &port_num)) {
      return Status::Invalid("Invalid port number '", port_text, "' in URI '", uri_string,
                             "'");
    }
    impl_->port_ = port_num;
  }

  return Status::OK();
}

}  // namespace internal
}  // namespace arrow
