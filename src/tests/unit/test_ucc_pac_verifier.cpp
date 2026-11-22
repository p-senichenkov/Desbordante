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
#include "core/algorithms/pac/pac_verifier/ucc_pac_verifier/ucc_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/util/pac_verifier_utils.h"
#include "core/algorithms/pac/pac_verifier/util/validation_mode.h"
#include "core/config/custom_metric/custom_vector_metric/type.h"
#include "core/config/exceptions.h"
#include "core/config/indices/type.h"
#include "core/config/names.h"
#include "core/model/types/builtin.h"
#include "core/util/custom_metric/custom_vector_metric.h"
#include "tests/common/all_csv_configs.h"
#include "tests/common/csv_config_util.h"

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

void RunUCCPACVerifierTest(algos::StdParamsMap params, double exp_epsilon, double exp_delta,
                           algos::StdParamsMap const& extra_params = {}) {
    params.insert(extra_params.begin(), extra_params.end());
    auto algo = algos::CreateAndLoadAlgorithm<algos::pac_verifier::UCCPACVerifier>(params);
    algo->Execute();
    auto pac = algo->GetPAC();
    EXPECT_NEAR(pac.GetEpsilon(), exp_epsilon, kThreshold);
    EXPECT_NEAR(pac.GetDelta(), exp_delta, kThreshold);
}

struct UCCPACVerifyingParams {
    algos::StdParamsMap params;
    double exp_epsilon;
    double exp_delta;

    UCCPACVerifyingParams(CSVConfig const& csv_config, config::IndicesType&& column_indices,
                          double expected_epsilon, double expected_delta,
                          std::optional<double> max_delta = std::nullopt,
                          std::optional<double> min_epsilon = std::nullopt,
                          std::optional<double> max_epsilon = std::nullopt,
                          config::CustomVectorMetricOptionType&& metric = nullptr)
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
                          algos::pac_verifier::EpsilonDelta const& eps_delta,
                          std::optional<double> max_delta = std::nullopt,
                          std::optional<double> min_epsilon = std::nullopt,
                          std::optional<double> max_epsilon = std::nullopt,
                          std::shared_ptr<util::ICustomVectorMetric>&& metric = nullptr)
        : UCCPACVerifyingParams(csv_config, std::move(column_indices), eps_delta.epsilon,
                                eps_delta.delta, std::move(max_delta), std::move(min_epsilon),
                                std::move(max_epsilon), std::move(metric)) {}
};

class TestUCCPACVerifier : public testing::TestWithParam<UCCPACVerifyingParams> {};

TEST_P(TestUCCPACVerifier, DefaultTest) {
    auto const& p = GetParam();
    RunUCCPACVerifierTest(p.params, p.exp_epsilon, p.exp_delta);
}

// Same dataset and indices are used in many tests
// https://github.com/p-senichenkov/Domain-PAC-validation-comparison/blob/main/UCC-PAC/metirc-coords-annotated.pdf
// Note that there are barely visible steps below visible knees, and algorithm "slides down" to
// these steps
static std::vector<algos::pac_verifier::EpsilonDelta> const kMetricCoordsKnees{
        {0, 0}, {34.995, 0.514}, {77.986, 0.586}, {108, 0.6}, {120, 0.74}, {197.191, 0.846}};

