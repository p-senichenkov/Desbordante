#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "core/algorithms/pac/pac.h"
#include "core/model/table/relational_schema.h"
#include "core/model/table/vertical.h"

namespace model {
/// @brief An FD PAC X -> Y specifies that if |t_i[A_l] - t_j[A_l]| <= Delta_l for each A_l in X,
/// then Pr(|t_i[B_l] - t_j[B_l]| <= eps_l) >= delta for each B_l in Y
class FDPAC : public PAC {
private:
    Vertical lhs_;
    Vertical rhs_;
    std::vector<double> lhs_Deltas_;

public:
    FDPAC() = default;

    FDPAC(const FDPAC&) = default;
    FDPAC(FDPAC&&) = default;
    FDPAC& operator=(const FDPAC&) = default;
    FDPAC& operator=(FDPAC&&) = default;

    FDPAC(std::shared_ptr<RelationalSchema const> rel_schema, Vertical&& lhs, Vertical&& rhs,
          std::vector<double> const& lhs_Deltas, std::vector<double>&& epsilons, double delta)
        : PAC(std::move(rel_schema), std::move(epsilons), {delta}),
          lhs_(std::move(lhs)),
          rhs_(std::move(rhs)),
          lhs_Deltas_(lhs_Deltas) {}

    ~FDPAC() override = default;

    Vertical const& GetLhs() const {
        return lhs_;
    }

    Vertical const& GetRhs() const {
        return rhs_;
    }

    std::vector<std::string> GetLhsColumnNames() const {
        std::vector<std::string> result(lhs_.GetArity());
        std::ranges::transform(lhs_.GetColumns(), result.begin(), std::mem_fn(&Column::GetName));
        return result;
    }

    std::vector<std::string> GetRhsColumnNames() const {
        std::vector<std::string> result(rhs_.GetArity());
        std::ranges::transform(rhs_.GetColumns(), result.begin(), std::mem_fn(&Column::GetName));
        return result;
    }

    std::vector<double> const& GetLhsDeltas() const {
        return lhs_Deltas_;
    }

    std::string ToShortString() const override {
        std::ostringstream oss;
        oss << lhs_.ToString() << " -(" << GetEpsilon() << ", " << GetDelta() << ")-> "
            << rhs_.ToString();
        return oss.str();
    }

    std::string ToLongString() const override {
        auto doubles_to_str = [](std::vector<double> const& doubles) {
            if (doubles.size() == 1) {
                return std::to_string(doubles.front());
            }
            std::ostringstream res;
            res << '{';
            for (auto it = doubles.begin(); it != doubles.end(); ++it) {
                if (it != doubles.begin()) {
                    res << ", ";
                }
                res << *it;
            }
            res << '}';
            return res.str();
        };
        std::ostringstream oss;
        oss << "FD PAC d(" << lhs_.ToString() << ") ≤ " << doubles_to_str(lhs_Deltas_)
            << "}) => Pr(d(" << rhs_.ToString() << ") ≤ " << doubles_to_str(GetEpsilons()) << ") ≥ "
            << GetDelta();
        return oss.str();
    }
};
}  // namespace model
