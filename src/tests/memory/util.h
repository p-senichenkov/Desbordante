#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "core/algorithms/algo_factory.h"

struct PeakMemoryUsage {
    unsigned long vspace_kb = 0;
    unsigned long rss_kb = 0;
};

template <>
struct std::formatter<PeakMemoryUsage, char> {
    template <typename ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("Invalid format args for PeakMemoryUsage");
        }
        return it;
    }

    template <typename FmtContext>
    FmtContext::iterator format(PeakMemoryUsage mem_usage, FmtContext& ctx) const {
        auto str = std::format("{{\"vspace_kb\": {}, \"rss_kb\": {}}}", mem_usage.vspace_kb,
                               mem_usage.rss_kb);
        return std::ranges::copy(std::move(str), ctx.out()).out;
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

template <typename Algo>
void MeasuredRun(algos::StdParamsMap const& params) {
    auto before = FetchMemoryUsage();
    auto algo = algos::CreateAndLoadAlgorithm<Algo>(params);
    auto after_load_data = FetchMemoryUsage();
    algo->Execute();
    auto after_execute = FetchMemoryUsage();

    std::cout << std::format("{{\"before\": {}, \"after_load\": {}, \"after_execute\": {}}}",
                             before, after_load_data, after_execute);
}
