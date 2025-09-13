#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <random>
#include <sstream>
#include "parser.hpp"
#include "aggregator.hpp"
#include "../core/sampling.hpp"
#include "../core/sketching.hpp"

namespace aqe {
namespace query {

// Structure to represent a data row
struct DataRow {
    std::unordered_map<std::string, std::string> values;
};

// Class to represent query results
class QueryResult {
    private:
        std::vector<std::vector<std::string>> rows;
        std::vector<std::string> column_names;
        bool is_approximate; 

    public:
        QueryResult() : is_approximate(false){}

        void addRow(const std::vector<std::string> &row) {
            rows.push_back(row);
        }

        void setColumnNames(const std::vector<std::string>& names) {
            column_names = names;
        }

        void setApproximate(bool approx) {
            is_approximate = approx; 
        }

        const std::vector<std::vector<std::string>>& getRows() const {
            return rows;
        }

        const std::vector<std::string>& getColumnNames() const {
            return column_names;
        }

        bool isApproximate() const { return is_approximate; } 
};

    // Query Executor class
    class QueryExecutor {
        private:
            std::unique_ptr<core::SamplingStrategy<DataRow>> sampler;
            std::unordered_map<std::string, std::unique_ptr<AggregateResult>> group_results;

        public:
            QueryExecutor() {}

            std::unique_ptr<QueryResult> execute(const Query& query, const std::vector<DataRow>& data) {
                group_results.clear(); // Ensure results are fresh for each query
                sampler.reset();

                auto result = std::make_unique<QueryResult>();
                setupSampling(query.sampling.method != SamplingMethod::NONE ? &query.sampling : nullptr);
                
                std::vector<DataRow> processed_data;
                double scaling_factor = 1.0;

                if (sampler) {
                    for (const auto& row : data) {
                        sampler->add(row);
                    }
                    processed_data = sampler->getSample();
                    result->setApproximate(true);
                    if (sampler->getSamplingRate() > 0) {
                        scaling_factor = 1.0 / sampler->getSamplingRate();
                    }
                } else {
                    processed_data = data;
                    result->setApproximate(false);
                }

                for (const auto& row : processed_data) {
                    processRow(query, row);
                }
                
                // Prepare final column headers
                std::vector<std::string> result_column_names;
                for (const auto& col : query.columns) {
                    result_column_names.push_back(col.alias.empty() ? col.name : col.alias);
                }
                result->setColumnNames(result_column_names);

                // Build the final result rows
                for (const auto& group_entry : group_results) {
                    const auto& agg_result = group_entry.second;
                    std::vector<std::string> result_row;
                    
                    std::unordered_map<std::string, std::string> group_by_map;
                    const auto& group_values = agg_result->getGroupByValues();
                    for(size_t i = 0; i < query.group_by_columns.size(); ++i) {
                        group_by_map[query.group_by_columns[i]] = group_values[i];
                    }

                    for (const auto& col : query.columns) {
                        if (col.aggregation == AggregationType::NONE) {
                            result_row.push_back(group_by_map[col.name]);
                        } else {
                            double final_value = agg_result->getResult(col.name);
                            // UPDATED: Scale COUNT and SUM results if we used a sample
                            if (sampler && (col.aggregation == AggregationType::COUNT || col.aggregation == AggregationType::SUM)) {
                                final_value *= scaling_factor;
                            }
                            result_row.push_back(std::to_string(final_value));
                        }
                    }
                    result->addRow(result_row);
                }

                return result;
            }
        

        private:
            void setupSampling(const Sampling* sampling) {
                if (!sampling || sampling->method == SamplingMethod::NONE) {
                    sampler.reset();
                    return;
                }

                switch (sampling->method) {
                    case SamplingMethod::RANDOM:
                        sampler = std::make_unique<core::SimpleRandomSampling<DataRow>>(sampling->rate);
                        break;
                    case SamplingMethod::SYSTEMATIC:
                        sampler = std::make_unique<core::SystematicSampling<DataRow>>(
                            static_cast<size_t>(1.0 / sampling->rate));
                        break;
                    case SamplingMethod::RESERVOIR:
                        sampler = std::make_unique<core::ReservoirSample<DataRow>>(sampling->size);
                        break;
                    case SamplingMethod::STRATIFIED: {
                        auto key_extractor = [sampling](const DataRow& row) {
                            return row.values.at(sampling->stratification_column);
                        };
                        // NEW VERSION in executor.hpp
                        sampler = std::make_unique<core::StratifiedSampling<DataRow, decltype(key_extractor)>>(
                            sampling->rate, key_extractor); // Pass the extractor as the second argument
                                        break;
                                    }
                    default:
                        sampler.reset();
                        break;
                }
            } 

            void processRow(const Query& query, const DataRow& row) {
                // Create group key for GROUP BY
                std::string group_key = "default";
                std::vector<std::string> group_values;
                if (!query.group_by_columns.empty()) {
                    std::stringstream ss;
                    for (const auto& group_col : query.group_by_columns) {
                        auto it = row.values.find(group_col);
                        
                        // --- UPDATED LOGIC START ---
                        if (it != row.values.end()) {
                            ss << it->second << "|";
                            group_values.push_back(it->second);
                        } else {
                            // If a group-by column is missing from the row,
                            // add a placeholder to prevent a crash.
                            ss << "NULL|";
                            group_values.push_back("NULL"); 
                        }
                        // --- UPDATED LOGIC END ---
                    }
                    group_key = ss.str();
                }
        
                // Get or create aggregate result for this group
                if (group_results.find(group_key) == group_results.end()) {
                    auto agg_result = std::make_unique<AggregateResult>();
                    for (const auto& col : query.columns) {
                        if (col.aggregation != AggregationType::NONE) {
                            agg_result->addAggregator(col.name, col.aggregation);
                        }
                    }
                    agg_result->setGroupByValues(group_values);
                    group_results[group_key] = std::move(agg_result);
                }

                // Process aggregations
                for (const auto& col : query.columns) {
                    if (col.aggregation != AggregationType::NONE) {
                        if (col.aggregation == AggregationType::COUNT) {
                            group_results[group_key]->addValue(col.name, 1.0);
                        } else {
                            auto value_it = row.values.find(col.name);
                            if (value_it != row.values.end()) {
                                try {
                                    double value = std::stod(value_it->second);
                                    group_results[group_key]->addValue(col.name, value);
                                } catch (const std::exception&) {
                                    // Handle non-numeric values gracefully
                                }
                            }
                        }
                    }
                }
            }
        };

    } // namespace query
} // namespace aqe