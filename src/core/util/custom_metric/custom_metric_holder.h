#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <variant>
#include <vector>

#include "core/config/exceptions.h"
#include "core/config/indices/type.h"
#include "core/model/table/column_layout_typed_relation_data.h"
#include "core/model/types/imetrizable_type.h"
#include "core/model/types/type.h"
#include "core/util/custom_metric/custom_metric.h"

namespace util {
/// @brief CustomMetricHolder is aimed to save a lot of `dynamic_cast`s for `IMetrizableType`-based
/// metrics
/// You should always prefer CustomMetricHolder over plain `std::shared_ptr<ICustomMetric>` until
/// you have custom type handling
class CustomMetricHolder {
private:
    friend void InitializeCustomMetricHolders(std::vector<CustomMetricHolder>& holders,
                                              model::TypedRelationData const& typed_relation_data,
                                              config::IndicesType const& column_indices);

    std::variant<model::Type const*, model::IMetrizableType const*> type_;
    std::shared_ptr<ICustomMetric> metric_;

public:
    // This method is intended to be used only in Option constructor
    std::shared_ptr<ICustomMetric>* ValuePtr() {
        return &metric_;
    }

    void SetType(model::Type const* type) {
        auto metr_type = dynamic_cast<model::IMetrizableType const*>(type);
        if (metr_type) {
            type_ = metr_type;
        } else {
            if (metric_->RequiresMetrizableType()) {
                auto type_name = type->ToString();
                throw config::ConfigurationError(std::format(
                        "Provided metric requires a metrizable type, but type {} is not metrizable",
                        type->ToString()));
            }
            type_ = type;
        }
    }

    double Dist(std::byte const* a, std::byte const* b) const {
        return std::visit([a, b, this](auto const* type) { return metric_->Dist(type, a, b); },
                          type_);
    }
};

/// @brief Initialize a set of `CustomMetricHolder`s, emitting more clear error messages than direct
/// calls to `holder.SetType()`
void InitializeCustomMetricHolders(std::vector<CustomMetricHolder>& holders,
                                   model::TypedRelationData const& typed_relation_data,
                                   config::IndicesType const& column_indices) {
    assert(holders.size() == column_indices.size());

    for (std::size_t i = 0; i < holders.size(); ++i) {
        auto& holder = holders[i];
        auto const& column_data = typed_relation_data.GetColumnData(column_indices[i]);
        auto const* type = &column_data.GetType();
        auto const* metrizable_type = dynamic_cast<model::IMetrizableType const*>(type);
        if (metrizable_type) {
            holder.type_ = type;
        } else {
            if (holder.metric_->RequiresMetrizableType()) {
                throw config::ConfigurationError(std::format(
                        "Metric for column {} requires a metrizable type, but type {} is not "
                        "metrizable",
                        column_data.GetColumn()->GetName(),
                        type->ToString());
            }
            holder.type_ = type;
        }
    }
}
}  // namespace util
