#pragma once

#include <vector>
#include <random>
#include <cmath>
#include <array>
#include <algorithm>
#include <stdexcept>
 

namespace aqe {
namespace core {
 
// Count-Min Sketch for approximate frequency counting
class CountMinSketch {
private:
    static constexpr size_t DEFAULT_DEPTH = 5;
    static constexpr size_t DEFAULT_WIDTH = 2048;
    
    std::vector<std::vector<int64_t>> sketch;
    std::vector<uint32_t> hash_seeds;
    size_t width;
    size_t depth;

    uint32_t hash(const std::string& item, uint32_t seed) const {
        uint32_t hash = seed;
        for (char c : item) {
            hash = hash * 31 + c;
        }
        return hash % width;
    }

public:
    CountMinSketch(size_t w = DEFAULT_WIDTH, size_t d = DEFAULT_DEPTH)
        : width(w), depth(d) {
        sketch.resize(depth, std::vector<int64_t>(width, 0));
        hash_seeds.resize(depth);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dist;
        
        for (size_t i = 0; i < depth; ++i) {
            hash_seeds[i] = dist(gen);
        }
    }

    void add(const std::string& item, int64_t count = 1) {
        for (size_t i = 0; i < depth; ++i) {
            size_t h = hash(item, hash_seeds[i]);
            sketch[i][h] += count;
        }
    }

    int64_t estimate(const std::string& item) const {
        int64_t min_count = INT64_MAX;
        
        for (size_t i = 0; i < depth; ++i) {
            size_t h = hash(item, hash_seeds[i]);
            min_count = std::min(min_count, sketch[i][h]);
        }
        
        return min_count;
    }

    void clear() {
        for (auto& row : sketch) {
            std::fill(row.begin(), row.end(), 0);
        }
    }
};

} // namespace core
} // namespace aqe