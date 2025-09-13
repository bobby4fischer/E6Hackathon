#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

namespace aqe {
namespace utils {

// Trims whitespace from both ends of a string
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

// Converts a string to uppercase
static std::string toUpper(const std::string& str) {
    std::string upper = str;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    return upper;
}

// Splits a CSV line into a vector of strings
static std::vector<std::string> splitCSV(const std::string& line) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ',')) {
        result.push_back(trim(item));
    }
    return result;
}

} // namespace utils
} // namespace aqe