#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

#include "query/parser.hpp"
#include "query/executor.hpp"
#include "utils/benchmark.hpp"
#include "utils/string_utils.hpp"

using namespace aqe::query;
using namespace aqe::utils;

std::vector<DataRow> loadDataFromCSV(const std::string& filename) {
    std::vector<DataRow> data;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open data file: " << filename << std::endl;
        return data;
    }

    std::string line;
    std::getline(file, line);
    std::vector<std::string> headers = splitCSV(line);
    
    while (std::getline(file, line)) {
        if (line.empty() || line == "\r") continue;
        DataRow row;
        auto values = splitCSV(line);
        for (size_t i = 0; i < headers.size() && i < values.size(); ++i) {
            row.values[headers[i]] = values[i];
        }
        data.push_back(row);
    }
    return data;
}

void printResults(const QueryResult& result) {
    const auto& headers = result.getColumnNames();
    const auto& rows = result.getRows();
    
    if (headers.empty()) return;

    std::vector<size_t> widths;
    for (const auto& header : headers) {
        widths.push_back(header.length());
    }

    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size() && i < widths.size(); ++i) {
            if (row[i].length() > widths[i]) {
                widths[i] = row[i].length();
            }
        }
    }

    // Print headers
    for (size_t i = 0; i < headers.size(); ++i) {
        std::cout << std::left << std::setw(widths[i] + 2) << headers[i];
    }
    std::cout << std::endl;

    // Print separator
    for (size_t width : widths) {
        std::cout << std::string(width + 2, '-');
    }
    std::cout << std::endl;

    // Print rows
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size() && i < widths.size(); ++i) {
            std::cout << std::left << std::setw(widths[i] + 2) << row[i];
        }
        std::cout << std::endl;
    }

    if (result.isApproximate()) {
        std::cout << "\nNote: Results are approximate." << std::endl;
    }
}

int main() {
    std::cout << "Approximate Query Engine Demo\n";
    std::cout << "----------------------------\n";
    auto data = loadDataFromCSV("data/large_data.csv");
    if (data.empty()) return 1;
    std::cout << "Loaded " << data.size() << " rows from data/large_data.csv\n";
    
    QueryParser parser;
    
    std::vector<std::pair<std::string, std::string>> queries = {
        // {"Total Row Count (Exact)", "SELECT COUNT(*) FROM data"},
        // {"Approximate Total Row Count (10% Sample)","SELECT COUNT(*) FROM data SAMPLE 10%"},
        // {"Total Sum of 'value' (Exact)", "SELECT SUM(value) FROM data"},
        // {"Approximate Total Sum of 'value' (10% Sample)", "SELECT SUM(value) FROM data SAMPLE 10%"},
        // {"Overall Average of 'value' (Exact)", "SELECT AVG(value) FROM data"},
        // {"Approximate Overall Average of 'value' (10% Sample)", "SELECT AVG(value) FROM data SAMPLE 10%"},
        // {"Min and Max of 'value' (Exact)", "SELECT MIN(value), MAX(value) FROM data"},
        // {"Approximate Min and Max of 'value' (10% Sample)", "SELECT MIN(value), MAX(value) FROM data SAMPLE 10%"},


        {"GROUP BY with COUNT, SUM and AVG", "SELECT category, COUNT(*), SUM(value), AVG(value) FROM data GROUP BY category"},
        {"Approximate GROUP BY with COUNT, SUM and AVG (20% Sample)", "SELECT category, COUNT(*), SUM(value), AVG(value) FROM data GROUP BY category SAMPLE 20%"},
    
        // {"Complex Query with Aliases", "SELECT category, COUNT(*) AS item_count, AVG(value) AS average_price FROM data GROUP BY category"}
    };
    
    for (const auto& [description, query_str] : queries) {
        std::cout << "\nExecuting: " << description << "...\n";
        
        try {
            Timer timer;

            QueryExecutor executor; // A fresh executor for each query
            auto query = parser.parse(query_str);
            auto result = executor.execute(*query, data);
            
            printResults(*result);
            std::cout << "Execution time: " << timer.elapsed() << "ms\n";
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    
    return 0;
}