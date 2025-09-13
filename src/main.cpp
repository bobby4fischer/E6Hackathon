#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "query/parser.hpp"
#include "query/executor.hpp"
#include "utils/benchmark.hpp"
#include "utils/string_utils.hpp"

using namespace aqe::query;
using namespace aqe::utils;

// Helper function to load data from CSV
std::vector<DataRow> loadDataFromCSV(const std::string& filename) {
    std::vector<DataRow> data;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open data file: " << filename << std::endl;
        return data;
    }

    std::string line;
    std::getline(file, line); // Read header
    std::vector<std::string> headers = splitCSV(line);
    
    while (std::getline(file, line)) {
        DataRow row;
        auto values = splitCSV(line);
        for (size_t i = 0; i < headers.size() && i < values.size(); ++i) {
            row.values[headers[i]] = values[i];
        }
        data.push_back(row);
    }
    return data;
}
    // UPDATED printResults FUNCTION
    void printResults(const QueryResult& result) {
        // Print column headers
        for (const auto& col_name : result.getColumnNames()) {
            std::cout << col_name << "\t";
        }
        std::cout << "\n" << std::string(50, '-') << std::endl;

        // Print rows
        for (const auto& row : result.getRows()) {
            for (const auto& cell : row) {
                std::cout << cell << "\t";
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
        {"Exact COUNT", "SELECT COUNT(value) FROM data"},
        {"Approximate COUNT (10% sample)", "SELECT COUNT(value) FROM data SAMPLE 10%"},
        {"Group By with AVG", "SELECT category, AVG(value) FROM data GROUP BY category"},
        {"Stratified Sampling", "SELECT category, AVG(value) FROM data GROUP BY category SAMPLE STRATIFIED BY category 20%"}
    };
    
    for (const auto& [description, query_str] : queries) {
        std::cout << "\nExecuting: " << description << "..." << std::flush;
        
        try {
            Timer timer;
            QueryExecutor executor;
            auto query = parser.parse(query_str);
            auto result = executor.execute(*query, data);
            
            std::cout << " done." << std::endl; // Print done after execution
            printResults(*result);
            std::cout << "Execution time: " << timer.elapsed() << "ms\n";
            
        } catch (const std::exception& e) {
            std::cerr << "\nError: " << e.what() << "\n";
        }
    }
    
    return 0;
}