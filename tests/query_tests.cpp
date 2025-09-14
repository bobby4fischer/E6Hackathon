#include <gtest/gtest.h>
#include "query/parser.hpp"
#include "query/executor.hpp"
#include <vector>
#include <algorithm>

using namespace aqe::query;

class QueryTest : public ::testing::Test {
protected:
    std::vector<DataRow> sample_data;
    void SetUp() override {
        sample_data.push_back({ {{"category", "A"}, {"value", "100"}} });
        sample_data.push_back({ {{"category", "B"}, {"value", "200"}} });
        sample_data.push_back({ {{"category", "A"}, {"value", "150"}} });
        sample_data.push_back({ {{"category", "B"}, {"value", "250"}} });
        sample_data.push_back({ {{"category", "C"}, {"value", "300"}} });
    }
};

// --- Parser Tests ---
TEST_F(QueryTest, ParserHandlesSimpleCount) {
    QueryParser parser;
    auto query = parser.parse("SELECT COUNT(value) FROM data");
    ASSERT_EQ(query->columns.size(), 1);
    // UPDATED: Access the first element of the columns vector
    EXPECT_EQ(query->columns[0].name, "value");
    EXPECT_EQ(query->columns[0].aggregation, AggregationType::COUNT);
    EXPECT_EQ(query->table_name, "data");
}

TEST_F(QueryTest, ParserHandlesMultipleAggregationsWithAliases) {
    QueryParser parser;
    auto query = parser.parse("SELECT SUM(value) AS total, AVG(value) as average FROM my_table");
    ASSERT_EQ(query->columns.size(), 2);
    EXPECT_EQ(query->table_name, "my_table");
    // UPDATED: Check properties for the first column (SUM)
    EXPECT_EQ(query->columns[0].name, "value");
    EXPECT_EQ(query->columns[0].alias, "total");
    EXPECT_EQ(query->columns[0].aggregation, AggregationType::SUM);
    // UPDATED: Check properties for the second column (AVG)
    EXPECT_EQ(query->columns[1].name, "value");
    EXPECT_EQ(query->columns[1].alias, "average");
    EXPECT_EQ(query->columns[1].aggregation, AggregationType::AVG);
}

TEST_F(QueryTest, ParserHandlesGroupBy) {
    QueryParser parser;
    auto query = parser.parse("SELECT category, AVG(value) FROM data GROUP BY category");
    ASSERT_EQ(query->columns.size(), 2);
    ASSERT_EQ(query->group_by_columns.size(), 1);
    // UPDATED: Access the first element of the group_by_columns vector
    EXPECT_EQ(query->group_by_columns[0], "category");
}

TEST_F(QueryTest, ParserHandlesSamplingClause) {
    QueryParser parser;
    auto query = parser.parse("SELECT COUNT(*) FROM data SAMPLE 15.5%");
    EXPECT_EQ(query->sampling.method, SamplingMethod::RANDOM);
    EXPECT_DOUBLE_EQ(query->sampling.rate, 0.155);
}

TEST_F(QueryTest, ParserThrowsOnMissingFromClause) {
    QueryParser parser;
    EXPECT_THROW(parser.parse("SELECT value"), ParseError);
}

// --- Executor Tests ---
TEST_F(QueryTest, ExecutorHandlesExactCount) {
    QueryParser parser;
    QueryExecutor executor;
    auto query = parser.parse("SELECT COUNT(value) FROM data");
    auto result = executor.execute(*query, sample_data);
    ASSERT_EQ(result->getRows().size(), 1);
    ASSERT_EQ(result->getRows()[0].size(), 1);
    // UPDATED: Access the specific cell [row][column] for the result
    EXPECT_DOUBLE_EQ(std::stod(result->getRows()[0][0]), 5.0);
    EXPECT_FALSE(result->isApproximate());
}

TEST_F(QueryTest, ExecutorHandlesExactSum) {
    QueryParser parser;
    QueryExecutor executor;
    auto query = parser.parse("SELECT SUM(value) FROM data");
    auto result = executor.execute(*query, sample_data);
    ASSERT_EQ(result->getRows().size(), 1);
    // 100 + 200 + 150 + 250 + 300 = 1000
    EXPECT_DOUBLE_EQ(std::stod(result->getRows()[0][0]), 1000.0);
}

TEST_F(QueryTest, ExecutorHandlesGroupByAndAvg) {
    QueryParser parser;
    QueryExecutor executor;
    auto query = parser.parse("SELECT category, AVG(value) FROM data GROUP BY category");
    auto result = executor.execute(*query, sample_data);
    ASSERT_EQ(result->getRows().size(), 3);
    
    auto result_rows = result->getRows();
    // UPDATED: Sort by the first column (category name) to have a predictable order
    std::sort(result_rows.begin(), result_rows.end(),
        [](const std::vector<std::string>& a, const std::vector<std::string>& b) {
        return a[0] < b[0];
    });

    // UPDATED: Check each sorted row and its columns
    // Category A: (100 + 150) / 2 = 125
    EXPECT_EQ(result_rows[0][0], "A");
    EXPECT_DOUBLE_EQ(std::stod(result_rows[0][1]), 125.0);
    // Category B: (200 + 250) / 2 = 225
    EXPECT_EQ(result_rows[1][0], "B");
    EXPECT_DOUBLE_EQ(std::stod(result_rows[1][1]), 225.0);
    // Category C: 300 / 1 = 300
    EXPECT_EQ(result_rows[2][0], "C");
    EXPECT_DOUBLE_EQ(std::stod(result_rows[2][1]), 300.0);
}

TEST_F(QueryTest, ExecutorHandlesMinAndMax) {
    QueryParser parser;
    QueryExecutor executor;
    auto query = parser.parse("SELECT MIN(value), MAX(value) FROM data");
    auto result = executor.execute(*query, sample_data);
    ASSERT_EQ(result->getRows().size(), 1);
    // UPDATED: A single row has two columns for MIN and MAX
    ASSERT_EQ(result->getRows()[0].size(), 2);
    EXPECT_DOUBLE_EQ(std::stod(result->getRows()[0][0]), 100.0); // Min
    EXPECT_DOUBLE_EQ(std::stod(result->getRows()[0][1]), 300.0); // Max
}