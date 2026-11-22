#pragma once

#include <cstddef>
#include <optional>

#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/dim_ucc_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/histogram.h"
#include "core/algorithms/pac/pac_verifier/util/pac_verifier_utils.h"

namespace algos::pac_verifier {
class HistogramUCCPACVerifier final : public DimUCCPACVerifier {
private:
    // Histogram bucket count is delta_steps * histogram_bucket_count_multiplier_
    unsigned histogram_bucket_count_multiplier_;

    utils::Histogram histogram_;
    // TODO: Don't think it's so necessary
    std::size_t zero_dist_pairs_ = 0;
    // Histogram upper bound can be approximate. This value is exact
    double real_max_distance_ = 0;

    std::size_t total_pairs_;
    std::optional<utils::Ecdf> cached_ecdf_;

    // Lower bound is always 0
    // Maybe we could optimize things a little without this assumption, but I'm not sure
    double GetHistogramUpperBound() const;
    // Error is no more than 2x (currently)
    double GetHistogramUpperBoundApprox() const;
    void BuildHistogram();

    std::vector<EpsilonDelta> CalculateEmpiricalProbabilities();

    double GetNumPairs(double delta) const;
    double GetDelta(std::size_t num_pairs) const;

    EpsilonDelta GetEpsilonDeltaForEpsilon(double epsilon) const;

    void ResetState() override {
        DimUCCPACVerifier::ResetState();
        zero_dist_pairs_ = 0;
        real_max_distance_ = 0;
    }

    void LoadDataInternal() override {
        DimUCCPACVerifier::LoadDataInternal();
        total_pairs_ = tuples_.size() * tuples_.size();
    }

    EpsilonDelta FindPAC() override;
    EpsilonDelta FindEpsilon() override;
    EpsilonDelta FindDelta() override;

public:
    HistogramUCCPACVerifier();
};
}  // namespace algos::pac_verifier
