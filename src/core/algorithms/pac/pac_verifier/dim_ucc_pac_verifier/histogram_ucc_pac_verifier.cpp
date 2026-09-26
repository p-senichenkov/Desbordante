#include "histogram_ucc_pac_verifier.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>

#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/histogram.h"
#include "core/algorithms/pac/pac_verifier/pac_verifier.h"
#include "core/config/column_index/type.h"

using namespace algos::pac_verifier;

double HistogramUCCPACVerifier::GetHistogramUpperBound() const {
    // TODO: This may be implemented much more effectively (though approximately)
    double max_dist = 0;
    for (std::size_t i = 0; i < tuples_->size() - 1; ++i) {
        for (std::size_t j = i + 1; j < tuples_->size(); ++j) {
            auto dist = metric_->Dist(tuple_type_->GetTypes(), (*tuples_)[i], (*tuples_)[j]);
            max_dist = std::max(max_dist, dist);
        }
    }
    return max_dist;
}

void HistogramUCCPACVerifier::BuildHistogram() {
    max_histogram_bound_ = GetHistogramUpperBound();
    histogram_ = utils::Histogram{DeltaSteps(), 0, max_histogram_bound_};

    for (std::size_t i = 0; i < tuples_->size() - 1; ++i) {
        for (std::size_t j = i + 1; j < tuples_->size(); ++j) {
            auto dist = metric_->Dist(tuple_type_->GetTypes(), (*tuples_)[i], (*tuples_)[j]);
            histogram_.Record(dist, 2);
            if (dist == 0) {
                zero_dist_pairs_ += 2;
            }
        }
    }

    // Diagonal
    histogram_.Record(0, tuples_->size());
    zero_dist_pairs_ += tuples_->size();
}

std::vector<PACVerifier::EpsilonDelta> HistogramUCCPACVerifier::CalculateEmpiricalProbabilities()
        const {
    std::vector<EpsilonDelta> result;
    auto buckets = histogram_.Buckets();

    auto zero_dist_delta = GetDelta(zero_dist_pairs_);
    if (zero_dist_delta > MinDelta()) {
        result.emplace_back(0, zero_dist_delta);
    }

    std::size_t running_total = 0;
    for (auto const& bucket : buckets) {
        running_total += bucket.elem_count;
        auto delta = GetDelta(running_total);
        if (delta >= MinDelta()) {
            auto epsilon = bucket.max_bound;
            result.emplace_back(epsilon, delta);
        }
    }
    // Ensure that (??, max delta) is always in empirical_probabilities
    if (MaxDelta() > 1 - kDistThreshold) {
        // TODO: Seems strange
        result.emplace_back(max_histogram_bound_, 1);
    }
    return result;
}

PACVerifier::EpsilonDelta HistogramUCCPACVerifier::GetEpsilonDeltaForEpsilon(double epsilon) const {
    auto [elem_count, upper_bound] = histogram_.GetElemCount(epsilon);
    return {upper_bound, GetDelta(elem_count)};
}

void HistogramUCCPACVerifier::ExecuteInternal() {
    LogCommonOptions();

    BuildHistogram();

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
