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

#include "arrow/flight/sql/example/sqlite_tables_schema_batch_reader.h"

#include <sqlite3.h>

#include <sstream>

#include "arrow/flight/sql/column_metadata.h"
#include "arrow/flight/sql/example/sqlite_server.h"
#include "arrow/flight/sql/example/sqlite_statement.h"
#include "arrow/flight/sql/server.h"
#include "arrow/ipc/writer.h"
#include "arrow/record_batch.h"

namespace arrow {
namespace flight {
namespace sql {
namespace example {

std::shared_ptr<Schema> SqliteTablesWithSchemaBatchReader::schema() const {
  return SqlSchema::GetTablesSchemaWithIncludedSchema();
}

Status SqliteTablesWithSchemaBatchReader::ReadNext(std::shared_ptr<RecordBatch>* batch) {
  std::stringstream schema_query;

  schema_query
      << "SELECT table_name, name, type, [notnull] FROM pragma_table_info(table_name)"
      << "JOIN(" << main_query_ << ") order by table_name";

  std::shared_ptr<example::SqliteStatement> schema_statement;
  ARROW_ASSIGN_OR_RAISE(schema_statement,
                        example::SqliteStatement::Create(db_, schema_query.str()))

  std::shared_ptr<RecordBatch> first_batch;

  ARROW_RETURN_NOT_OK(reader_->ReadNext(&first_batch));

  if (!first_batch) {
    *batch = NULLPTR;
    return Status::OK();
  }

  const std::shared_ptr<Array> table_name_array =
      first_batch->GetColumnByName("table_name");

  BinaryBuilder schema_builder;

  auto* string_array = reinterpret_cast<StringArray*>(table_name_array.get());

  std::vector<std::shared_ptr<Field>> column_fields;
  for (int i = 0; i < table_name_array->length(); i++) {
    const std::string& table_name = string_array->GetString(i);

    while (sqlite3_step(schema_statement->GetSqlite3Stmt()) == SQLITE_ROW) {
      std::string sqlite_table_name = std::string(reinterpret_cast<const char*>(
          sqlite3_column_text(schema_statement->GetSqlite3Stmt(), 0)));
      if (sqlite_table_name == table_name) {
        const char* column_name = reinterpret_cast<const char*>(
            sqlite3_column_text(schema_statement->GetSqlite3Stmt(), 1));
        const char* column_type = reinterpret_cast<const char*>(
            sqlite3_column_text(schema_statement->GetSqlite3Stmt(), 2));
        int nullable = sqlite3_column_int(schema_statement->GetSqlite3Stmt(), 3);

        const ColumnMetadata& column_metadata = GetColumnMetadata(
            GetSqlTypeFromTypeName(column_type), sqlite_table_name.c_str());
        column_fields.push_back(arrow::field(column_name, GetArrowType(column_type),
                                             nullable == 0,
                                             column_metadata.metadata_map()));
      }
    }
    const arrow::Result<std::shared_ptr<Buffer>>& value =
        ipc::SerializeSchema(*arrow::schema(column_fields));

    std::shared_ptr<Buffer> schema_buffer;
    ARROW_ASSIGN_OR_RAISE(schema_buffer, value);

    column_fields.clear();
    ARROW_RETURN_NOT_OK(
        schema_builder.Append(::arrow::util::string_view(*schema_buffer)));
  }

  std::shared_ptr<Array> schema_array;
  ARROW_RETURN_NOT_OK(schema_builder.Finish(&schema_array));

  ARROW_ASSIGN_OR_RAISE(*batch, first_batch->AddColumn(4, "table_schema", schema_array));

  return Status::OK();
}

}  // namespace example
}  // namespace sql
}  // namespace flight
}  // namespace arrow
