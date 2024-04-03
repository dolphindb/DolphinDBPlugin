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

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "arrow/io/interfaces.h"
#include "arrow/memory_pool.h"
#include "arrow/record_batch.h"
#include "arrow/status.h"
#include "arrow/type.h"
#include "arrow/util/visibility.h"

namespace arrow {

namespace adapters {

namespace orc {

/// \class ORCFileReader
/// \brief Read an Arrow Table or RecordBatch from an ORC file.
class ARROW_EXPORT ORCFileReader {
 public:
  ~ORCFileReader();

  /// \brief Creates a new ORC reader.
  ///
  /// \param[in] file the data source
  /// \param[in] pool a MemoryPool to use for buffer allocations
  /// \param[out] reader the returned reader object
  /// \return Status
  static Status Open(const std::shared_ptr<io::RandomAccessFile>& file, MemoryPool* pool,
                     std::unique_ptr<ORCFileReader>* reader);

  /// \brief Return the schema read from the ORC file
  ///
  /// \param[out] out the returned Schema object
  Status ReadSchema(std::shared_ptr<Schema>* out);

  /// \brief Read the file as a Table
  ///
  /// The table will be composed of one record batch per stripe.
  ///
  /// \param[out] out the returned Table
  Status Read(std::shared_ptr<Table>* out);

  /// \brief Read the file as a Table
  ///
  /// The table will be composed of one record batch per stripe.
  ///
  /// \param[in] schema the Table schema
  /// \param[out] out the returned Table
  Status Read(const std::shared_ptr<Schema>& schema, std::shared_ptr<Table>* out);

  /// \brief Read the file as a Table
  ///
  /// The table will be composed of one record batch per stripe.
  ///
  /// \param[in] include_indices the selected field indices to read
  /// \param[out] out the returned Table
  Status Read(const std::vector<int>& include_indices, std::shared_ptr<Table>* out);

  /// \brief Read the file as a Table
  ///
  /// The table will be composed of one record batch per stripe.
  ///
  /// \param[in] schema the Table schema
  /// \param[in] include_indices the selected field indices to read
  /// \param[out] out the returned Table
  Status Read(const std::shared_ptr<Schema>& schema,
              const std::vector<int>& include_indices, std::shared_ptr<Table>* out);

  /// \brief Read a single stripe as a RecordBatch
  ///
  /// \param[in] stripe the stripe index
  /// \param[out] out the returned RecordBatch
  Status ReadStripe(int64_t stripe, std::shared_ptr<RecordBatch>* out);

  /// \brief Read a single stripe as a RecordBatch
  ///
  /// \param[in] stripe the stripe index
  /// \param[in] include_indices the selected field indices to read
  /// \param[out] out the returned RecordBatch
  Status ReadStripe(int64_t stripe, const std::vector<int>& include_indices,
                    std::shared_ptr<RecordBatch>* out);

  /// \brief Seek to designated row. Invoke NextStripeReader() after seek
  ///        will return stripe reader starting from designated row.
  ///
  /// \param[in] row_number the rows number to seek
  Status Seek(int64_t row_number);

  /// \brief Get a stripe level record batch iterator with specified row count
  ///         in each record batch. NextStripeReader serves as a fine grain
  ///         alternative to ReadStripe which may cause OOM issue by loading
  ///         the whole stripes into memory.
  ///
  /// \param[in] batch_size the number of rows each record batch contains in
  ///            record batch iteration.
  /// \param[out] out the returned stripe reader
  Status NextStripeReader(int64_t batch_size, std::shared_ptr<RecordBatchReader>* out);

  /// \brief Get a stripe level record batch iterator with specified row count
  ///         in each record batch. NextStripeReader serves as a fine grain
  ///         alternative to ReadStripe which may cause OOM issue by loading
  ///         the whole stripes into memory.
  ///
  /// \param[in] batch_size Get a stripe level record batch iterator with specified row
  /// count in each record batch.
  ///
  /// \param[in] include_indices the selected field indices to read
  /// \param[out] out the returned stripe reader
  Status NextStripeReader(int64_t batch_size, const std::vector<int>& include_indices,
                          std::shared_ptr<RecordBatchReader>* out);

  /// \brief The number of stripes in the file
  int64_t NumberOfStripes();

  /// \brief The number of rows in the file
  int64_t NumberOfRows();

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
  ORCFileReader();
};

}  // namespace orc

}  // namespace adapters

}  // namespace arrow
