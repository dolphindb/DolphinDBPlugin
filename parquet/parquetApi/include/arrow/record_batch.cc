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

#include "arrow/record_batch.h"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "arrow/array.h"
#include "arrow/array/validate.h"
#include "arrow/pretty_print.h"
#include "arrow/status.h"
#include "arrow/table.h"
#include "arrow/type.h"
#include "arrow/util/atomic_shared_ptr.h"
#include "arrow/util/iterator.h"
#include "arrow/util/logging.h"
#include "arrow/util/vector.h"

namespace arrow {

Result<std::shared_ptr<RecordBatch>> RecordBatch::AddColumn(
    int i, std::string field_name, const std::shared_ptr<Array>& column) const {
  auto field = ::arrow::field(std::move(field_name), column->type());
  return AddColumn(i, field, column);
}

std::shared_ptr<Array> RecordBatch::GetColumnByName(const std::string& name) const {
  auto i = schema_->GetFieldIndex(name);
  return i == -1 ? NULLPTR : column(i);
}

int RecordBatch::num_columns() const { return schema_->num_fields(); }

/// \class SimpleRecordBatch
/// \brief A basic, non-lazy in-memory record batch
class SimpleRecordBatch : public RecordBatch {
 public:
  SimpleRecordBatch(std::shared_ptr<Schema> schema, int64_t num_rows,
                    std::vector<std::shared_ptr<Array>> columns)
      : RecordBatch(std::move(schema), num_rows), boxed_columns_(std::move(columns)) {
    columns_.resize(boxed_columns_.size());
    for (size_t i = 0; i < columns_.size(); ++i) {
      columns_[i] = boxed_columns_[i]->data();
    }
  }

  SimpleRecordBatch(const std::shared_ptr<Schema>& schema, int64_t num_rows,
                    std::vector<std::shared_ptr<ArrayData>> columns)
      : RecordBatch(std::move(schema), num_rows), columns_(std::move(columns)) {
    boxed_columns_.resize(schema_->num_fields());
  }

  std::shared_ptr<Array> column(int i) const override {
    std::shared_ptr<Array> result = internal::atomic_load(&boxed_columns_[i]);
    if (!result) {
      result = MakeArray(columns_[i]);
      internal::atomic_store(&boxed_columns_[i], result);
    }
    return result;
  }

  std::shared_ptr<ArrayData> column_data(int i) const override { return columns_[i]; }

  ArrayDataVector column_data() const override { return columns_; }

  Result<std::shared_ptr<RecordBatch>> AddColumn(
      int i, const std::shared_ptr<Field>& field,
      const std::shared_ptr<Array>& column) const override {
    ARROW_CHECK(field != nullptr);
    ARROW_CHECK(column != nullptr);

    if (!field->type()->Equals(column->type())) {
      return Status::Invalid("Column data type ", field->type()->name(),
                             " does not match field data type ", column->type()->name());
    }
    if (column->length() != num_rows_) {
      return Status::Invalid(
          "Added column's length must match record batch's length. Expected length ",
          num_rows_, " but got length ", column->length());
    }

    ARROW_ASSIGN_OR_RAISE(auto new_schema, schema_->AddField(i, field));

    return RecordBatch::Make(new_schema, num_rows_,
                             internal::AddVectorElement(columns_, i, column->data()));
  }

  Result<std::shared_ptr<RecordBatch>> RemoveColumn(int i) const override {
    ARROW_ASSIGN_OR_RAISE(auto new_schema, schema_->RemoveField(i));

    return RecordBatch::Make(new_schema, num_rows_,
                             internal::DeleteVectorElement(columns_, i));
  }

  std::shared_ptr<RecordBatch> ReplaceSchemaMetadata(
      const std::shared_ptr<const KeyValueMetadata>& metadata) const override {
    auto new_schema = schema_->WithMetadata(metadata);
    return RecordBatch::Make(new_schema, num_rows_, columns_);
  }

  std::shared_ptr<RecordBatch> Slice(int64_t offset, int64_t length) const override {
    std::vector<std::shared_ptr<ArrayData>> arrays;
    arrays.reserve(num_columns());
    for (const auto& field : columns_) {
      arrays.emplace_back(field->Slice(offset, length));
    }
    int64_t num_rows = std::min(num_rows_ - offset, length);
    return std::make_shared<SimpleRecordBatch>(schema_, num_rows, std::move(arrays));
  }

  Status Validate() const override {
    if (static_cast<int>(columns_.size()) != schema_->num_fields()) {
      return Status::Invalid("Number of columns did not match schema");
    }
    return RecordBatch::Validate();
  }

 private:
  std::vector<std::shared_ptr<ArrayData>> columns_;

