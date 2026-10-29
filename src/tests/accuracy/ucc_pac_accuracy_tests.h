#pragma once

#include <cstddef>
#include <limits>
#include <optional>
#include <utility>

#include "core/algorithms/algo_factory.h"
#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/histogram_ucc_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/ucc_pac_verifier/ucc_pac_verifier.h"
#include "core/config/names.h"
#include "tests/accuracy/epsilon_delta_error.h"
#include "tests/accuracy/runner.h"
#include "tests/common/all_csv_configs.h"

namespace accuracy {
namespace ucc_pac {

using BaselineAlgo = algos::pac_verifier::UCCPACVerifier;
using TestedAlgo = algos::pac_verifier::HistogramUCCPACVerifier;
constexpr static auto kNumericOption = config::names::kHistogramMultiplier;
using NumericOptionType = unsigned;

template <typename T>
boost::any OptToAny(std::optional<T>&& opt) {
    return opt ? *opt : boost::any{};
}

template <typename Algo>
std::pair<double, double> Execute(algos::StdParamsMap const& params) {
    auto algo = algos::CreateAndLoadAlgorithm<Algo>(params);
    algo->Execute();
    auto const& pac = algo->GetPAC();
    return {pac.GetEpsilon(), pac.GetDelta()};
}

AccuracyTest UCCPACAccuracyTest(CSVConfig const& csv_config, config::IndicesType&& column_indices,
                                std::optional<double> max_delta = std::nullopt,
                                std::optional<double> min_epsilon = std::nullopt,
                                std::optional<double> max_epsilon = std::nullopt,
                                std::shared_ptr<util::ICustomVectorMetric>&& metric = nullptr) {
    using namespace config::names;

    algos::StdParamsMap params{
            {kCsvConfig, csv_config},
            {kColumnIndices, std::move(column_indices)},
            {kMaxDelta, OptToAny(std::move(max_delta))},
            {kMinEpsilon, OptToAny(std::move(min_epsilon))},
            {kMaxEpsilon, OptToAny(std::move(max_epsilon))},
            {kMetric, std::move(metric)},
    };

    return [params = std::move(params)](std::size_t n) mutable {
        auto [baseline_eps, baseline_delta] = Execute<BaselineAlgo>(params);

        params[kNumericOption] = static_cast<NumericOptionType>(n);
        auto [actual_eps, actual_delta] = Execute<TestedAlgo>(params);

        return EpsilonDeltaError{.epsilon = {.baseline = baseline_eps, .actual = actual_eps},
                                 .delta = {.baseline = baseline_delta, .actual = actual_delta}};
    };
}

using namespace tests;

// Test from Refinement unit-test suite
void RegisterUCCPACRefinementAccuracyTests(AccuracyTestRunner& runner) {
    auto urchins_metric = std::make_shared<util::CustomVectorMetric>(
            [](auto const&, auto const& first, auto const& second) {
                assert(first.size() == 2 && second.size() == 2);

                // Values should be remapped properly, i. e. Col1 becomes [0]
                // and Col2 becomes [1]
                double col1_dist = std::abs(model::Type::GetValue<model::Int>(first[0]) -
                                            model::Type::GetValue<model::Int>(second[0]));

                std::string col2_first_str = model::Type::GetValue<model::String>(first[1]);
                assert(col2_first_str.size() == 1);
                std::string col2_second_str = model::Type::GetValue<model::String>(second[1]);
                assert(col2_second_str.size() == 1);
                // "Alphabet distance"
                double col2_dist = std::abs(col2_first_str.front() - col2_second_str.front());

                return col1_dist * col2_dist;
            });

    runner.Register("Refinement-0", UCCPACAccuracyTest(kMetricMovies, {2}));
    runner.Register("Refinement-1", UCCPACAccuracyTest(kMetricCoords, {2, 3}));
    runner.Register("Refinement-2", UCCPACAccuracyTest(kMarineUrchins, {1, 2}, 0.8, std::nullopt,
                                                       std::nullopt, std::move(urchins_metric)));
}

AccuracyTest MetricCoordsBounds(std::optional<double> max_delta,
                                std::optional<double> min_eps = std::nullopt,
                                std::optional<double> max_eps = std::nullopt) {
    return UCCPACAccuracyTest(kMetricCoords, {2, 3}, std::move(max_delta), std::move(min_eps),
                              std::move(max_eps));
}

// Tests from ParamethrizedRefinement unit-test suite
void RegisterUCCPACParamethrizedRefinementAccuracyTests(AccuracyTestRunner& runner) {
    runner.Register("ParamethrizedRefinement-min-max-eps", MetricCoordsBounds(1, 50, 180));
    runner.Register("ParamethrizedRefinement-max-eps", MetricCoordsBounds(1, std::nullopt, 180));
    runner.Register("ParamethrizedRefinement-min-eps", MetricCoordsBounds(1, 175));
    runner.Register("ParamethrizedRefinement-min-eps-2", MetricCoordsBounds(1, 125));
    runner.Register("ParamethrizedRefinement-max-delta", MetricCoordsBounds(0.57));
    runner.Register("ParamethrizedRefinement-min-eps-max-delta", MetricCoordsBounds(0.777, 50));
    runner.Register("ParamethrizedRefinement-max-eps-max-delta",
                    MetricCoordsBounds(0.7, std::nullopt, 110));
    runner.Register("ParamethrizedRefinement-all-bounds", MetricCoordsBounds(0.7, 50, 110));
}

// Tests from Validation unit-test suite
void RegisterUCCPACValidationAccuracyTests(AccuracyTestRunner& runner) {
    constexpr static double kMetricCoordsThirdKneeDelta = 0.586;
    constexpr static double kInfinity = std::numeric_limits<double>::infinity();

    runner.Register("Validation-find-eps",
                    MetricCoordsBounds(kMetricCoordsThirdKneeDelta + 0.02, kInfinity, kInfinity));
    runner.Register("Validation-find-delta", MetricCoordsBounds(std::nullopt, 80, 80));
}
}  // namespace ucc_pac

void RegisterUCCPACAccuracyTests(AccuracyTestRunner& runner) {
    ucc_pac::RegisterUCCPACRefinementAccuracyTests(runner);
    ucc_pac::RegisterUCCPACParamethrizedRefinementAccuracyTests(runner);
}
}  // namespace accuracy
