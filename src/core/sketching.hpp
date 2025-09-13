#pragma once

#include <vector>
#include <cmath>
#include <random>
#include <bitset>
#include <array>
#include "data_structures.hpp"


namespace aqe {
namespace core {
#if defined(_MSC_VER)
#include <intrin.h> // For the MSVC intrinsic
#endif
// HyperLogLog implementation for cardinality estimation
class HyperLogLog {
private:
    static constexpr size_t NUM_BUCKETS = 1024;  // 2^10
    static constexpr size_t BUCKET_BITS = 10;
    std::vector<uint8_t> registers;
    std::hash<std::string> hasher;

    inline size_t getBucket(uint64_t hash) const {
        return hash >> (64 - BUCKET_BITS);
    }
    
    inline uint8_t getLeadingZeros(uint64_t hash) const {
                hash = hash << BUCKET_BITS; // Remove bucket bits
                if (hash == 0) {
                    return 64 - BUCKET_BITS;
                }
        #if defined(_MSC_VER)
                unsigned long index;
                _BitScanReverse64(&index, hash);
                return static_cast<uint8_t>(63 - index);
        #else
                return __builtin_clzll(hash);
        #endif
            }

public:
    HyperLogLog() : registers(NUM_BUCKETS, 0) {}

    void add(const std::string& item) {
        uint64_t hash = hasher(item);
        size_t bucket = getBucket(hash);
        uint8_t zeros = getLeadingZeros(hash);
        registers[bucket] = std::max(registers[bucket], zeros);
    }

    double estimate() const {
        double sum = 0.0;
        for (uint8_t val : registers) {
            sum += 1.0 / (1ull << val);
        }

        double alpha = 0.7213 / (1.0 + 1.079 / NUM_BUCKETS);
        double estimate = alpha * NUM_BUCKETS * NUM_BUCKETS / sum;
        
        // Apply corrections for small and large ranges
        if (estimate <= 2.5 * NUM_BUCKETS) {
            // Small range correction
            uint32_t zeros = std::count(registers.begin(), registers.end(), 0);
            if (zeros != 0) {
                estimate = NUM_BUCKETS * std::log(NUM_BUCKETS / (double)zeros);
            }
        } else if (estimate > (1ull << 32) / 30.0) {
            // Large range correction
            estimate = -pow(2, 32) * log(1.0 - estimate / pow(2, 32));
        }
        
        return estimate;
    }

    void clear() {
        std::fill(registers.begin(), registers.end(), 0);
    }
};

// Bloom Filter implementation for membership testing
class BloomFilter {
private:
    static constexpr size_t NUM_HASH_FUNCTIONS = 3;
    std::vector<bool> bits;
    size_t num_bits;
    std::array<std::hash<std::string>, NUM_HASH_FUNCTIONS> hashers;

    size_t getIndex(const std::string& item, size_t hash_function) const {
        return hashers[hash_function](item) % num_bits;
    }

public:
    BloomFilter(size_t size = 10000) : num_bits(size), bits(size, false) {}

    void add(const std::string& item) {
        for (size_t i = 0; i < NUM_HASH_FUNCTIONS; ++i) {
            bits[getIndex(item, i)] = true;
        }
    }

    bool mightContain(const std::string& item) const {
        for (size_t i = 0; i < NUM_HASH_FUNCTIONS; ++i) {
            if (!bits[getIndex(item, i)]) {
                return false;
            }
        }
        return true;
    }

    void clear() {
        std::fill(bits.begin(), bits.end(), false);
    }

    double getFalsePositiveRate() const {
        size_t set_bits = std::count(bits.begin(), bits.end(), true);
        double p = static_cast<double>(set_bits) / num_bits;
        return std::pow(p, NUM_HASH_FUNCTIONS);
    }
};

// Exponential Histogram for sliding window counting
class ExponentialHistogram {
private:
    struct Bucket {
        uint64_t count;
        uint64_t timestamp;
    };

    std::vector<Bucket> buckets;
    uint64_t window_size;
    double epsilon;

public:
    ExponentialHistogram(uint64_t window = 1000, double eps = 0.01)
        : window_size(window), epsilon(eps) {}

    void add(uint64_t timestamp, uint64_t count = 1) {
        // Remove expired buckets
        uint64_t cutoff = timestamp > window_size ? timestamp - window_size : 0;
        buckets.erase(
            std::remove_if(buckets.begin(), buckets.end(),
                [cutoff](const Bucket& b) { return b.timestamp < cutoff; }),
            buckets.end());

        // Add new bucket
        buckets.push_back({count, timestamp});

        // Merge buckets if needed
        mergeBuckets();
    }

    uint64_t estimate(uint64_t current_time) const {
        uint64_t cutoff = current_time > window_size ? current_time - window_size : 0;
        uint64_t sum = 0;

        for (const auto& bucket : buckets) {
            if (bucket.timestamp >= cutoff) {
                sum += bucket.count;
            }
        }

        return sum;
    }

private:
    void mergeBuckets() {
        size_t k = static_cast<size_t>(std::ceil(1.0 / epsilon));
        size_t max_buckets = k * (1 + std::floor(std::log2(window_size)));

        while (buckets.size() > max_buckets) {
            // Find oldest buckets with similar sizes
            auto it = std::adjacent_find(buckets.begin(), buckets.end(),
                [](const Bucket& a, const Bucket& b) {
                    return a.count == b.count;
                });

            if (it != buckets.end()) {
                // Merge adjacent buckets
                it->count *= 2;
                buckets.erase(it + 1);
            } else {
                // If no similar buckets found, merge oldest
                if (buckets.size() >= 2) {
                    buckets[0].count += buckets[1].count;
                    buckets.erase(buckets.begin() + 1);
                }
            }
        }
    }
};

} // namespace core
} // namespace aqe