  // Caching boxed array data
  mutable std::vector<std::shared_ptr<Array>> boxed_columns_;
};

RecordBatch::RecordBatch(const std::shared_ptr<Schema>& schema, int64_t num_rows)
    : schema_(schema), num_rows_(num_rows) {}

std::shared_ptr<RecordBatch> RecordBatch::Make(
    std::shared_ptr<Schema> schema, int64_t num_rows,
    std::vector<std::shared_ptr<Array>> columns) {
  DCHECK_EQ(schema->num_fields(), static_cast<int>(columns.size()));
  return std::make_shared<SimpleRecordBatch>(std::move(schema), num_rows, columns);
}

std::shared_ptr<RecordBatch> RecordBatch::Make(
    std::shared_ptr<Schema> schema, int64_t num_rows,
    std::vector<std::shared_ptr<ArrayData>> columns) {
  DCHECK_EQ(schema->num_fields(), static_cast<int>(columns.size()));
  return std::make_shared<SimpleRecordBatch>(std::move(schema), num_rows,
                                             std::move(columns));
}

Result<std::shared_ptr<RecordBatch>> RecordBatch::FromStructArray(
    const std::shared_ptr<Array>& array) {
  if (array->type_id() != Type::STRUCT) {
    return Status::Invalid("Cannot construct record batch from array of type ",
                           *array->type());
  }
  if (array->null_count() != 0) {
    return Status::Invalid(
        "Unable to construct record batch from a StructArray with non-zero nulls.");
  }
  return Make(arrow::schema(array->type()->fields()), array->length(),
              array->data()->child_data);
}

Result<std::shared_ptr<StructArray>> RecordBatch::ToStructArray() const {
  if (num_columns() != 0) {
    return StructArray::Make(columns(), schema()->fields());
  }
  return std::make_shared<StructArray>(arrow::struct_({}), num_rows_,
                                       std::vector<std::shared_ptr<Array>>{},
                                       /*null_bitmap=*/nullptr,
                                       /*null_count=*/0,
                                       /*offset=*/0);
}

std::vector<std::shared_ptr<Array>> RecordBatch::columns() const {
  std::vector<std::shared_ptr<Array>> children(num_columns());
  for (int i = 0; i < num_columns(); ++i) {
    children[i] = column(i);
  }
  return children;
}

const std::string& RecordBatch::column_name(int i) const {
  return schema_->field(i)->name();
}

bool RecordBatch::Equals(const RecordBatch& other, bool check_metadata) const {
  if (num_columns() != other.num_columns() || num_rows_ != other.num_rows()) {
    return false;
  }

  if (check_metadata) {
    if (!schema_->Equals(*other.schema(), /*check_metadata=*/true)) {
      return false;
    }
  }

  for (int i = 0; i < num_columns(); ++i) {
    if (!column(i)->Equals(other.column(i))) {
      return false;
    }
  }

  return true;
}

bool RecordBatch::ApproxEquals(const RecordBatch& other) const {
  if (num_columns() != other.num_columns() || num_rows_ != other.num_rows()) {
    return false;
  }

  for (int i = 0; i < num_columns(); ++i) {
    if (!column(i)->ApproxEquals(other.column(i))) {
      return false;
    }
  }

  return true;
}

std::shared_ptr<RecordBatch> RecordBatch::Slice(int64_t offset) const {
  return Slice(offset, this->num_rows() - offset);
}

std::string RecordBatch::ToString() const {
  std::stringstream ss;
  ARROW_CHECK_OK(PrettyPrint(*this, 0, &ss));
  return ss.str();
}

Status RecordBatch::Validate() const {
  for (int i = 0; i < num_columns(); ++i) {
    const auto& array = *this->column(i);
    if (array.length() != num_rows_) {
      return Status::Invalid("Number of rows in column ", i,
                             " did not match batch: ", array.length(), " vs ", num_rows_);
    }
    const auto& schema_type = *schema_->field(i)->type();
    if (!array.type()->Equals(schema_type)) {
      return Status::Invalid("Column ", i,
                             " type not match schema: ", array.type()->ToString(), " vs ",
                             schema_type.ToString());
    }
    RETURN_NOT_OK(internal::ValidateArray(array));
  }
  return Status::OK();
}

Status RecordBatch::ValidateFull() const {
  RETURN_NOT_OK(Validate());
  for (int i = 0; i < num_columns(); ++i) {
    const auto& array = *this->column(i);
    RETURN_NOT_OK(internal::ValidateArrayData(array));
  }
  return Status::OK();
}

// ----------------------------------------------------------------------
// Base record batch reader

Status RecordBatchReader::ReadAll(std::vector<std::shared_ptr<RecordBatch>>* batches) {
  while (true) {
    std::shared_ptr<RecordBatch> batch;
    RETURN_NOT_OK(ReadNext(&batch));
    if (!batch) {
      break;
    }
    batches->emplace_back(std::move(batch));
  }
  return Status::OK();
}

Status RecordBatchReader::ReadAll(std::shared_ptr<Table>* table) {
  std::vector<std::shared_ptr<RecordBatch>> batches;
  RETURN_NOT_OK(ReadAll(&batches));
  return Table::FromRecordBatches(schema(), std::move(batches)).Value(table);
}

class SimpleRecordBatchReader : public RecordBatchReader {
 public:
  SimpleRecordBatchReader(Iterator<std::shared_ptr<RecordBatch>> it,
                          std::shared_ptr<Schema> schema)
      : schema_(std::move(schema)), it_(std::move(it)) {}

  SimpleRecordBatchReader(std::vector<std::shared_ptr<RecordBatch>> batches,
                          std::shared_ptr<Schema> schema)
      : schema_(std::move(schema)), it_(MakeVectorIterator(std::move(batches))) {}

  Status ReadNext(std::shared_ptr<RecordBatch>* batch) override {
    return it_.Next().Value(batch);
  }

  std::shared_ptr<Schema> schema() const override { return schema_; }

 protected:
  std::shared_ptr<Schema> schema_;
  Iterator<std::shared_ptr<RecordBatch>> it_;
};

Result<std::shared_ptr<RecordBatchReader>> RecordBatchReader::Make(
    std::vector<std::shared_ptr<RecordBatch>> batches, std::shared_ptr<Schema> schema) {
  if (schema == nullptr) {
    if (batches.size() == 0 || batches[0] == nullptr) {
      return Status::Invalid("Cannot infer schema from empty vector or nullptr");
    }

    schema = batches[0]->schema();
  }

  return std::make_shared<SimpleRecordBatchReader>(std::move(batches), schema);
}

}  // namespace arrow
