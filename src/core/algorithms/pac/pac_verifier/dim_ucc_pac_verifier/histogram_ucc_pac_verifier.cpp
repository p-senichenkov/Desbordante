#include "histogram_ucc_pac_verifier.h"

#include <algorithm>
#include <cstddef>

#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/histogram.h"
#include "core/algorithms/pac/pac_verifier/pac_verifier.h"

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
    histogram_ = utils::Histogram{DeltaSteps(), 0, GetHistogramUpperBound()};

    for (std::size_t i = 0; i < tuples_->size() - 1; ++i) {
        for (std::size_t j = i + 1; j < tuples_->size(); ++j) {
            auto dist = metric_->Dist(tuple_type_->GetTypes(), (*tuples_)[i], (*tuples_)[j]);
            histogram_.Record(dist, 2);
        }
    }

    // Diagonal
    histogram_.Record(0, tuples_->size());
}

std::vector<PACVerifier::EpsilonDelta> HistogramUCCPACVerifier::CalculateEmpiricalProbabilities()
        const {
    std::vector<PACVerifier::EpsilonDelta> result;
    auto buckets = histogram_.Buckets();
    std::size_t running_total = 0;

    // TODO: To preserve such behavior we have to store zero-distance pairs count explicitly
    // auto no_pairs_delta = GetDelta(curr_size);
    // if (no_pairs_delta > MinDelta()) {
    //     result.emplace_back(0, GetDelta(curr_size));
    // }

    if (MinDelta() <= 0) {
        result.emplace_back(0, GetDelta(0));
    }
    for (auto const& bucket : buckets) {
        // TODO:
    }
}
