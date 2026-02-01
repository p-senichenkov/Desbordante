#pragma once

#include <algorithm>
#include <memory>
#include <sstream>
#include <variant>
#include <vector>

#include "core/algorithms/pac/fd_pac.h"
#include "core/algorithms/pac/model/tuple.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/column_metric.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_highlight.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/tuple_metric.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/tuple_pair.h"
#include "core/algorithms/pac/pac_verifier/pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/util/make_tuples.h"
#include "core/config/descriptions.h"
#include "core/config/indices/type.h"
#include "core/config/names.h"
#include "core/config/option_using.h"
#include "core/model/types/type.h"
#include "core/util/bitset_utils.h"
#include "core/util/logger.h"

namespace algos::pac_verifier {
// FD PAC X -> Y specifies that
// if |t_i[A_l] - t_j[A_l]| <= Delta_l for each A_l in X
// then Pr(|t_i[B_l] - t_j[B_l]| <= eps_l) >= delta for each B_l in Y
//
// NOTE: PAC-Man based algorihtms (including this one) can handle only PACs that have
// eps_1 = ... = eps_n = eps
//
// Key steps of the algorithm:
// 1. Select subset Gamma of r^2:
//    Gamma = {(t_i, t_j) : |t_i[A_l] - t_j[A_l]| <= Delta_l for each A_l in X}
// 3. Sort Gamma by max|t_i[B_l] - t_j[B_l]|
// 4. For each eps(n) find sigma(n):
//    sigma(n) = {(t_i, t_j) in Gamma : |t_i[B_l] - t_j[B_l]| <= eps(n) for each B_l in Y}
//    by widening sigma(n - 1). See Domain PAC verifier for more details on this approach.
// 5. Calculate empirical probability for each eps(n) as follows:
//	   emp_prob(n) = |sigma(n)| / |Gamma|
//
// Optimizations and other notes:
// 1. For each pair (t_i, t_j) in Gamma select l0, l1 such that
//    |t_i[A_l0] - t_j[A_l0]| = max|t_i[A_l] - t_j[A_l]| for each A_l in X
//    |t_i[B_l1] - t_j[B_l1]| = max|t_i[B_l] - t_j[B_l]| for each B_l in Y
//    It's obvious that only A_l0 and B_l1 must be considered.
// 2. Only subset Gamma' of Gamma needs to be considered:
//    Gamma' = {(t_i, t_j) in Gamma : i < j}
//    sigma' = sigma \cap Gamma'
//    Thus empirical probability becomes
//    emp_prob = (2|sigma'(n)| + |r|) / (2|Gamma| + |r|)

/// @brief Functional Probabilistic Approximate Constraints verifier.
/// FDPACVerifier<true> is C++/Python version, which takes @c vectors of @c std::functions as
/// metrics. FDPACVerifier<false> is CLI version, which always uses default metrics.
template <bool MetricOpt = true>
class FDPACVerifier : public PACVerifier {
private:
    using Pairs = std::vector<TuplePair>;
    using PairsIt = Pairs::const_iterator;

    config::IndicesType lhs_indices_;
    config::IndicesType rhs_indices_;

    std::shared_ptr<std::vector<model::Type const*>> lhs_types_;
    std::shared_ptr<std::vector<model::Type const*>> rhs_types_;

    std::vector<ValueMetric> lhs_metrics_opt_;
    std::vector<ValueMetric> rhs_metrics_opt_;

    std::shared_ptr<TupleMetric const> metrics_;

    std::vector<double> lhs_Deltas_;

    std::shared_ptr<pac::model::Tuples> lhs_tuples_;
    std::shared_ptr<pac::model::Tuples> rhs_tuples_;

    // Gamma is a set of pairs such that |t_i[A_l] - t_j[A_l]| <= Delta_l for each A_l in X.
    // This vector holds only RHSs of such pairs, sorted by max{|t_i[B_l] - t_j[B_l]|}
    std::shared_ptr<Pairs> sorted_gamma_;

    /// @brief Fill sorted_gamma. Called in Execute, because Deltas is an execute option.
    void PreparePairs();

    std::vector<std::size_t> CountSatisfyingPairs() const;

    void ProcessPACTypeOptions() override;
    void PreparePACTypeData() override;
    unsigned long long ExecuteInternal() override;

    void MakeExecuteOptsAvailable() override {
        PACVerifier::MakeExecuteOptsAvailable();
        MakeOptionsAvailable({config::names::kLhsDeltas});
    }

public:
    FDPACVerifier();

