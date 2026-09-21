// Copy of UCC PAC verifier tests

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "core/algorithms/algo_factory.h"
#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/histogram_ucc_pac_verifier.h"
#include "core/config/exceptions.h"
#include "core/config/indices/type.h"
#include "core/config/names.h"
#include "core/model/types/builtin.h"
#include "core/util/custom_metric/custom_vector_metric.h"
#include "gtest/gtest.h"
#include "tests/common/all_csv_configs.h"

namespace {
template <typename T>
boost::any OptToAny(std::optional<T>&& opt) {
    return opt ? *opt : boost::any{};
}
}  // namespace

namespace tests {
using namespace config::names;
using namespace pac::model;

constexpr static auto kThreshold = 1e-3;

struct EpsilonDelta {
    double epsilon;
    double delta;
};

struct UCCPACVerifyingParams {
    algos::StdParamsMap params;
    double exp_epsilon;
    double exp_delta;

    UCCPACVerifyingParams(CSVConfig const& csv_config, config::IndicesType&& column_indices,
                          double expected_epsilon, double expected_delta,
                          std::optional<double> max_delta = std::nullopt,
                          std::optional<double> min_epsilon = std::nullopt,
                          std::optional<double> max_epsilon = std::nullopt,
                          std::shared_ptr<util::ICustomVectorMetric>&& metric = nullptr)
        : params({
                  {kCsvConfig, csv_config},
                  {kColumnIndices, std::move(column_indices)},
                  {kMaxDelta, OptToAny(std::move(max_delta))},
                  {kMinEpsilon, OptToAny(std::move(min_epsilon))},
                  {kMaxEpsilon, OptToAny(std::move(max_epsilon))},
                  {kMetric, std::move(metric)},
          }),
          exp_epsilon(expected_epsilon),
          exp_delta(expected_delta) {}

