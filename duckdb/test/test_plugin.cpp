// Test file for DuckDB plugin
// This file contains unit tests for the DuckDB plugin functionality

#include "gtest/gtest.h"
#include "duckdb.hpp"
#include <fstream>
#include <iostream>

// Mock DolphinDB headers for testing
// In actual implementation, these would be the real DolphinDB headers

class DuckDBPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary in-memory DuckDB database for testing
        db = std::make_unique<duckdb::DuckDB>(:memory:);
        conn = std::make_unique<duckdb::Connection>(*db);

        // Create test tables
        createTestTables();
    }

    void TearDown() override {
        conn.reset();
        db.reset();
    }

    void createTestTables() {
        // Create a simple test table
        conn->Query("CREATE TABLE test_table (id INTEGER, name VARCHAR, value DOUBLE)");
        conn->Query("INSERT INTO test_table VALUES (1, 'test1', 10.5)");
        conn->Query("INSERT INTO test_table VALUES (2, 'test2', 20.5)");
        conn->Query("INSERT INTO test_table VALUES (3, 'test3', 30.5)");

        // Create a table with various data types
        conn->Query(R"(
            CREATE TABLE types_table (
                bool_col BOOLEAN,
                tinyint_col TINYINT,
                smallint_col SMALLINT,
                int_col INTEGER,
                bigint_col BIGINT,
                float_col FLOAT,
                double_col DOUBLE,
                varchar_col VARCHAR,
                date_col DATE,
                time_col TIME,
                timestamp_col TIMESTAMP
            )
        )");
        conn->Query(R"(
            INSERT INTO types_table VALUES
            (true, 127, 32767, 2147483647, 9223372036854775807, 1.5, 2.5, 'test', '2024-01-01', '12:30:45', '2024-01-01 12:30:45')
        )");

        // Create a table for performance testing
        conn->Query("CREATE TABLE large_table (id INTEGER, value DOUBLE, timestamp TIMESTAMP)");
        for(int i = 0; i < 10000; i++) {
            conn->Query("INSERT INTO large_table VALUES (" + std::to_string(i) + ", " + std::to_string(i * 1.5) + ", '2024-01-01 00:00:00')");
        }
    }

    std::unique_ptr<duckdb::DuckDB> db;
    std::unique_ptr<duckdb::Connection> conn;
};

// Test basic connection
TEST_F(DuckDBPluginTest, BasicConnection) {
    ASSERT_NE(db, nullptr);
    ASSERT_NE(conn, nullptr);
    EXPECT_TRUE(db != nullptr);
}

// Test table creation and data insertion
TEST_F(DuckDBPluginTest, TableCreation) {
    auto result = conn->Query("SELECT * FROM test_table");
    EXPECT_FALSE(result->HasError());
    EXPECT_EQ(result->RowCount(), 3);
}

// Test data type conversion
TEST_F(DuckDBPluginTest, DataTypeConversion) {
    auto result = conn->Query("SELECT * FROM types_table");
    EXPECT_FALSE(result->HasError());
    EXPECT_EQ(result->RowCount(), 1);

    auto chunk = result->Fetch();
    EXPECT_NE(chunk, nullptr);
    EXPECT_EQ(chunk->size(), 1);

    // Verify boolean
    duckdb::Value bool_val = chunk->data[0].GetValue(0);
    EXPECT_TRUE(bool_val.GetValue<bool>());

    // Verify integer types
    duckdb::Value tinyint_val = chunk->data[1].GetValue(0);
    EXPECT_EQ(tinyint_val.GetValue<int8_t>(), 127);

    duckdb::Value smallint_val = chunk->data[2].GetValue(0);
    EXPECT_EQ(smallint_val.GetValue<int16_t>(), 32767);

    duckdb::Value int_val = chunk->data[3].GetValue(0);
    EXPECT_EQ(int_val.GetValue<int32_t>(), 2147483647);

    duckdb::Value bigint_val = chunk->data[4].GetValue(0);
    EXPECT_EQ(bigint_val.GetValue<int64_t>(), 9223372036854775807);

    // Verify floating point types
    duckdb::Value float_val = chunk->data[5].GetValue(0);
    EXPECT_FLOAT_EQ(float_val.GetValue<float>(), 1.5);

    duckdb::Value double_val = chunk->data[6].GetValue(0);
    EXPECT_DOUBLE_EQ(double_val.GetValue<double>(), 2.5);

    // Verify string
    duckdb::Value varchar_val = chunk->data[7].GetValue(0);
    EXPECT_EQ(varchar_val.GetValue<std::string>(), "test");

    // Verify date
    duckdb::Value date_val = chunk->data[8].GetValue(0);
    EXPECT_EQ(date_val.GetValue<duckdb::date_t>().days, 19758); // 2024-01-01

    // Verify time
    duckdb::Value time_val = chunk->data[9].GetValue(0);
    EXPECT_GT(time_val.GetValue<duckdb::dtime_t>().micros, 0);

    // Verify timestamp
    duckdb::Value timestamp_val = chunk->data[10].GetValue(0);
    EXPECT_GT(timestamp_val.GetValue<duckdb::timestamp_t>().micros, 0);
}

