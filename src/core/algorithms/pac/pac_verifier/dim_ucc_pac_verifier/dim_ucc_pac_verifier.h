#pragma once

#include "core/algorithms/pac/model/tuple_type.h"
#include "core/algorithms/pac/pac_verifier/pac_verifier.h"
#include "core/algorithms/pac/ucc_pac.h"
#include "core/config/indices/type.h"
#include "core/util/custom_metric/custom_vector_metric.h"

namespace algos::pac_verifier {
// UCC PAC verifier without highlights
// Provides some base for optimizations
class DimUCCPACVerifier : public PACVerifier {
protected:
    config::IndicesType column_indices_;
    std::shared_ptr<util::ICustomVectorMetric> metric_;

    std::shared_ptr<pac::model::TupleType> tuple_type_;
    std::shared_ptr<std::vector<pac::model::Tuple>> tuples_;

    std::optional<model::UCCPAC> pac_;

    double GetNumPairs(double delta) const;
    double GetDelta(std::size_t num_pairs) const;

    void ResetState() override {
        pac_ = std::nullopt;
    }

    void PreparePACTypeData() override;

public:
    DimUCCPACVerifier();

    model::UCCPAC const& GetPAC() const {
        if (!pac_) {
            throw std::runtime_error("Execute must called before GetPAC");
        }
        return *pac_;
    }
};
}  // namespace algos::pac_verifier