    UCCPACVerifyingParams(CSVConfig const& csv_config, config::IndicesType&& column_indices,
                          EpsilonDelta const& eps_delta,
                          std::optional<double> max_delta = std::nullopt,
                          std::optional<double> min_epsilon = std::nullopt,
                          std::optional<double> max_epsilon = std::nullopt,
                          std::shared_ptr<util::ICustomVectorMetric>&& metric = nullptr)
        : UCCPACVerifyingParams(csv_config, std::move(column_indices), eps_delta.epsilon,
                                eps_delta.delta, std::move(max_delta), std::move(min_epsilon),
                                std::move(max_epsilon), std::move(metric)) {}
};

class TestHistogramUCCPACVerifier : public testing::TestWithParam<UCCPACVerifyingParams> {};

TEST_P(TestHistogramUCCPACVerifier, DefaultTest) {
    auto const& p = GetParam();
    auto verifier =
            algos::CreateAndLoadAlgorithm<algos::pac_verifier::HistogramUCCPACVerifier>(p.params);
    verifier->Execute();

    auto const& pac = verifier->GetPAC();
    EXPECT_NEAR(pac.GetEpsilon(), p.exp_epsilon, kThreshold);
    EXPECT_NEAR(pac.GetDelta(), p.exp_delta, kThreshold);
}

auto const kAlphabetMetric = [](std::string const& a, std::string const& b) {
    return std::abs(a.front() - b.front());
};

// Same dataset and indices are used in many tests
// https://github.com/p-senichenkov/Domain-PAC-validation-comparison/blob/main/UCC-PAC/metirc-coords-annotated.pdf
// Note that there are barely visible steps below visible knees, and algorithm "slides down" to
// these steps
static std::vector<EpsilonDelta> const kMetricCoordsKnees{
        {0, 0}, {34.995, 0.514}, {77.986, 0.586}, {108, 0.6}, {120, 0.74}, {197.191, 0.846}};

INSTANTIATE_TEST_SUITE_P(
        Refinement, TestHistogramUCCPACVerifier,
        testing::ValuesIn({
                // Quite ordinary UCC PAC
                UCCPACVerifyingParams(kMetricMovies, {2}, 11, 0.402, 0.5),
                UCCPACVerifyingParams(kMetricCoords, {2, 3}, kMetricCoordsKnees[1], 0.7),
                // Custom metric (also check that passing a temporary metric is OK)
                UCCPACVerifyingParams(
                        kMarineUrchins, {1, 2}, 46, 0.768, 0.8, std::nullopt, std::nullopt,
                        std::make_shared<util::CustomVectorMetric>(
                                [](auto const&, auto const& first, auto const& second) {
                                    assert(first.size() == 2 && second.size() == 2);

                                    // Values should be remapped properly, i. e. Col1 becomes [0]
                                    // and Col2 becomes [1]
                                    double col1_dist =
                                            std::abs(model::Type::GetValue<model::Int>(first[0]) -
                                                     model::Type::GetValue<model::Int>(second[0]));

                                    std::string col2_first_str =
                                            model::Type::GetValue<model::String>(first[1]);
                                    assert(col2_first_str.size() == 1);
                                    std::string col2_second_str =
                                            model::Type::GetValue<model::String>(second[1]);
                                    assert(col2_second_str.size() == 1);
                                    // "Alphabet distance"
                                    double col2_dist = std::abs(col2_first_str.front() -
                                                                col2_second_str.front());

                                    return col1_dist * col2_dist;
                                })),
                // TODO(p-senchenkov): Something more?
        }));

UCCPACVerifyingParams MetricCoordsBoundsParams(EpsilonDelta const& eps_delta,
                                               std::optional<double> max_delta = std::nullopt,
                                               std::optional<double> min_eps = std::nullopt,
                                               std::optional<double> max_eps = std::nullopt) {
    return {kMetricCoords,        {2, 3},
            eps_delta.epsilon,    eps_delta.delta,
            std::move(max_delta), std::move(min_eps),
            std::move(max_eps)};
}

INSTANTIATE_TEST_SUITE_P(
        ParametrizedRefinement, TestHistogramUCCPACVerifier,
        testing::ValuesIn({
                // Both min epsilon and max epsilon
                MetricCoordsBoundsParams(kMetricCoordsKnees[2], 1, 50, 180),
                // Only max epsilon
                MetricCoordsBoundsParams(kMetricCoordsKnees[1], 1, std::nullopt, 180),
                // Only min epsilon
                MetricCoordsBoundsParams(kMetricCoordsKnees[5], 1, 175),
                // NOTE: algorithm is allowed to select passed min bound
                MetricCoordsBoundsParams({125, 0.775}, 1, 125),
                // Only max delta
                MetricCoordsBoundsParams(kMetricCoordsKnees[1], 0.57),
                // Min epsilon and max delta
                MetricCoordsBoundsParams(kMetricCoordsKnees[2], 0.777, 50),
                // Max epsilon and max delta
                MetricCoordsBoundsParams(kMetricCoordsKnees[1], 0.7, std::nullopt, 110),
                // All bounds
                MetricCoordsBoundsParams(kMetricCoordsKnees[2], 0.7, 50, 110),
        }));

INSTANTIATE_TEST_SUITE_P(
        Validation, TestHistogramUCCPACVerifier,
        testing::ValuesIn({
                // Find epsilon by delta
                MetricCoordsBoundsParams(kMetricCoordsKnees[2],
                                         // TODO(psenichenkov@): maybe add two separate functions,
                                         // and do not complicate things?
                                         kMetricCoordsKnees[2].delta + 0.02,
                                         std::numeric_limits<double>::infinity(),
                                         std::numeric_limits<double>::infinity()),
                // Find delta by epsilon
                MetricCoordsBoundsParams(kMetricCoordsKnees[2], std::nullopt, 80, 80),
        }));

TEST(UCCPACVerifierTests, DefaultMetricFails) {
    // Check that an attempt to use default metric on non-metrizable column results in clear error
    algos::StdParamsMap params{
            {kCsvConfig, kMixedWithNulls},
            {kColumnIndices, config::IndicesType{0}},
    };
    auto verifier =
            algos::CreateAndLoadAlgorithm<algos::pac_verifier::HistogramUCCPACVerifier>(params);
    EXPECT_THROW(verifier->Execute(), config::ConfigurationError);
}
}  // namespace tests
