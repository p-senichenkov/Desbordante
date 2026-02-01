#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/algorithms/pac/model/tuple.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_cluster.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/tuple_metric.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/tuple_pair.h"
#include "core/model/types/type.h"

namespace algos::pac_verifier {
class FDPACHighlight {
private:
    using PairsIt = std::vector<TuplePair>::const_iterator;

    using LhsRhsData = std::pair<pac::model::Tuple, pac::model::Tuple>;
    using RawPair = std::pair<LhsRhsData, LhsRhsData>;
    using LhsRhsString = std::pair<std::string, std::string>;
    using StringPair = std::pair<LhsRhsString, LhsRhsString>;

    std::shared_ptr<pac::model::Tuples> lhs_tuples_;
    std::shared_ptr<pac::model::Tuples> rhs_tuples_;
    std::shared_ptr<std::vector<model::Type const*>> lhs_types_;
    std::shared_ptr<std::vector<model::Type const*>> rhs_types_;
    std::shared_ptr<TupleMetric const> metrics_;
    // Pairs are kept here to extend their lifetime
    std::shared_ptr<std::vector<TuplePair>> pairs_;
    // Range of pairs that for the highlight
    PairsIt begin_, end_;

    std::unordered_map<std::size_t, std::unordered_map<std::size_t, double>> cached_lhs_distances_;
    std::vector<std::size_t> cached_lhs_row_indices_;

    double DistOnColumn(std::size_t first_idx, std::size_t second_idx, std::size_t col_idx) const;

public:
    FDPACHighlight(std::shared_ptr<pac::model::Tuples> lhs_tuples,
                   std::shared_ptr<pac::model::Tuples> rhs_tuples,
                   std::shared_ptr<std::vector<model::Type const*>> lhs_types,
                   std::shared_ptr<std::vector<model::Type const*>> rhs_types,
                   std::shared_ptr<TupleMetric const> metrics,
                   std::shared_ptr<std::vector<TuplePair>> pairs, PairsIt begin, PairsIt end)
        : lhs_tuples_(std::move(lhs_tuples)),
          rhs_tuples_(std::move(rhs_tuples)),
          lhs_types_(std::move(lhs_types)),
          rhs_types_(std::move(rhs_types)),
          metrics_(std::move(metrics)),
          pairs_(std::move(pairs)),
          begin_(begin),
          end_(end) {}

    /// @brief Get pairs of row indices
    std::vector<std::pair<std::size_t, std::size_t>> RowIndices() const {
        std::vector<std::pair<std::size_t, std::size_t>> result;
        for (auto it = begin_; it != end_; ++it) {
            result.emplace_back(it->first_idx, it->second_idx);
            result.emplace_back(it->second_idx, it->first_idx);
        }
        return result;
    }

    /// @brief Get total number of pairs highlighted
    std::size_t NumPairs() const {
        // Each pair has a counterpart
        return 2 * std::distance(begin_, end_);
    }

    /// @brief Value @c types associated with left-hand side columns
    std::vector<model::Type const*> const& LhsTypes() const {
        return *lhs_types_;
    }

    /// @brief Value @c types associated with right-hand side columns
    std::vector<model::Type const*> const& RhsTypes() const {
        return *rhs_types_;
    }

    /// @brief Get pairs as pointers to @c std::byte that can be used with types (see @c LhsTypes,
    /// @c RhsTypes)
    std::vector<RawPair> RawData() const;

    /// @brief Get data as pairs of strings
    std::vector<StringPair> StringData() const;

    std::string ToString() const;

    /// @brief Break pairs into clusters with max distance between LHS values @c lhs_diam for each
    /// column.
    /// @note a) This doesn't guarantee optimal partition (i. e. minimal number of
    /// clusters).
    /// @note b) This operation may be heavy if highlight contains a lot of pairs.
    std::vector<FDPACCluster> Clusterize(std::vector<double> lhs_diams);
};
}  // namespace algos::pac_verifier
