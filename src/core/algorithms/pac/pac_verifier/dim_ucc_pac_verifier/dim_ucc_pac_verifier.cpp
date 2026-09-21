#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/dim_ucc_pac_verifier.h"

#include "core/algorithms/pac/pac_verifier/util/make_tuples.h"
#include "core/config/custom_metric/custom_vector_metric/option.h"
#include "core/config/descriptions.h"
#include "core/config/indices/option.h"
#include "core/config/names.h"
#include "core/config/option_using.h"
#include "core/config/tabular_data/input_table/option.h"

using namespace algos::pac_verifier;

double DimUCCPACVerifier::GetNumPairs(double delta) const {
    // delta = (2 * pairs + total_tuples) / total_tuples^2
    // => pairs = (delta * total_tuples^2 - total_tuples) / 2 =
    //          = total_tuples * (delta * total_tuples - 1) / 2
    double num_pairs = static_cast<double>(tuples_->size() * (delta * tuples_->size() - 1)) / 2;
    if (num_pairs < 0) {
        num_pairs = 0;
    }
    return num_pairs;
}

double DimUCCPACVerifier::GetDelta(std::size_t num_pairs) const {
    // See "key ideas"
    double delta =
            static_cast<double>(2 * num_pairs + tuples_->size()) / std::pow(tuples_->size(), 2);
    assert(delta >= -PACVerifier::kDistThreshold && delta <= 1 + PACVerifier::kDistThreshold);
    return delta;
}

void DimUCCPACVerifier::PreparePACTypeData() {
    std::vector<model::Type const*> types(column_indices_.size());
    auto const& col_data = TypedRelation().GetColumnData();
    std::ranges::transform(column_indices_, types.begin(),
                           [&col_data](std::size_t const idx) { return &col_data[idx].GetType(); });

    tuple_type_ = std::make_shared<pac::model::TupleType>(std::move(types));

    tuples_ = pac::util::MakeTuples(TypedRelation().GetColumnData(), column_indices_);
}

DimUCCPACVerifier::DimUCCPACVerifier() {
    DESBORDANTE_OPTION_USING;
    using namespace config;

    RegisterCommonOptions(false, true);

    RegisterOption(kTableOpt(&input_table_).SetConditionalOpts({{nullptr, {kColumnIndices}}}));

    RegisterOption(IndicesOption{kColumnIndices, kDColumnIndices, nullptr}(
            &column_indices_, [this]() { return input_table_->GetNumberOfColumns(); }));
    RegisterOption(VectorMetricOption(&metric_));

    MakeOptionsAvailable({kTableOpt.GetName(), kCustomMetric});
}
