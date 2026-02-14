#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

#include "core/algorithms/pac/model/tuple.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/column_metric.h"

namespace algos::pac_verifier {
/// @brief Holds @c ColumnMetrics for all columns and calculates distance between tuples in terms
/// of FD PAC verifier, i. e. as the maximum distance between values
class TupleMetric {
private:
    std::vector<ColumnMetric> lhs_metrics_;
    std::vector<ColumnMetric> rhs_metrics_;

    double Distance(pac::model::Tuple const& a, pac::model::Tuple const& b,
                    std::vector<ColumnMetric> const& metrics) const {
        assert(a.size() == metrics.size());
        assert(b.size() == metrics.size());

        double max_dist = 0;
        for (std::size_t col_num = 0; col_num < metrics.size(); ++col_num) {
            max_dist = std::max(max_dist, metrics[col_num].Dist(a[col_num], b[col_num]));
        }
        return max_dist;
    }

public:
    TupleMetric(std::vector<ColumnMetric>&& lhs_metrics, std::vector<ColumnMetric>&& rhs_metrics)
        : lhs_metrics_(std::move(lhs_metrics)), rhs_metrics_(std::move(rhs_metrics)) {}

    double LhsDistOnColumn(pac::model::Tuple const& a, pac::model::Tuple const& b,
                           std::size_t col_num) const {
        return lhs_metrics_[col_num].Dist(a[col_num], b[col_num]);
    }

    double RhsDist(pac::model::Tuple const& a, pac::model::Tuple const& b) const {
        return Distance(a, b, rhs_metrics_);
    }
};
}  // namespace algos::pac_verifier
