#pragma once

#include <string>

namespace aqe {
namespace utils {

struct Config {
    std::string default_data_path = "data/sample_data.csv";
    double default_confidence_level = 0.95;
};

} // namespace utils
} // namespace aqe