    FDPACHighlight GetHighlights(double eps_1 = -1, double eps_2 = -1) const;
};

template <bool MetricOpt>
void FDPACVerifier<MetricOpt>::PreparePairs() {
    // All pairs have first_idx < second_idx. See "key ideas".
    sorted_gamma_ = std::make_shared<Pairs>();
    auto const num_rows = TypedRelation().GetNumRows();
    for (std::size_t i = 0; i < num_rows; ++i) {
        for (std::size_t j = i + 1; j < num_rows; ++j) {
            auto const& first_lhs = (*lhs_tuples_)[i];
            auto const& second_lhs = (*lhs_tuples_)[j];
            bool add_to_gamma = true;
            double max_lhs_dist = metrics_->LhsDist(first_lhs, second_lhs);
            for (std::size_t col_num = 0; col_num < first_lhs.size(); ++col_num) {
                auto lhs_dist = metrics_->LhsDistOnColumn(first_lhs, second_lhs, col_num);
                if (lhs_dist > lhs_Deltas_[col_num]) {
                    add_to_gamma = false;
                    break;
                }
                max_lhs_dist = std::max(max_lhs_dist, lhs_dist);
            }
            if (add_to_gamma) {
                auto const& first_rhs = (*rhs_tuples_)[i];
                auto const& second_rhs = (*rhs_tuples_)[j];
                double max_rhs_dist = metrics_->RhsDist(first_rhs, second_rhs);
                sorted_gamma_->push_back(TuplePair{i, j, max_lhs_dist, max_rhs_dist});
            }
        }
    }
    std::ranges::sort(*sorted_gamma_, {}, [](TuplePair const& p) { return p.rhs_dist; });

    LOG_TRACE("Sorted gamma:");
    for ([[maybe_unused]] auto const& pair : *sorted_gamma_) {
        LOG_TRACE("\tFirst: {}, second: {}, lhs_dist: {}, rhs_dist: {}", pair.first_idx,
                  pair.second_idx, pair.lhs_dist, pair.rhs_dist);
    }
}

template <bool MetricOpt>
std::vector<std::size_t> FDPACVerifier<MetricOpt>::CountSatisfyingPairs() const {
    std::vector<std::size_t> satisf_pairs;

    auto after_last_it = sorted_gamma_->begin();
    // Repeatedly widen sigma(n)
    auto eps_step = (MaxEpsilon() - MinEpsilon()) / (EpsilonSteps() - 1);
    auto eps = MinEpsilon();
    for (std::size_t step = 0; step < EpsilonSteps(); ++step) {
        eps += eps_step;
        after_last_it = std::ranges::partition_point(
                after_last_it, sorted_gamma_->end(),
                [eps](TuplePair const& p) { return p.rhs_dist <= eps; });
        satisf_pairs.push_back(std::distance(sorted_gamma_->begin(), after_last_it));
    }
    return satisf_pairs;
}

template <bool MetricOpt>
void FDPACVerifier<MetricOpt>::ProcessPACTypeOptions() {
    auto const& col_data = TypedRelation().GetColumnData();
    lhs_types_ = std::make_shared<std::vector<model::Type const*>>(lhs_indices_.size());
    std::ranges::transform(lhs_indices_, lhs_types_->begin(),
                           [&col_data](std::size_t const idx) { return &col_data[idx].GetType(); });

    rhs_types_ = std::make_shared<std::vector<model::Type const*>>(rhs_indices_.size());
    std::ranges::transform(rhs_indices_, rhs_types_->begin(),
                           [&col_data](std::size_t const idx) { return &col_data[idx].GetType(); });

    while (lhs_metrics_opt_.size() < lhs_indices_.size()) {
        lhs_metrics_opt_.emplace_back();
    }
    std::vector<ColumnMetric> lhs_metrics(lhs_indices_.size());
    std::ranges::transform(
            *lhs_types_, lhs_metrics_opt_, lhs_metrics.begin(),
            [dist_is_infty{DistFromNullIsInfty()}](model::Type const* type, ValueMetric v_metric) {
                return std::visit(
                        [&type, dist_is_infty](auto&& arg) {
                            return ColumnMetric{type, std::move(arg), dist_is_infty};
                        },
                        v_metric);
            });

    while (rhs_metrics_opt_.size() < rhs_indices_.size()) {
        rhs_metrics_opt_.emplace_back();
    }
    std::vector<ColumnMetric> rhs_metrics(rhs_indices_.size());
    std::ranges::transform(
            *rhs_types_, rhs_metrics_opt_, rhs_metrics.begin(),
            [dist_is_infty{DistFromNullIsInfty()}](model::Type const* type, ValueMetric v_metric) {
                return std::visit(
                        [&type, dist_is_infty](auto&& arg) {
                            return ColumnMetric{type, std::move(arg), dist_is_infty};
                        },
                        v_metric);
            });
    metrics_ = std::make_shared<TupleMetric>(std::move(lhs_metrics), std::move(rhs_metrics));
}

template <bool MetricOpt>
void FDPACVerifier<MetricOpt>::PreparePACTypeData() {
    using namespace pac::util;

    lhs_tuples_ = MakeTuples(TypedRelation().GetColumnData(), lhs_indices_);
    rhs_tuples_ = MakeTuples(TypedRelation().GetColumnData(), rhs_indices_);
}

template <bool MetricOpt>
unsigned long long FDPACVerifier<MetricOpt>::ExecuteInternal() {
    auto start = std::chrono::system_clock::now();

    while (lhs_Deltas_.size() < lhs_indices_.size()) {
        lhs_Deltas_.push_back(lhs_Deltas_.back());
    }

    auto vec_to_str = [](auto const& vec) -> std::string {
        std::ostringstream oss;
        oss << '{';
        for (auto it = vec.begin(); it != vec.end(); ++it) {
            if (it != vec.begin()) {
                oss << ", ";
            }
            oss << *it;
        }
        oss << '}';
        return oss.str();
    };
    LOG_INFO("Verifying FD PAC {} -> {} with LHS Deltas = {}", vec_to_str(lhs_indices_),
             vec_to_str(rhs_indices_), vec_to_str(lhs_Deltas_));

    PreparePairs();

    auto satisf_pairs = CountSatisfyingPairs();
    std::vector<double> emp_probabilities(satisf_pairs.size());
    auto num_rows = TypedRelation().GetNumRows();
    auto denominator = 2 * sorted_gamma_->size() + num_rows;
    std::ranges::transform(satisf_pairs, emp_probabilities.begin(),
                           [num_rows, denominator](std::size_t const sat_pairs) {
                               return static_cast<double>(2 * sat_pairs + num_rows) / denominator;
                           });
    auto [epsilon, delta] = FindEpsilonDelta(emp_probabilities);
    // By definition, FD PAC has different epsilons for each RHS column, but this algo cannot verify
    // such FDs
    std::vector<double> epsilons(rhs_indices_.size(), epsilon);
    auto const schema = TypedRelation().GetSharedPtrSchema();
    MakePAC<model::FDPAC>(
            schema,
            schema->GetVertical(util::IndicesToBitset(lhs_indices_.begin(), lhs_indices_.end(),
                                                      TypedRelation().GetNumColumns())),
            schema->GetVertical(util::IndicesToBitset(rhs_indices_.begin(), rhs_indices_.end(),
                                                      TypedRelation().GetNumColumns())),
            lhs_Deltas_, std::move(epsilons), delta);

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now() - start)
                           .count();
    LOG_INFO("Result: {}", GetPAC().ToLongString());
    LOG_INFO("Validation took {}ms", elapsed);
    return elapsed;
}

