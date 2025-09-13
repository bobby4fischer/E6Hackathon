#pragma once

#include <vector>
#include <random>
#include <cmath>
#include <memory> 
#include <unordered_map>
#include <stdexcept>
#include <string>

namespace aqe {
namespace core {

// Abstract base class for sampling strategies
template<typename T>
class SamplingStrategy {
public:
    virtual ~SamplingStrategy() = default;
    virtual void add(const T& item) = 0;
    virtual std::vector<T> getSample() const = 0; // UPDATED: Return by value for safety
    virtual void clear() = 0;
    virtual double getSamplingRate() const = 0;
};

// Simple Random Sampling implementation
template<typename T>
class SimpleRandomSampling : public SamplingStrategy<T> {
private:
    double sampling_rate;
    std::vector<T> sample;
    std::mt19937 gen;
    std::uniform_real_distribution<> dist;

public:
    SimpleRandomSampling(double rate = 0.1) 
        : sampling_rate(rate), gen(std::random_device{}()), dist(0.0, 1.0) {
        if (rate <= 0.0 || rate > 1.0) {
            throw std::invalid_argument("Sampling rate must be between 0 and 1");
        }
    }

    void add(const T& item) override {
        if (dist(gen) < sampling_rate) {
            sample.push_back(item);
        }
    }

    std::vector<T> getSample() const override {
        return sample;
    }

    void clear() override {
        sample.clear();
    }

    double getSamplingRate() const override {
        return sampling_rate;
    }
};

// Systematic Sampling implementation
template<typename T>
class SystematicSampling : public SamplingStrategy<T> {
private:
    size_t step_size;
    size_t current_count;
    std::vector<T> sample;

public:
    SystematicSampling(size_t step = 10) 
        : step_size(step), current_count(0) {
        if (step < 1) {
            throw std::invalid_argument("Step size must be at least 1");
        }
    }

    void add(const T& item) override {
        current_count++;
        if (current_count > 0 && current_count % step_size == 0) {
            sample.push_back(item);
        }
    }

    std::vector<T> getSample() const override {
        return sample;
    }

    void clear() override {
        sample.clear();
        current_count = 0;
    }

    double getSamplingRate() const override {
        return 1.0 / static_cast<double>(step_size);
    }
};

// Reservoir Sampling implementation
template<typename T>
class ReservoirSample : public SamplingStrategy<T> {
private:
    std::vector<T> reservoir;
    size_t max_size;
    size_t total_seen;
    std::mt19937 gen;

public:
    ReservoirSample(size_t size) : max_size(size), total_seen(0), gen(std::random_device{}()) {
        reservoir.reserve(size);
    }

    void add(const T& item) override {
        total_seen++;
        if (reservoir.size() < max_size) {
            reservoir.push_back(item);
        } else {
            std::uniform_int_distribution<size_t> dist(0, total_seen - 1);
            size_t j = dist(gen);
            if (j < max_size) {
                reservoir[j] = item;
            }
        }
    }

    std::vector<T> getSample() const override {
        return reservoir;
    }

    void clear() override {
        reservoir.clear();
        total_seen = 0;
    }

    double getSamplingRate() const override {
        if (total_seen == 0) {
            return 0.0;
        }
        return static_cast<double>(reservoir.size()) / total_seen;
    }
};

// Stratified Sampling implementation
template<typename T, typename KeyExtractor>
class StratifiedSampling : public SamplingStrategy<T> {
private:
    // UPDATED: Added missing member variables
    double sampling_rate;
    std::unordered_map<std::string, std::unique_ptr<ReservoirSample<T>>> strata;
    KeyExtractor key_extractor;

public:
    StratifiedSampling(double rate, KeyExtractor extractor)
        : sampling_rate(rate), key_extractor(extractor) {
        if (rate <= 0.0 || rate > 1.0) {
            throw std::invalid_argument("Sampling rate must be between 0 and 1");
        }
    } 

    void add(const T& item) override {
        std::string stratum_key = key_extractor(item);
        if (strata.find(stratum_key) == strata.end()) {
            strata[stratum_key] = std::make_unique<ReservoirSample<T>>(100); // Default reservoir size per stratum
        }
        strata[stratum_key]->add(item);
    }

    std::vector<T> getSample() const override {
        std::vector<T> result;
        for (const auto& [key, reservoir] : strata) {
            const auto& stratum_sample = reservoir->getSample();
            result.insert(result.end(), stratum_sample.begin(), stratum_sample.end());
        }
        return result;
    }

    void clear() override {
        strata.clear(); 
    }

    double getSamplingRate() const override {
        return sampling_rate;
    }
};

} // namespace core
} // namespace aqe