#pragma once

#include <cstddef>
#include <vector>

#include "core/algorithms/pac/pac_verifier/util/pac_verifier_utils.h"
#include "core/algorithms/pac/pac_verifier/util/tuple_pair.h"

namespace algos::pac_verifier {
/// @brief Utility class for PAC-Man-based PAC verifiers that work with pairs of tuples
class PairwiseEmpiricalProbabilitiesCalculator {
private:
    double const min_delta_;
    double const max_delta_;
    unsigned long const delta_steps_;

protected:
    using Pairs = std::vector<TuplePair>;
    using PairsIt = Pairs::const_iterator;

    Pairs const& sorted_pairs_;

    PairwiseEmpiricalProbabilitiesCalculator(double min_delta, double max_delta, double delta_steps,
                                             Pairs const& sorted_pairs)
        : min_delta_(min_delta),
          max_delta_(max_delta),
          delta_steps_(delta_steps),
          sorted_pairs_(sorted_pairs) {}

    /// @brief Get delta for a given number of pairs
    virtual double GetDelta(std::size_t num_pairs) const = 0;
    /// @brief Get number of pairs for a given delta
    /// The value shouldn't be rounded
    virtual double GetNumPairs(double delta) const = 0;
    /// @brief Given a (eps, delta) pair, get (eps, delta') pair, that is better in some way
    /// Different PAC verifiers may have different refinement tactics
    /// Must produce a dereferenceable iterator (i. e. may not produce end())
    virtual void RefineDelta(PairsIt&) const = 0;

public:
    /// @brief For each delta_i find the least eps_i such that PAC_{eps_i}^{delta_i} holds.
    /// Then refine delta_i, i. e. find the greatest delta_i' such that PAC_{eps_i}^{delta_i} holds.
    /// @return (eps_i, delta_i') pairs
    std::vector<EpsilonDelta> CalculateEmpiricalProbabilities() const;
    EpsilonDelta GetEpsilonDeltaForEpsilonImpl(double epsilon) const;
};
}  // namespace algos::pac_verifier
