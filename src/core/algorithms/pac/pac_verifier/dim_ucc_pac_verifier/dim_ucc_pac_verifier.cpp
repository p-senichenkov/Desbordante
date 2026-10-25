#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/dim_ucc_pac_verifier.h"

#include <algorithm>

#include "core/algorithms/pac/pac_verifier/options.h"
#include "core/algorithms/pac/pac_verifier/util/make_tuples.h"
#include "core/config/custom_metric/custom_vector_metric/option.h"
#include "core/config/descriptions.h"
#include "core/config/indices/option.h"
#include "core/config/names.h"
#include "core/config/option_using.h"
#include "core/config/tabular_data/input_table/option.h"

using namespace algos::pac_verifier;

void DimUCCPACVerifier::PreparePACTypeData() {
    std::vector<model::Type const*> types(column_indices_.size());
    auto const& col_data = TypedRelation().GetColumnData();
    std::ranges::transform(column_indices_, types.begin(),
                           [&col_data](std::size_t const idx) { return &col_data[idx].GetType(); });

    std::vector<std::string> column_names;
    column_names.reserve(column_indices_.size());
    std::ranges::transform(
            column_indices_, std::back_inserter(column_names),
            [&col_data](std::size_t idx) { return col_data[idx].GetColumn()->GetName(); });
    metric_ = metric_opt_->SetTypes(types, column_names);

    tuple_type_ = std::make_shared<pac::model::TupleType>(std::move(types));

    tuples_ = pac::util::MakeTuples(TypedRelation().GetColumnData(), column_indices_);
}

DimUCCPACVerifier::DimUCCPACVerifier() {
    DESBORDANTE_OPTION_USING;
    using namespace config;

    auto get_min_epsilon = [this]() { return min_epsilon_; };
    auto get_max_epsilon = [this]() { return max_epsilon_; };
    auto get_min_delta = [this]() { return min_delta_; };
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