// Test query execution
TEST_F(DuckDBPluginTest, QueryExecution) {
    auto result = conn->Query("SELECT COUNT(*) as count FROM test_table");
    EXPECT_FALSE(result->HasError());

    auto chunk = result->Fetch();
    EXPECT_NE(chunk, nullptr);

    duckdb::Value count_val = chunk->data[0].GetValue(0);
    EXPECT_EQ(count_val.GetValue<int64_t>(), 3);
}

// Test pagination
TEST_F(DuckDBPluginTest, Pagination) {
    // Test LIMIT
    auto result = conn->Query("SELECT * FROM test_table LIMIT 2");
    EXPECT_FALSE(result->HasError());
    EXPECT_EQ(result->RowCount(), 2);

    // Test OFFSET
    result = conn->Query("SELECT * FROM test_table OFFSET 1");
    EXPECT_FALSE(result->HasError());
    EXPECT_EQ(result->RowCount(), 2);

    // Test LIMIT and OFFSET
    result = conn->Query("SELECT * FROM test_table LIMIT 1 OFFSET 1");
    EXPECT_FALSE(result->HasError());
    EXPECT_EQ(result->RowCount(), 1);
}

// Test large table performance
TEST_F(DuckDBPluginTest, LargeTablePerformance) {
    auto start = std::chrono::high_resolution_clock::now();

    auto result = conn->Query("SELECT * FROM large_table");
    EXPECT_FALSE(result->HasError());
    EXPECT_EQ(result->RowCount(), 10000);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Query executed in " << duration.count() << "ms" << std::endl;

    // Performance assertion: should complete in less than 5 seconds
    EXPECT_LT(duration.count(), 5000);
}

// Test schema extraction
TEST_F(DuckDBPluginTest, SchemaExtraction) {
    auto result = conn->Query("SELECT * FROM test_table LIMIT 0");
    EXPECT_FALSE(result->HasError());

    auto& types = result->types();
    auto& names = result->names();

    EXPECT_EQ(types.size(), 3);
    EXPECT_EQ(names.size(), 3);

    EXPECT_EQ(names[0], "id");
    EXPECT_EQ(names[1], "name");
    EXPECT_EQ(names[2], "value");

    EXPECT_EQ(types[0].id(), duckdb::LogicalTypeId::INTEGER);
    EXPECT_EQ(types[1].id(), duckdb::LogicalTypeId::VARCHAR);
    EXPECT_EQ(types[2].id(), duckdb::LogicalTypeId::DOUBLE);
}

// Test NULL handling
TEST_F(DuckDBPluginTest, NullHandling) {
    conn->Query("CREATE TABLE null_table (id INTEGER, value DOUBLE)");
    conn->Query("INSERT INTO null_table VALUES (1, NULL)");
    conn->Query("INSERT INTO null_table VALUES (2, 10.5)");

    auto result = conn->Query("SELECT * FROM null_table");
    EXPECT_FALSE(result->HasError());
    EXPECT_EQ(result->RowCount(), 2);

    auto chunk = result->Fetch();
    EXPECT_NE(chunk, nullptr);

    // First row should have NULL value
    duckdb::Value val1 = chunk->data[1].GetValue(0);
    EXPECT_TRUE(val1.IsNull());

    // Second row should have non-NULL value
    duckdb::Value val2 = chunk->data[1].GetValue(1);
    EXPECT_FALSE(val2.IsNull());
    EXPECT_DOUBLE_EQ(val2.GetValue<double>(), 10.5);
}

// Test error handling
TEST_F(DuckDBPluginTest, ErrorHandling) {
    // Test syntax error
    auto result = conn->Query("SELECT * FROM nonexistent_table");
    EXPECT_TRUE(result->HasError());

    // Test invalid SQL
    result = conn->Query("INVALID SQL");
    EXPECT_TRUE(result->HasError());
}

// Main function to run all tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
