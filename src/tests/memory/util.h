#pragma once

#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#include "core/algorithms/algo_factory.h"

namespace memory {
namespace detail {
struct PeakMemoryUsage {
    unsigned long vspace_kb = 0;
    unsigned long rss_kb = 0;
};

inline std::ostream& operator<<(std::ostream& os, PeakMemoryUsage const& mem_usage) {
    os << "{\"vspace_kb\": " << mem_usage.vspace_kb << ", \"rss_kb\": " << mem_usage.rss_kb << "}";
    return os;
}

inline PeakMemoryUsage FetchMemoryUsage() {
    std::ifstream ifs{"/proc/self/status"};

    PeakMemoryUsage result;

    std::string line;
    std::string key;
    unsigned long value;
    while (std::getline(ifs, line)) {
        std::istringstream iss{line};
        iss >> key;
        if (key == "VmPeak:") {
            iss >> value;
            result.vspace_kb = value;
        } else if (key == "VmHWM:") {
            iss >> value;
            result.rss_kb = value;
        }
    }
    return result;
}
}  // namespace detail

template <typename Algo>
void MeasuredRun(algos::StdParamsMap const& params) {
    auto before = detail::FetchMemoryUsage();
    auto algo = algos::CreateAndLoadAlgorithm<Algo>(params);
    auto after_load_data = detail::FetchMemoryUsage();
    algo->Execute();
    auto after_execute = detail::FetchMemoryUsage();

    std::cout << "{\"before\": " << before << ", \"after_load\": " << after_load_data
              << ", \"after_execute\": " << after_execute << "}\n";
}
}  // namespace memory