template <bool MetricOpt>
FDPACVerifier<MetricOpt>::FDPACVerifier() : PACVerifier() {
    DESBORDANTE_OPTION_USING;

    RegisterOption(Option(&lhs_indices_, kLhsIndices, kDLhsIndices));
    RegisterOption(Option(&rhs_indices_, kRhsIndices, kDRhsIndices));
    if constexpr (MetricOpt) {
        RegisterOption(
                Option(&lhs_metrics_opt_, kLhsMetrics, kDLhsMetrics, std::vector<ValueMetric>{}));
        RegisterOption(
                Option(&rhs_metrics_opt_, kRhsMetrics, kDRhsMetrics, std::vector<ValueMetric>{}));
    }
    RegisterOption(Option(&lhs_Deltas_, kLhsDeltas, kDLhsDeltas)
                           .SetNormalizeFunc([this](std::vector<double>& value) {
                               while (value.size() < lhs_indices_.size()) {
                                   value.push_back(value.back());
                               }
                           }));

    MakeOptionsAvailable({kLhsIndices, kRhsIndices});
    if constexpr (MetricOpt) {
        MakeOptionsAvailable({kLhsMetrics, kRhsMetrics});
    }
}

template <bool MetricOpt>
FDPACHighlight FDPACVerifier<MetricOpt>::GetHighlights(double eps_1, double eps_2) const {
    if (eps_1 < 0) {
        eps_1 = MinEpsilon();
    }
    if (eps_2 < 0) {
        eps_2 = GetPAC().GetEpsilons().front();
    }
    LOG_TRACE("Calculating higlights from {} to {}...", eps_1, eps_2);

    auto begin = std::ranges::partition_point(
            *sorted_gamma_, [eps_1](TuplePair const& p) { return p.rhs_dist <= eps_1; });
    auto end = std::ranges::partition_point(
            begin, sorted_gamma_->end(),
            [eps_2](TuplePair const& p) { return p.rhs_dist <= eps_2; });

    LOG_TRACE("Highlighted pairs [{}, {})", std::distance(sorted_gamma_->begin(), begin),
              std::distance(sorted_gamma_->begin(), end));
    return FDPACHighlight{lhs_tuples_, rhs_tuples_,   lhs_types_, rhs_types_,
                          metrics_,    sorted_gamma_, begin,      end};
}
}  // namespace algos::pac_verifier
