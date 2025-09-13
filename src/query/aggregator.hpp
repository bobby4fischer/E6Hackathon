#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <limits>
#include "parser.hpp"

namespace aqe {
namespace query {

// Base class for aggregators
class Aggregator {
public:
    virtual ~Aggregator() = default;
    virtual void addValue(double value) = 0;
    virtual double getResult() const = 0;
};

// COUNT aggregator
class CountAggregator : public Aggregator {
private:
    size_t count = 0;

public:
    void addValue(double) override {
        ++count;
    }

    double getResult() const override {
        return static_cast<double>(count);
    }
};

// SUM aggregator
class SumAggregator : public Aggregator {
private:
    double sum = 0.0;

public:
    void addValue(double value) override {
        sum += value;
    }

    double getResult() const override {
        return sum;
    }
};

// AVG aggregator
class AvgAggregator : public Aggregator {
private:
    double sum = 0.0;
    size_t count = 0;

public:
    void addValue(double value) override {
        sum += value;
        ++count;
    }

    double getResult() const override {
        return count > 0 ? sum / count : 0.0;
    }
};

// MIN aggregator
class MinAggregator : public Aggregator {
private:
    double min = std::numeric_limits<double>::max();
    bool has_value = false;

public:
    void addValue(double value) override {
        min = has_value ? std::min(min, value) : value;
        has_value = true;
    }

    double getResult() const override {
        return has_value ? min : 0.0;
    }
};

// MAX aggregator
class MaxAggregator : public Aggregator {
private:
    double max = std::numeric_limits<double>::lowest();
    bool has_value = false;

public:
    void addValue(double value) override {
        max = has_value ? std::max(max, value) : value;
        has_value = true;
    }

    double getResult() const override {
        return has_value ? max : 0.0;
    }
};

// Class to hold aggregation results
class AggregateResult {
private:
    std::unordered_map<std::string, std::unique_ptr<Aggregator>> aggregators;
    std::vector<std::string> group_by_values;

public:
    void addAggregator(const std::string& column, AggregationType type) {
        switch (type) {
            case AggregationType::COUNT:
                aggregators[column] = std::make_unique<CountAggregator>();
                break;
            case AggregationType::SUM:
                aggregators[column] = std::make_unique<SumAggregator>();
                break;
            case AggregationType::AVG:
                aggregators[column] = std::make_unique<AvgAggregator>();
                break;
            case AggregationType::MIN:
                aggregators[column] = std::make_unique<MinAggregator>();
                break;
            case AggregationType::MAX:
                aggregators[column] = std::make_unique<MaxAggregator>();
                break;
            default:
                break;
        }
    }

    void addValue(const std::string& column, double value) {
        if (auto it = aggregators.find(column); it != aggregators.end()) {
            it->second->addValue(value);
        }
    }

    double getResult(const std::string& column) const {
        if (auto it = aggregators.find(column); it != aggregators.end()) {
            return it->second->getResult();
        }
        return 0.0;
    }

    void setGroupByValues(const std::vector<std::string>& values) {
        group_by_values = values;
    }

    const std::vector<std::string>& getGroupByValues() const {
        return group_by_values;
    }
};

} // namespace query
} // namespace aqe