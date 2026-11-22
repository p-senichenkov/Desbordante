#pragma once

#include "core/algorithms/algorithm.h"
#include "core/algorithms/pac/model/tuple.h"
#include "core/algorithms/pac/pac_verifier/util/pac_verifier_utils.h"
#include "core/algorithms/pac/pac_verifier/util/validation_mode.h"
#include "core/algorithms/pac/ucc_pac.h"
#include "core/config/custom_metric/custom_vector_metric/type.h"
#include "core/config/indices/type.h"
#include "core/config/names.h"
#include "core/config/tabular_data/input_table_type.h"
#include "core/model/table/column_layout_typed_relation_data.h"

namespace algos::pac_verifier {
// UCC PAC verifier without highlights
// Provides some base for optimizations
class DimUCCPACVerifier : public Algorithm {
private:
    config::InputTable input_table_;
    config::IndicesType column_indices_;
    config::CustomVectorMetricOptionType metric_opt_;

    PACValidationMode mode_;

    std::unique_ptr<model::ColumnLayoutTypedRelationData> typed_relation_;
    std::optional<model::UCCPAC> pac_;

    void MakeExecuteOptsAvailable() override {
        using namespace config::names;

        MakeOptionsAvailable({kPacValidationMode, kDiagonalThreshold});
    }

    void ExecuteInternal() override;

protected:
    config::CustomVectorMetricType metric_;
    double diagonal_threshold_;

    // Find PAC mode
    double min_epsilon_;
    double max_epsilon_;
    double max_delta_;
    unsigned long delta_steps_;

    // Find epsilon mode
    double delta_;

    // Find delta mode
    double epsilon_;

    std::vector<pac::model::Tuple> tuples_;

    void LoadDataInternal() override;

    void ResetState() override {
        pac_ = std::nullopt;
    }

    virtual EpsilonDelta FindPAC() = 0;
    virtual EpsilonDelta FindEpsilon() = 0;
    virtual EpsilonDelta FindDelta() = 0;

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
