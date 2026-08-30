#include "core/util/custom_metric/custom_metrics_holder.h"

#include <variant>

#include "core/util/custom_metric/custom_metric_util.h"

using namespace util;

namespace {
template <typename... T>
class Overloaded : T... {
    using T::operator()...;
};

using TypeVariant = std::variant<model::Type const*, model::IMetrizableType const*>;
}  // namespace

void CustomMetricsHolder::SetTypes(model::TypedRelationData const& typed_relation_data,
                                   config::IndicesType const& column_indices) {
    assert(metrics_.size() == column_indices.size());
    assert(((void)"SetTypes cannot be called twice", types_.empty()));

    types_.reserve(metrics_.size());
    for (std::size_t i = 0; i < column_indices.size(); ++i) {
        auto const& column_data = typed_relation_data.GetColumnData(column_indices[i]);
        auto const* type = &column_data.GetType();

        types_.push_back(custom_metric_util::ConvertType(type, metrics_[i]));
    }
}

double CustomMetricsHolder::Dist(std::byte const* a, std::byte const* b, std::size_t idx) const {
    auto has_value = [](auto const* type) { return type != nullptr; };
    assert(((void)"Custom metric holder must be initialized first",
            std::visit([](auto const* p) { return p != nullptr; }, types_[idx])));
    assert((void)"Metric cannot be nullptr, do you use CustomMetricsOption?"),
            std::visit([](auto const& p) { return p != nullptr; }, metrics_[idx]);

    return custom_metric_util::Dist(types_[idx], metrics_[idx], a, b);
}
