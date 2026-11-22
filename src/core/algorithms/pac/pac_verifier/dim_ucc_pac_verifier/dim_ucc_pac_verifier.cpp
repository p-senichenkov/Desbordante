#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/dim_ucc_pac_verifier.h"

#include <algorithm>

#include "core/algorithms/pac/model/tuple.h"
#include "core/algorithms/pac/pac_verifier/options.h"
#include "core/config/custom_metric/custom_vector_metric/option.h"
#include "core/config/descriptions.h"
#include "core/config/indices/option.h"
#include "core/config/names.h"
#include "core/config/option_using.h"
#include "core/config/tabular_data/input_table/option.h"
#include "core/util/logger.h"

using namespace algos::pac_verifier;

namespace {
pac::model::Tuples MakeTuples(std::vector<::model::TypedColumnData> const& col_data,
                              config::IndicesType const& indices) {
    std::vector<std::vector<std::byte const*> const*> columns_data;
    columns_data.reserve(indices.size());
    std::ranges::transform(
            indices, std::back_inserter(columns_data),
            [&col_data](auto const col_idx) { return &col_data[col_idx].GetData(); });

    auto num_rows = col_data.front().GetNumRows();
    pac::model::Tuples tuples;
    tuples.reserve(num_rows);
    for (std::size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
        pac::model::Tuple tuple(indices.size());
        std::ranges::transform(columns_data, tuple.begin(),
                               [row_idx](auto const* col_data) { return (*col_data)[row_idx]; });
        tuples.push_back(tuple);
    }
    return tuples;
}
}  // namespace

void DimUCCPACVerifier::ExecuteInternal() {
    LOG_DEBUG("Common PAC verifier options: delta: [0, {}], eps: [{}, {}], delta steps: {}",
              max_delta_, min_epsilon_, max_epsilon_, delta_steps_);
    std::ostringstream oss;
    oss << '{';
    for (auto it = column_indices_.begin(); it != column_indices_.end(); ++it) {
        if (it != column_indices_.begin()) {
            oss << ", ";
        }
        oss << *it;
    }
    oss << '}';
    LOG_INFO("Verifying UCC PAC on columns {}", oss.str());

    EpsilonDelta eps_delta;
    auto [mode, eps_override, delta_override] =
            SelectPACValidationMode(mode_, min_epsilon_, max_epsilon_, 0, max_delta_);
    switch (mode) {
        case PACValidationMode::kFindPac:
            eps_delta = FindPAC();
            break;
        case PACValidationMode::kFindEpsilon:
            if (delta_override) {
                delta_ = *delta_override;
            }
            eps_delta = FindEpsilon();
            break;
        case PACValidationMode::kFindDelta:
            if (eps_override) {
                epsilon_ = *eps_override;
            }
            eps_delta = FindDelta();
            break;
    }

    Vertical columns = typed_relation_->GetSchema()->GetVertical(
            util::IndicesToBitset(column_indices_, typed_relation_->GetNumColumns()));
    pac_ = model::UCCPAC{std::move(columns), eps_delta.epsilon, eps_delta.delta};
    LOG_INFO("Result: {}", pac_->ToLongString());
}

void DimUCCPACVerifier::LoadDataInternal() {
    typed_relation_ = model::ColumnLayoutTypedRelationData::CreateFrom(*input_table_, true);
    input_table_->Reset();
    if (typed_relation_->GetColumnData().empty()) {
        throw std::runtime_error("Got an empty dataset: PAC validation is meaningless.");
    }

    std::vector<model::Type const*> types(column_indices_.size());
    auto const& col_data = typed_relation_->GetColumnData();
    std::ranges::transform(column_indices_, types.begin(),
                           [&col_data](std::size_t const idx) { return &col_data[idx].GetType(); });

    std::vector<std::string> column_names;
    column_names.reserve(column_indices_.size());
    std::ranges::transform(
            column_indices_, std::back_inserter(column_names),
            [&col_data](std::size_t idx) { return col_data[idx].GetColumn()->GetName(); });
    metric_ = metric_opt_->SetTypes(types, column_names);

    tuples_ = MakeTuples(typed_relation_->GetColumnData(), column_indices_);
}

DimUCCPACVerifier::DimUCCPACVerifier() {
    DESBORDANTE_OPTION_USING;
    using namespace config;

    auto get_min_epsilon = [this]() { return min_epsilon_; };
    auto get_max_epsilon = [this]() { return max_epsilon_; };
    auto get_min_delta = []() { return 0; };
    auto get_max_delta = [this]() { return max_delta_; };

    RegisterOption(kMinEpsilonOption(&min_epsilon_).SetConditionalOpts({{nullptr, {kMaxEpsilon}}}));
    RegisterOption(MaxEpsilonOption(get_min_epsilon)(&max_epsilon_)
                           .SetConditionalOpts({{nullptr, {kMaxDelta}}}));
    RegisterOption(MaxDeltaOption(get_min_epsilon, get_max_epsilon)(&max_delta_)
                           .SetConditionalOpts({{nullptr, {kDeltaSteps}}}));
    RegisterOption(DeltaStepsOption(get_min_delta, get_max_delta)(&delta_steps_));
    RegisterOption(kDiagonalThresholdOption(&diagonal_threshold_));

    RegisterOption(kTableOpt(&input_table_).SetConditionalOpts({{nullptr, {kColumnIndices}}}));

    RegisterOption(IndicesOption{kColumnIndices, kDColumnIndices, nullptr}(
            &column_indices_, [this]() { return input_table_->GetNumberOfColumns(); }));
    RegisterOption(VectorMetricOption(&metric_opt_));

    MakeOptionsAvailable({kTableOpt.GetName(), kCustomMetric});
}
