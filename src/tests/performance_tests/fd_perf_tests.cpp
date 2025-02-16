#include <cctype>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "algorithms/algo_factory.h"
#include "algorithms/fd/hyfd/hyfd.h"
#include "algorithms/fd/pyro/pyro.h"
#include "algorithms/fd/tane/tane.h"
#include "all_csv_configs.h"
#include "config/error/type.h"
#include "config/max_lhs/type.h"
#include "config/names.h"
#include "config/thread_number/type.h"

namespace tests {

using namespace algos;
using namespace config::names;

// HeavyDatasets is a common flag that is used everywhere in gtest_filters
// (i. e. these tests are disabled by default)
// PerformanceTests is a new flag that will be used for the same purpose for performance testing.

// Dataset name is needed to get right statistics if dataset is changed.

// If you are adding tests here, please give them meaningful names.
// Only test names will be visible in performance-testing output.
// Test parameters are also visible, but you *must* give them normal names
// (40-byte object <40-00 00-af...> is *not* a normal name)

TEST(HeavyDatasetsPerformanceTests, HyFD_Iowa600k) {
    StdParamsMap param_map{
            {kCsvConfig, kIowa600k},
            {kThreads, static_cast<config::ThreadNumType>(1)},
            {kMaximumLhs, static_cast<config::MaxLhsType>(2)},
    };
    auto algo = CreateAndLoadAlgorithm<hyfd::HyFD>(param_map);

    algo->Execute();
    // Performace tests always succeed
    SUCCEED();
}

TEST(HeavyDatasetsPerformanceTests, Pyro_Iowa600k) {
    StdParamsMap param_map{
            {kCsvConfig, kIowa600k},
            {kError, static_cast<config::ErrorType>(0.0)},
            {kSeed, decltype(algos::pyro::Parameters::seed){0}},
            {kMaximumLhs, static_cast<config::MaxLhsType>(2)},
            {kThreads, static_cast<config::ThreadNumType>(1)},
    };
    auto algo = CreateAndLoadAlgorithm<Pyro>(param_map);

    algo->Execute();
    SUCCEED();
}

struct AFDTaneMiningParams {
    CSVConfig csv_config_;
    AfdErrorMeasure error_measure_;

    AFDTaneMiningParams(CSVConfig const& csv_config, AfdErrorMeasure error_measure)
        : csv_config_(csv_config), error_measure_(error_measure) {}
};

class AFDTaneTest : public ::testing::TestWithParam<AFDTaneMiningParams> {};

// Here, AFDTane will be displayed
TEST_P(AFDTaneTest, AFDTane) {
    auto const& p = GetParam();
    StdParamsMap param_map{
            {kCsvConfig, p.csv_config_},
            {kError, static_cast<config::ErrorType>(0.5)},
            {kAfdErrorMeasure, p.error_measure_},
    };
    auto algo = CreateAndLoadAlgorithm<Tane>(param_map);

    algo->Execute();
    SUCCEED();
}

INSTANTIATE_TEST_SUITE_P(
        HeavyDatasetsPerformanceTests, AFDTaneTest,
        ::testing::Values(AFDTaneMiningParams(kMushroom2k, +algos::AfdErrorMeasure::g1),
                          AFDTaneMiningParams(kMushroom2k, +algos::AfdErrorMeasure::pdep),
                          AFDTaneMiningParams(kMushroom2k, +algos::AfdErrorMeasure::tau),
                          AFDTaneMiningParams(kMushroom2k, +algos::AfdErrorMeasure::mu_plus),
                          AFDTaneMiningParams(kMushroom2k, +algos::AfdErrorMeasure::rho)),
        [](::testing::TestParamInfo<AFDTaneTest::ParamType> const& info) -> std::string {
            auto param = info.param;

            // googletest supports only alphanumerical characters in value name,
            // so there shouldn't be any separators.
            // Not-alnum characters are simply removed.
            // Measure and dataset names start with uppercase letter to separate them.
            std::string measure = param.error_measure_._to_string();
            std::erase_if(measure, [](char ch) { return std::isalnum(ch) == 0; });
            measure.front() = std::toupper(measure.front());

            std::string dataset = param.csv_config_.path.stem();
            std::erase_if(dataset, [](char ch) { return std::isalnum(ch) == 0; });
            dataset.front() = std::toupper(dataset.front());

            return measure + dataset;
        });

}  // namespace tests
