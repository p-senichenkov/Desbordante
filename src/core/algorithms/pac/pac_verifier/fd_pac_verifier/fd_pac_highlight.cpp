#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_highlight.h"

#include <algorithm>
#include <cstddef>
#include <forward_list>
#include <iterator>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_cluster.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/tuple_pair.h"

namespace algos::pac_verifier {
double FDPACHighlight::DistOnColumn(std::size_t first_idx, std::size_t second_idx,
                                    std::size_t col_idx) const {
    if (first_idx > second_idx) {
        std::swap(first_idx, second_idx);
    }
    auto const& first_tp = (*lhs_tuples_)[first_idx];
    auto const& second_tp = (*lhs_tuples_)[second_idx];
    return metrics_->LhsDistOnColumn(first_tp, second_tp, col_idx);
}

std::vector<FDPACHighlight::RawPair> FDPACHighlight::RawData() const {
    std::vector<RawPair> result;
    for (auto it = begin_; it != end_; ++it) {
        auto first_idx = it->first_idx;
        auto second_idx = it->second_idx;
        LhsRhsData first{(*lhs_tuples_)[first_idx], (*rhs_tuples_)[first_idx]};
        LhsRhsData second{(*lhs_tuples_)[second_idx], (*rhs_tuples_)[second_idx]};
        result.emplace_back(std::move(first), std::move(second));
    }
    return result;
}

std::vector<FDPACHighlight::StringPair> FDPACHighlight::StringData() const {
    std::vector<StringPair> result;
    for (auto it = begin_; it != end_; ++it) {
        auto first_idx = it->first_idx;
        auto second_idx = it->second_idx;
        LhsRhsString first{pac::model::TupleToString((*lhs_tuples_)[first_idx], *lhs_types_),
                           pac::model::TupleToString((*rhs_tuples_)[first_idx], *rhs_types_)};
        LhsRhsString second{pac::model::TupleToString((*lhs_tuples_)[second_idx], *lhs_types_),
                            pac::model::TupleToString((*rhs_tuples_)[second_idx], *rhs_types_)};
        result.emplace_back(std::move(first), std::move(second));
    }
    return result;
}

std::string FDPACHighlight::ToString() const {
    auto string_pairs = StringData();
    std::ostringstream result;
    result << '[';
    for (auto it = string_pairs.begin(); it != string_pairs.end(); ++it) {
        if (it != string_pairs.begin()) {
            result << ", ";
        }
        auto const& [first_str, second_str] = *it;
        result << "({" << first_str.first << " -> " << first_str.second << "}, {"
               << second_str.first << " -> " << second_str.second << "})";
    }
    result << ']';
    return result.str();
}

std::vector<FDPACCluster> FDPACHighlight::Clusterize(std::vector<double> lhs_diams) {
    while (lhs_diams.size() < lhs_types_->size()) {
        lhs_diams.push_back(lhs_diams.back());
    }
    auto shared_diams = std::make_shared<std::vector<double>>(std::move(lhs_diams));

    std::vector<FDPACCluster> clusters;

    /// Simple greedy partitioning algorithm:
    /// While there are tuples:
    ///   1. Select arbitary tuple T
    ///   2. Create cluster C = {T}
    ///   3. For each tuple P: add P to C if |T - P| < eps
    std::forward_list<std::size_t> lhs_row_indices;
    for (auto it = begin_; it != end_; ++it) {
        lhs_row_indices.push_front(it->first_idx);
        lhs_row_indices.push_front(it->second_idx);
    }
    lhs_row_indices.sort();
    lhs_row_indices.unique();

    while (!lhs_row_indices.empty()) {
        auto leader_idx = lhs_row_indices.front();
        lhs_row_indices.pop_front();
        std::vector<std::size_t> cluster_indices{leader_idx};
        auto close_to_leader = [this, leader_idx, shared_diams](std::size_t const idx) {
            for (std::size_t col_num = 0; col_num < lhs_types_->size(); ++col_num) {
                if (DistOnColumn(leader_idx, idx, col_num) > (*shared_diams)[col_num]) {
                    return false;
                }
            }
            return true;
        };
        std::ranges::copy_if(lhs_row_indices, std::back_inserter(cluster_indices), close_to_leader);
        std::erase_if(lhs_row_indices, close_to_leader);

        clusters.emplace_back(pairs_, lhs_tuples_, rhs_tuples_, lhs_types_, rhs_types_,
                              std::move(cluster_indices), shared_diams);
    }
    return clusters;
}
}  // namespace algos::pac_verifier
