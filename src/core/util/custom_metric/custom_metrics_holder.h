#pragma once

#include <cstddef>
#include <variant>
#include <vector>

#include "core/config/custom_metric/custom_metrics/type.h"
#include "core/config/indices/type.h"
#include "core/model/table/column_layout_typed_relation_data.h"
#include "core/model/types/imetrizable_type.h"
#include "core/model/types/type.h"

namespace util {
/// @brief CustomMetricsHolder is aimed to save a lot of `dynamic_cast`s for `IMetrizableType`-based
/// metrics
/// You should always prefer CustomMetricsHolder over plain
/// `std::vector<std::shared_ptr<ICustomMetric>>` until you have custom type handling
class CustomMetricsHolder {
private:
    std::vector<std::variant<model::Type const*, model::IMetrizableType const*>> types_;
    config::CustomMetricsType metrics_;

public:
    // This method is intended to be used only in Option constructor
    config::CustomMetricsType* ValuePtr() {
        return &metrics_;
    }

    void SetTypes(model::TypedRelationData const& typed_relation_data,
                  config::IndicesType const& column_indices);

    double Dist(std::byte const* a, std::byte const* b, std::size_t idx) const;
};
}  // namespace util
