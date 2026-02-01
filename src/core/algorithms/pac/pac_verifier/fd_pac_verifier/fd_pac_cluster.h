#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core/algorithms/pac/model/tuple.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/tuple_metric.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/tuple_pair.h"
#include "core/model/types/type.h"

namespace algos::pac_verifier {
/// @brief A set of tuples that have close LHSs, but different RHSs
class FDPACCluster {
private:
    std::shared_ptr<std::vector<TuplePair>> pairs_;
    std::shared_ptr<pac::model::Tuples> lhs_tuples_;
    std::shared_ptr<pac::model::Tuples> rhs_tuples_;
    std::shared_ptr<std::vector<::model::Type const*>> lhs_types_;
    std::shared_ptr<std::vector<::model::Type const*>> rhs_types_;
    std::shared_ptr<TupleMetric const> metrics_;
    std::vector<std::size_t> indices_;
    std::shared_ptr<std::vector<double>> exp_lhs_diams_;

    mutable std::vector<double> real_lhs_diams_;
    mutable std::vector<double> real_rhs_diams_;

    template <typename Res>
    std::vector<Res> TransformIndices(std::function<Res(std::size_t const)> func) const {
        std::vector<Res> result(indices_.size());
        std::ranges::transform(indices_, result.begin(), func);
        return result;
    }

    void CalculateRealDiameters() const {
        real_lhs_diams_ = std::vector<double>(lhs_types_->size(), 0);
        real_rhs_diams_ = std::vector<double>(rhs_types_->size(), 0);

        for (auto const i : indices_) {
            for (auto const j : indices_) {
                if (i != j) {
                    for (std::size_t col_idx = 0; col_idx < lhs_types_->size(); ++col_idx) {
                        real_lhs_diams_[col_idx] =
                                std::max(real_lhs_diams_[col_idx],
                                         metrics_->LhsDistOnColumn((*lhs_tuples_)[i],
                                                                   (*lhs_tuples_)[j], col_idx));
                    }
                    for (std::size_t col_idx = 0; col_idx < rhs_types_->size(); ++col_idx) {
                        real_rhs_diams_[col_idx] =
                                std::max(real_rhs_diams_[col_idx],
                                         metrics_->RhsDistOnColumn((*rhs_tuples_)[i],
                                                                   (*rhs_tuples_)[j], col_idx));
                    }
                }
            }
        }
    }

public:
    FDPACCluster(std::shared_ptr<std::vector<TuplePair>> pairs,
                 std::shared_ptr<pac::model::Tuples> lhs_tuples,
                 std::shared_ptr<pac::model::Tuples> rhs_tuples,
                 std::shared_ptr<std::vector<::model::Type const*>> lhs_types,
                 std::shared_ptr<std::vector<::model::Type const*>> rhs_types,
                 std::vector<std::size_t>&& indices, std::shared_ptr<std::vector<double>> lhs_diams)
        : pairs_(std::move(pairs)),
          lhs_tuples_(std::move(lhs_tuples)),
          rhs_tuples_(std::move(rhs_tuples)),
          lhs_types_(std::move(lhs_types)),
          rhs_types_(std::move(rhs_types)),
          indices_(std::move(indices)),
          exp_lhs_diams_(std::move(lhs_diams)) {}

    /// @brief Get row numbers of values that form the cluster
    std::vector<std::size_t> const& Indices() const {
        return indices_;
    }

    /// @brief Get @c Types of left-hand side columns of this cluster
    std::vector<::model::Type const*> const& LhsTypes() const {
        return *lhs_types_;
    }

    /// @brief Get @c Types of right-hand side columns of this cluster
    std::vector<::model::Type const*> const& RhsTypes() const {
        return *rhs_types_;
    }

    /// @brief Get LHS data as pointers to @c std::byte, that can be used with with types (see @c
    /// LhsTypes)
    pac::model::Tuples RawLhsData() const {
        return TransformIndices<pac::model::Tuple>(
                [this](std::size_t const idx) { return (*lhs_tuples_)[idx]; });
    }

    /// @brief Get RHS data as pointers to @c std::byte, that can be used with types (see @c
    /// RhsTypes)
    pac::model::Tuples RawRhsData() const {
        return TransformIndices<pac::model::Tuple>(
                [this](std::size_t const idx) { return (*rhs_tuples_)[idx]; });
    }

    /// @brief Get LHS values that form the cluster as strings
    std::vector<std::string> LhsStringData() const {
        return TransformIndices<std::string>([this](std::size_t const idx) {
            return pac::model::TupleToString((*lhs_tuples_)[idx], *lhs_types_);
        });
    }

    /// @brief Get RHS values that form the cluster as strings
    std::vector<std::string> RhsStringData() const {
        return TransformIndices<std::string>([this](std::size_t const idx) {
            return pac::model::TupleToString((*rhs_tuples_)[idx], *rhs_types_);
        });
    }

    /// @brief Get maximum distance between two values in cluster's LHS, which was requested when
    /// building this cluster
    std::vector<double> const& ExpectedLhsDiameters() const {
        return *exp_lhs_diams_;
    }

    std::vector<double> const& RealLhsDiameters() const {
        if (real_lhs_diams_.empty()) {
            CalculateRealDiameters();
        }
        return real_lhs_diams_;
    }

    /// @brief Get maximum distance between two values in this cluster's RHS
    std::vector<double> const& RealRhsDiameters() const {
        if (real_rhs_diams_.empty()) {
            CalculateRealDiameters();
        }
        return real_rhs_diams_;
    }
};
}  // namespace algos::pac_verifier
