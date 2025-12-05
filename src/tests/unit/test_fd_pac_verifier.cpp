#include <algorithm>
#include <cstddef>
#include <functional>
#include <unordered_set>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "core/algorithms/algo_factory.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/column_metric.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_cluster.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_verifier.h"
#include "core/config/indices/type.h"
#include "core/config/names.h"
#include "core/csv_parser/csv_parser.h"
#include "core/tests/common/all_csv_configs.h"
#include "core/util/csv_config_util.h"

namespace tests {
using namespace config::names;
using namespace pac::model;

constexpr static auto kThreshold = 1e-3;

struct FDPACVerifyingParams {
    std::size_t rhs_arity;
    algos::StdParamsMap params;
    double exp_epsilon;
    double exp_delta;

    FDPACVerifyingParams(CSVConfig const& csv_config, config::IndicesType&& lhs_indices,
                         config::IndicesType&& rhs_indices, double expected_epsilon,
                         double expected_delta, std::vector<double> lhs_deltas = {1},
                         double min_delta = 0.9, double min_epsilon = 0, double max_epsilon = 1,
                         unsigned long epsilon_steps = 100, bool dist_from_null_is_infinity = false,
                         std::vector<algos::pac_verifier::ValueMetric>&& lhs_metrics = {},
                         std::vector<algos::pac_verifier::ValueMetric>&& rhs_metrics = {})
        : rhs_arity(rhs_indices.size()),
          params({{kCsvConfig, csv_config},
                  {kLhsIndices, std::move(lhs_indices)},
                  {kRhsIndices, std::move(rhs_indices)},
                  {kLhsDeltas, std::move(lhs_deltas)},
                  {kMinDelta, min_delta},
                  {kMinEpsilon, min_epsilon},
                  {kMaxEpsilon, max_epsilon},
                  {kEpsilonSteps, epsilon_steps},
                  {kDistFromNullIsInfinity, dist_from_null_is_infinity},
                  {kLhsMetrics, std::move(lhs_metrics)},
                  {kRhsMetrics, std::move(rhs_metrics)}}),
          exp_epsilon(expected_epsilon),
          exp_delta(expected_delta) {}
};

class TestFDPACVerifier : public testing::TestWithParam<FDPACVerifyingParams> {};

TEST_P(TestFDPACVerifier, DefaultTest) {
    auto const& p = GetParam();
    auto verifier = algos::CreateAndLoadAlgorithm<algos::pac_verifier::FDPACVerifier<>>(p.params);
    verifier->Execute();

    auto const& pac = verifier->GetPAC();
    auto const& epsilons = pac.GetEpsilons();
    ASSERT_EQ(epsilons.size(), p.rhs_arity);
    for (std::size_t i = 0; i < p.rhs_arity; ++i) {
        EXPECT_NEAR(epsilons[i], p.exp_epsilon, kThreshold);
    }
    EXPECT_NEAR(pac.GetDelta(), p.exp_delta, kThreshold);
}

auto const kAlphabetMetric = [](std::string const& a, std::string const& b) {
    return std::abs(a.front() - b.front());
};

INSTANTIATE_TEST_SUITE_P(
        FDPACVerifierTests, TestFDPACVerifier,
        testing::Values(
                // Simple test with pure metric FD
                FDPACVerifyingParams(kMetricCoords, {2}, {3}, 0.06, 1, {0.2}),
                // Simple test with pure PFD
                FDPACVerifyingParams(kTestND, {1}, {2}, 0, 0.666, {0.2}, 0.5),
                // Custom metrics test
                FDPACVerifyingParams(kMushroom, {0}, {3}, 8.889, 0.676, {0}, 0.5, 0, 20, 100, false,
                                     {kAlphabetMetric}, {kAlphabetMetric}),
                // Multi-column values
                FDPACVerifyingParams(kTestFDPAC, {0, 1}, {2, 3}, 0.061, 0.89, {1, 10}, 0.7)));

using IndexPairs = std::vector<std::pair<std::size_t, std::size_t>>;
using Clusters = std::vector<std::vector<std::size_t>>;

// Indices are being sorted while clusteruzing, so, despite being not optimal, clusterization is
// stable (except for situations when partition depends on metric error)
struct FDPACHighlightParams {
    algos::StdParamsMap params;
    double eps_1;
    double eps_2;
    std::vector<double> lhs_diams;
    IndexPairs expected_highlight;
    Clusters expected_cluster;

