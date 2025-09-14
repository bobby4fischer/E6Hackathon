#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <regex>
#include <sstream>
#include "../utils/string_utils.hpp"   

namespace aqe {
namespace query {

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& message) : std::runtime_error(message) {}
};

// ... (Enums and Structs are the same) ...
enum class AggregationType { COUNT, SUM, AVG, MIN, MAX, NONE };
enum class SamplingMethod { NONE, RANDOM, SYSTEMATIC, RESERVOIR, STRATIFIED };

    struct Column {
        std::string name;
        std::string alias;
        AggregationType aggregation;
        bool is_star;

        Column(const std::string& n, const std::string& a = "", 
            AggregationType agg = AggregationType::NONE)
            : name(n), alias(a), aggregation(agg), is_star(n == "*") {}
    };

    struct Sampling {
        SamplingMethod method;
        double rate;
        size_t size;
        std::string stratification_column;

        Sampling() : method(SamplingMethod::NONE), rate(1.0), size(0) {}

        void validate() const {
            if (method == SamplingMethod::RANDOM && (rate <= 0.0 || rate > 1.0)) {
                throw ParseError("Sampling rate must be between 0 and 1");
            }
            if (method == SamplingMethod::RESERVOIR && size == 0) {
                throw ParseError("Reservoir sample size must be greater than 0");
            }
        }
    };

    class Query {
    public:
        std::vector<Column> columns;
        std::string table_name;
        std::vector<std::string> group_by_columns;
        Sampling sampling;
 

        void validate() const {
            if (table_name.empty()) {
                throw ParseError("Table name cannot be empty");
            }
             
            bool has_aggregation = false;
            bool has_non_agg_column = false;
            for (const auto& col : columns) {
                if (col.aggregation != AggregationType::NONE) {
                    has_aggregation = true;
                } else if (col.name != "*") {
                    has_non_agg_column = true;
                }
            }
            if (has_non_agg_column && has_aggregation && group_by_columns.empty()) {
                throw ParseError("Queries with both aggregated and non-aggregated columns require a GROUP BY clause.");
            } 
            sampling.validate();
        }
    };
    class QueryParser {
        public:
            QueryParser() {}

            std::unique_ptr<Query> parse(const std::string& query_str) {
                auto query = std::make_unique<Query>();
                try {
                    std::string upper_query = aqe::utils::toUpper(query_str);
                    size_t select_pos = findKeyword(upper_query, "SELECT");
                    size_t from_pos = findKeyword(upper_query, "FROM");

                    std::string select_clause = query_str.substr(select_pos + 6, from_pos - (select_pos + 6));
                    parseColumns(query.get(), select_clause);

                    std::string rest_of_query = query_str.substr(from_pos + 4);
                    parseFromAndOtherClauses(query.get(), rest_of_query);

                    query->validate();
                    return query;
                } catch (const std::exception& e) {
                    throw ParseError(std::string("Failed to parse query: ") + e.what());
                }
            }

        private: 
            void parseColumns(Query* query, const std::string& columns_str) {
                std::stringstream ss(columns_str);
                std::string part;
                while(std::getline(ss, part, ',')) {
                    std::string column_str = aqe::utils::trim(part);
                    if (column_str.empty()) continue;

                    static const std::regex agg_regex(R"((COUNT|SUM|AVG|MIN|MAX)\s*\(([^)]+)\)(?:\s+AS\s+([\w]+))?)", std::regex::icase);
                    std::smatch matches;

                    if (std::regex_match(column_str, matches, agg_regex)) {
                        std::string func = aqe::utils::toUpper(matches[1].str());
                        std::string inner_col = aqe::utils::trim(matches[2].str());
                        std::string alias = matches[3].matched ? matches[3].str() : "";

                        if (alias.empty()) {
                            alias = func + "(" + aqe::utils::toUpper(inner_col) + ")";
                        }

                        AggregationType agg_type = AggregationType::NONE;
                        if (func == "COUNT") agg_type = AggregationType::COUNT;
                        else if (func == "SUM") agg_type = AggregationType::SUM;
                        else if (func == "AVG") agg_type = AggregationType::AVG;
                        else if (func == "MIN") agg_type = AggregationType::MIN;
                        else if (func == "MAX") agg_type = AggregationType::MAX;
                        
                        query->columns.emplace_back(inner_col, alias, agg_type);
                    } else {
                        query->columns.emplace_back(column_str);
                    }
                }
            }
 
            void parseFromAndOtherClauses(Query* query, const std::string& rest_str) {
                std::string upper_rest = aqe::utils::toUpper(rest_str);
                size_t group_by_pos = upper_rest.find("GROUP BY");
                size_t sample_pos = upper_rest.find("SAMPLE");

                size_t table_end = std::min(group_by_pos, sample_pos);
                query->table_name = aqe::utils::trim(rest_str.substr(0, table_end));

                if (group_by_pos != std::string::npos) {
                    size_t clause_end = (sample_pos > group_by_pos) ? sample_pos : std::string::npos;
                    parseGroupBy(query, rest_str.substr(group_by_pos + 8, clause_end - (group_by_pos + 8)));
                }
                
                if (sample_pos != std::string::npos) {
                    parseSampling(query, rest_str.substr(sample_pos + 6));
                }
            }

            void parseGroupBy(Query* query, const std::string& group_by_str) {
                std::stringstream ss(group_by_str);
                std::string column;
                while (std::getline(ss, column, ',')) {
                    std::string trimmed_column = aqe::utils::trim(column);
                    if (!trimmed_column.empty()) {
                        query->group_by_columns.push_back(trimmed_column);
                    }
                }
            }
            size_t findKeyword(const std::string& query, const std::string& keyword) {
                size_t pos = query.find(keyword);
                if (pos == std::string::npos) {
                    throw ParseError("Missing " + keyword + " clause");
                }
                return pos;
            }

            // UPDATED: This function is now feature-complete
            void parseSampling(Query* query, const std::string& sample_str) {
                static const std::regex sample_regex(
                    R"(\s*(?:(RESERVOIR)\s+(\d+)|(SYSTEMATIC)\s+(\d+)|(STRATIFIED)\s+BY\s+([\w]+)\s+(\d+(?:\.\d+)?)%|(\d+(?:\.\d+)?)%))", 
                    std::regex::icase
                );
                
                std::smatch matches;
                if (std::regex_search(sample_str, matches, sample_regex)) {
                    if (matches[1].matched) { // RESERVOIR
                        query->sampling.method = SamplingMethod::RESERVOIR;
                        query->sampling.size = std::stoull(matches[2].str());
                    } else if (matches[3].matched) { // SYSTEMATIC
                        query->sampling.method = SamplingMethod::SYSTEMATIC;
                        query->sampling.size = std::stoull(matches[4].str());
                    } else if (matches[5].matched) { // STRATIFIED
                        query->sampling.method = SamplingMethod::STRATIFIED;
                        query->sampling.stratification_column = matches[6].str();
                        query->sampling.rate = std::stod(matches[7].str()) / 100.0;
                    } else if (matches[8].matched) { // RANDOM (percentage)
                        query->sampling.method = SamplingMethod::RANDOM;
                        query->sampling.rate = std::stod(matches[8].str()) / 100.0;
                    }
                } else {
                    throw ParseError("Invalid SAMPLE clause format");
                }
            }
        };

    } // namespace query
} // namespace aqe