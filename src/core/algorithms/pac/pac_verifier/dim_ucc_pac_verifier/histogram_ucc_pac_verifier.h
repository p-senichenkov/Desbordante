#pragma once

#include <cstddef>

#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/dim_ucc_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/histogram.h"

namespace algos::pac_verifier {
class HistogramUCCPACVerifier final : public DimUCCPACVerifier {
private:
    utils::Histogram histogram_;
    // TODO: Don't think it's so necessary (both variables below)
    std::size_t zero_dist_pairs_ = 0;
    double max_histogram_bound_;

    // Lower bound is always 0
    // Maybe we could optimize things a little without this assumption, but I'm not sure
    double GetHistogramUpperBound() const;
    void BuildHistogram();
    std::vector<EpsilonDelta> CalculateEmpiricalProbabilities() const;

    EpsilonDelta GetEpsilonDeltaForEpsilon(double epsilon) const override;
    void ExecuteInternal() override;

    void ResetState() override {
        DimUCCPACVerifier::ResetState();
        zero_dist_pairs_ = 0;
    }
};
}  // namespace algos::pac_verifier
