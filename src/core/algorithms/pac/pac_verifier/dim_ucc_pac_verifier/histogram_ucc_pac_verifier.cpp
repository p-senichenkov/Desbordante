#include "histogram_ucc_pac_verifier.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <string>

#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/histogram.h"
#include "core/algorithms/pac/pac_verifier/pac_verifier.h"
#include "core/config/column_index/type.h"
#include "core/config/descriptions.h"
#include "core/config/names.h"
#include "core/config/option.h"
#include "core/config/option_using.h"
#include "core/util/logger.h"

using namespace algos::pac_verifier;

double HistogramUCCPACVerifier::GetHistogramUpperBound() const {
    // TODO: This may be implemented much more effectively (though approximately)
    double max_dist = 0;
    for (std::size_t i = 0; i < tuples_->size() - 1; ++i) {
        for (std::size_t j = i + 1; j < tuples_->size(); ++j) {
            auto dist = (*metric_)((*tuples_)[i], (*tuples_)[j]);
            max_dist = std::max(max_dist, dist);
        }
    }
    return max_dist;
}

double HistogramUCCPACVerifier::GetHistogramUpperBoundApprox() const {
    // d(first, tuples[i]) + d(tuples[i], tuples[j]) >= d(first, tuples[j])
    // d(tuples[i], tuples[j]) <= 2 * max{d(first, tuples[i]), d(first, tuples[j])}
    double max_dist_single_dim = 0;
    auto const& first = tuples_->front();
    for (auto it = std::next(tuples_->begin()); it != tuples_->end(); ++it) {
        auto dist = (*metric_)(first, *it);
        max_dist_single_dim = std::max(max_dist_single_dim, dist);
    }
    return max_dist_single_dim * 2;
}

void HistogramUCCPACVerifier::BuildHistogram() {
    if (max_epsilon_ > 0) {
        histogram_ = utils::Histogram(delta_steps_ * histogram_bucket_count_multiplier_,
                                      min_epsilon_, max_epsilon_);
    } else {
        histogram_ = utils::Histogram{delta_steps_ * histogram_bucket_count_multiplier_,
                                      min_epsilon_, GetHistogramUpperBound()};
    }

    real_max_distance_ = 0;
    for (std::size_t i = 0; i < tuples_->size() - 1; ++i) {
        for (std::size_t j = i + 1; j < tuples_->size(); ++j) {
            auto dist = (*metric_)((*tuples_)[i], (*tuples_)[j]);
            histogram_.Record(dist, 2);
            if (dist == 0) {
                zero_dist_pairs_ += 2;
            }
            real_max_distance_ = std::max(real_max_distance_, dist);
        }
    }

    // Diagonal
    histogram_.Record(0, tuples_->size());
    zero_dist_pairs_ += tuples_->size();
}

std::vector<PACVerifier::EpsilonDelta> HistogramUCCPACVerifier::CalculateEmpiricalProbabilities() {
    std::vector<EpsilonDelta> result;
    auto buckets = histogram_.Buckets();

    auto zero_dist_delta = GetDelta(zero_dist_pairs_);
    if (zero_dist_delta > min_delta_) {
        result.emplace_back(0, zero_dist_delta);
    }

    std::vector<std::size_t> ecdf;
    std::vector<double> bucket_upper_bounds;
    ecdf.reserve(buckets.size());
    bucket_upper_bounds.reserve(buckets.size());
    std::size_t running_total = 0;
    for (auto const& bucket : buckets) {
        running_total += bucket.elem_count;
        ecdf.push_back(running_total);
        bucket_upper_bounds.push_back(bucket.upper_bound);

        auto delta = GetDelta(running_total);
        if (delta >= max_delta_) {
            break;
        }
        auto epsilon = bucket.upper_bound;
        result.emplace_back(epsilon, delta);
    }
    if (bucket_upper_bounds.empty()) {
        cached_ecdf_ = utils::Ecdf{{}, {running_total}};
    } else {
        // Make last bucket open
        bucket_upper_bounds.erase(std::prev(bucket_upper_bounds.end()));
        cached_ecdf_ = utils::Ecdf{bucket_upper_bounds, std::move(ecdf)};
    }

    // Ensure that (??, max delta) is always in empirical_probabilities
    if (max_delta_ > 1 - kDistThreshold) {
        // TODO: Seems strange
        result.emplace_back(real_max_distance_, 1);
    }
    return result;
}

// We don't have complex calculations with Greek letters now -- just explicitly count all pairs
double HistogramUCCPACVerifier::GetDelta(std::size_t num_pairs) const {
    assert(num_pairs <= total_pairs_);

    double delta = static_cast<double>(num_pairs) / total_pairs_;
    assert(delta >= -PACVerifier::kDistThreshold && delta <= 1 + PACVerifier::kDistThreshold);
    return delta;
}

double HistogramUCCPACVerifier::GetNumPairs(double delta) const {
    assert(0 <= delta && delta <= 1);
    return delta * total_pairs_;
}

PACVerifier::EpsilonDelta HistogramUCCPACVerifier::GetEpsilonDeltaForEpsilon(double epsilon) const {
    // It's guaranteed that GetEpsilonDeltaForEpsilon is always called after ExecuteInternal()
    assert(cached_ecdf_);

    auto [elem_count, upper_bound] = cached_ecdf_->Get(epsilon);
    // A little refinement: if bucket is empty, we can take arbitrary epsilon
    if (histogram_.Get(epsilon).first > 0) {
        epsilon = upper_bound;
    }
    return {epsilon, GetDelta(elem_count)};
}

void HistogramUCCPACVerifier::ExecuteInternal() {
    LogCommonOptions();

    // TODO: Since we build histogram in "execute-time", we can save a pass when max_epsilon is
    // given (but this requires a thorough review of PAC-Man).
    // Otherwise, we can move delta_steps to "configuration" and build a histogram in
    // "configure-time"
    BuildHistogram();
    LOG_DEBUG("Histogram: {}", histogram_.ToString());
    histogram_.Dump("histogram.txt");

    auto emp_probabilities = CalculateEmpiricalProbabilities();
    auto [epsilon, delta] = FindEpsilonDelta(std::move(emp_probabilities));

    std::vector<std::string> column_names;
    column_names.reserve(column_indices_.size());
    auto const& columns = TypedRelation().GetSchema()->GetColumns();
    std::ranges::transform(
            column_indices_, std::back_inserter(column_names),
            [&columns](config::IndexType idx) -> std::string { return columns[idx]->GetName(); });
    auto column_indices = column_indices_;
    pac_ = model::UCCPAC(std::move(column_indices), std::move(column_names), epsilon, delta);
}

HistogramUCCPACVerifier::HistogramUCCPACVerifier() {
    DESBORDANTE_OPTION_USING;

    RegisterOption(Option{&histogram_bucket_count_multiplier_, kHistogramMultiplier,
                          kDHistogramMultiplier, 1u});
    MakeOptionsAvailable({kHistogramMultiplier});
}
