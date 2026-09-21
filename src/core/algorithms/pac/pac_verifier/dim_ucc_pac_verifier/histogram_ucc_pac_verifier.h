#pragma once

#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/dim_ucc_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/histogram.h"

namespace algos::pac_verifier {
class HistogramUCCPACVerifier final : public DimUCCPACVerifier {
private:
    utils::Histogram histogram_;

    // Lower bound is always 0
    // Maybe we could optimize things a little without this assumption, but I'm not sure
    double GetHistogramUpperBound() const;
    void BuildHistogram();
    std::vector<EpsilonDelta> CalculateEmpiricalProbabilities() const;

    // TODO:
    EpsilonDelta GetEpsilonDeltaForEpsilon(double epsilon) const override;

    void PreparePACTypeData() override;
    void ExecuteInternal() override;
};
}  // namespace algos::pac_verifier
