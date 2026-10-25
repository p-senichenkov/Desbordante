#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/json/src.hpp>

#include "core/algorithms/algo_factory.h"

namespace memory {
namespace detail {
struct PeakMemoryUsage {
    unsigned long vspace_kb = 0;
    unsigned long rss_kb = 0;

    boost::json::object ToJson() const {
        return boost::json::object{{"vspace_kb", vspace_kb}, {"rss_kb", rss_kb}};
    }
};

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
    auto start = std::chrono::steady_clock::now();
    auto algo = algos::CreateAndLoadAlgorithm<Algo>(params);
    auto load_data_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                     std::chrono::steady_clock::now() - start)
                                     .count();

    auto after_load_data = detail::FetchMemoryUsage();
    start = std::chrono::steady_clock::now();
    algo->Execute();
    auto execute_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::steady_clock::now() - start)
                                   .count();
    auto after_execute = detail::FetchMemoryUsage();

    boost::json::object top_level_obj{
            {"memory", boost::json::object{{"before", before.ToJson()},
                                           {"after_load", after_load_data.ToJson()},
                                           {"after_execute", after_execute.ToJson()}}},
            {"time",
             boost::json::object{{"load_data", load_data_time_ms}, {"execute", execute_time_ms}}},
    };
    std::cout << top_level_obj << '\n';
}
}  // namespace memory