INSTANTIATE_TEST_SUITE_P(
        Refinement, TestUCCPACVerifier,
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

UCCPACVerifyingParams MetricCoordsBoundsParams(algos::pac_verifier::EpsilonDelta const& eps_delta,
                                               std::optional<double> max_delta = std::nullopt,
                                               std::optional<double> min_eps = std::nullopt,
                                               std::optional<double> max_eps = std::nullopt) {
    return {kMetricCoords,        {2, 3},
            eps_delta.epsilon,    eps_delta.delta,
            std::move(max_delta), std::move(min_eps),
            std::move(max_eps)};
}

INSTANTIATE_TEST_SUITE_P(
        ParametrizedRefinement, TestUCCPACVerifier,
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

TEST(UCCPACVerifierTests, DefaultMetricFails) {
    // Check that an attempt to use default metric on non-metrizable column results in clear error
    algos::StdParamsMap params{
            {kCsvConfig, kMixedWithNulls},
            {kColumnIndices, config::IndicesType{0}},
    };
    EXPECT_THROW(algos::CreateAndLoadAlgorithm<algos::pac_verifier::UCCPACVerifier>(params),
                 config::ConfigurationError);
}

struct UCCPACVerifierModesParams {
    algos::StdParamsMap params;
    double epsilon;
    double delta;

    UCCPACVerifierModesParams(CSVConfig const& csv_config, config::IndicesType&& col_indices,
                              double epsilon, double delta)
        : params({
                  {kCsvConfig, csv_config},
                  {kColumnIndices, std::move(col_indices)},
          }),
          epsilon(epsilon),
          delta(delta) {}

    UCCPACVerifierModesParams(CSVConfig const& csv_config, config::IndicesType&& col_indices,
                              algos::pac_verifier::EpsilonDelta epsilon_delta)
        : UCCPACVerifierModesParams(csv_config, std::move(col_indices), epsilon_delta.epsilon,
                                    epsilon_delta.delta) {}
};

class TestUCCPACVerifierModes : public ::testing::TestWithParam<UCCPACVerifierModesParams> {};

TEST_P(TestUCCPACVerifierModes, ValidationModes) {
    using enum algos::pac_verifier::PACValidationMode;
    constexpr static auto kInfinity = std::numeric_limits<double>::infinity();

    auto const& p = GetParam();

    // Explicit find-epsilon
    RunUCCPACVerifierTest(p.params, p.epsilon, p.delta,
                          {{kPacValidationMode, kFindEpsilon}, {kDelta, p.delta}});
    // Implicit find-epsilon
    RunUCCPACVerifierTest(
            p.params, p.epsilon, p.delta,
            {{kMinEpsilon, kInfinity}, {kMaxEpsilon, kInfinity}, {kMaxDelta, p.delta}});
    // Explicit find-delta
    RunUCCPACVerifierTest(p.params, p.epsilon, p.delta,
                          {{kPacValidationMode, kFindDelta}, {kEpsilon, p.epsilon}});
    // Implicit find-delta
    RunUCCPACVerifierTest(p.params, p.epsilon, p.delta,
                          {{kMinEpsilon, p.epsilon}, {kMaxEpsilon, p.epsilon}});
}

INSTANTIATE_TEST_SUITE_P(ValidationModes, TestUCCPACVerifierModes,
                         testing::ValuesIn({
                                 UCCPACVerifierModesParams(kMetricCoords, {2, 3},
                                                           kMetricCoordsKnees[2]),
                                 // TODO: moar!
                         }));

using IndexPairs = std::vector<std::pair<std::size_t, std::size_t>>;

struct UCCPACHighlightParams {
    algos::StdParamsMap params;
    double eps_1;
    double eps_2;
    IndexPairs expected_highlight;

    UCCPACHighlightParams(CSVConfig const& csv_config, config::IndicesType&& column_indices,
                          IndexPairs&& expected_highlight, double eps_1 = 0, double eps_2 = -1)
        : params({
                  {kCsvConfig, csv_config},
                  {kColumnIndices, column_indices},
          }),
          eps_1(eps_1),
          eps_2(eps_2),
          expected_highlight(std::move(expected_highlight)) {}
};

class TestUCCPACHighlight : public testing::TestWithParam<UCCPACHighlightParams> {};

TEST_P(TestUCCPACHighlight, DefaultTest) {
    auto const& p = GetParam();
    auto verifier = algos::CreateAndLoadAlgorithm<algos::pac_verifier::UCCPACVerifier>(p.params);
    verifier->Execute();

    auto highlight = verifier->GetHighlights(p.eps_1, p.eps_2);
    auto highlight_indices = highlight.RowIndices();
    EXPECT_THAT(highlight_indices, testing::UnorderedElementsAreArray(p.expected_highlight));
}

INSTANTIATE_TEST_SUITE_P(
        UCCPACHighlightTests, TestUCCPACHighlight,
        testing::ValuesIn({
                UCCPACHighlightParams(kMetricMovies, {2},
                                      {
                                              // NOTE: Highlights are (xx, xx]
                                              {2, 3},  {2, 4},  {2, 5},   {3, 2},   {3, 6}, {3, 8},
                                              {4, 2},  {4, 8},  {4, 10},  {5, 2},   {5, 9}, {6, 3},
                                              {6, 9},  {7, 10}, {8, 3},   {8, 4},   {9, 5}, {9, 6},
                                              {10, 4}, {10, 7}, {10, 11}, {11, 10},
                                      },
                                      5, 10),
        }));
// TODO: moar highlight tests!
}  // namespace tests