    FDPACHighlightParams(CSVConfig const& csv_config, config::IndicesType&& lhs_indices,
                         config::IndicesType&& rhs_indices, IndexPairs&& expected_highlight,
                         Clusters&& expected_clusters, double eps_1 = -1, double eps_2 = -1,
                         std::vector<double> lhs_diams = {1e-3},
                         std::vector<double> lhs_deltas = {1}, double min_delta = 0.9,
                         double min_epsilon = 0, double max_epsilon = 1,
                         unsigned long epsilon_steps = 100, bool dist_from_null_is_infinity = false,
                         std::vector<algos::pac_verifier::ValueMetric>&& lhs_metrics = {},
                         std::vector<algos::pac_verifier::ValueMetric>&& rhs_metrics = {})
        : params({{kCsvConfig, csv_config},
                  {kLhsIndices, std::move(lhs_indices)},
                  {kRhsIndices, std::move(rhs_indices)},
                  {kLhsDeltas, std::move(lhs_deltas)},
                  {kMinDelta, min_delta},
                  {kMinEpsilon, min_epsilon},
                  {kMaxEpsilon, max_epsilon},
                  {kEpsilonSteps, epsilon_steps},
                  {kDistFromNullIsInfinity, dist_from_null_is_infinity},
                  {kLhsMetrics, std::move(lhs_metrics)},
                  {kRhsMetrics, std::move(rhs_metrics)}}),
          eps_1(eps_1),
          eps_2(eps_2),
          lhs_diams(std::move(lhs_diams)),
          expected_highlight(std::move(expected_highlight)),
          expected_cluster(std::move(expected_clusters)) {}
};

class TestFDPACHighlight : public testing::TestWithParam<FDPACHighlightParams> {};

// Binary matcher that applies UnorderedElementsAreArray(arg0) to arg1
MATCHER(BinaryUnorderedElementsAreArray, "") {
    return testing::Matches(testing::UnorderedElementsAreArray(std::get<0>(arg)))(std::get<1>(arg));
}

TEST_P(TestFDPACHighlight, DefaultTest) {
    auto const& p = GetParam();
    auto verifier = algos::CreateAndLoadAlgorithm<algos::pac_verifier::FDPACVerifier<>>(p.params);
    verifier->Execute();

    auto highlight = verifier->GetHighlights(p.eps_1, p.eps_2);
    auto higlight_indices = highlight.RowIndices();
    EXPECT_THAT(higlight_indices, testing::UnorderedElementsAreArray(p.expected_highlight));
    auto clusters = highlight.Clusterize(p.lhs_diams);
    Clusters cluster_indices(clusters.size());
    std::ranges::transform(clusters, cluster_indices.begin(),
                           std::mem_fn(&algos::pac_verifier::FDPACCluster::Indices));
    EXPECT_THAT(cluster_indices,
                testing::UnorderedPointwise(BinaryUnorderedElementsAreArray(), p.expected_cluster));
}

INSTANTIATE_TEST_SUITE_P(
        FDPACHighlightsTests, TestFDPACHighlight,
        testing::Values(
                // LHS values are "pure FD", so clusters are the same as would be in FD verifier
                FDPACHighlightParams(kMetricAddresses, {1}, {2},
                                     {{0, 3},
                                      {3, 0},
                                      {5, 6},
                                      {6, 5},
                                      {7, 9},
                                      {9, 7},
                                      {8, 9},
                                      {9, 8},
                                      {10, 12},
                                      {12, 10},
                                      {11, 12},
                                      {12, 11}},
                                     {{0, 3}, {5, 6}, {7, 8, 9}, {10, 11, 12}}, 5, 10),
                // FDPACVerifierTests/3
                FDPACHighlightParams(
                        kTestFDPAC, {0, 1}, {2, 3},
                        {{6, 7}, {7, 6}, {6, 8}, {8, 6}, {15, 16}, {16, 15}, {16, 17}, {17, 16}},
                        {{6, 7, 8}, {15, 16, 17}}, 0.7, 10, {0.8, 8}, {1, 10}, {}, 0.9, 0, 5)));
}  // namespace tests
