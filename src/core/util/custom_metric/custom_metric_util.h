#include <cstddef>
#include <format>
#include <memory>
#include <variant>

#include "core/config/custom_metric/custom_metric/type.h"
#include "core/config/exceptions.h"
#include "core/model/types/type.h"
#include "core/util/custom_metric/custom_metric.h"

namespace util::custom_metric_util {
template <typename... T>
class Overloaded : T... {
    using T::operator()...;
};

using TypeVariant = std::variant<model::Type const*, model::IMetrizableType const*>;

TypeVariant ConvertType(model::Type const* type, config::CustomMetricType const& metric) {
    return std::visit(
            Overloaded{
                    [type](std::shared_ptr<ICustomMetric> const&) -> TypeVariant { return type; },
                    [type](std::shared_ptr<IMetrizableCustomMetric> const&) -> TypeVariant {
                        auto metr_type = dynamic_cast<model::IMetrizableType const*>(type);
                        if (metr_type) {
                            return metr_type;
                        }
                        throw config::ConfigurationError(
                                std::format("Provided metric requires a metrizable type, but "
                                            "type {} is not metrizable",
                                            type->ToString()));
                    }},
            metric);
}

double Dist(TypeVariant const& type, config::CustomMetricType const& metric, std::byte const* a,
            std::byte const* b) {
    return std::visit(
            Overloaded{[&type, a, b](std::shared_ptr<ICustomMetric> const& metric) {
                           return metric->Dist(std::get<model::Type const*>(type), a, b);
                       },
                       [&type, a, b](std::shared_ptr<IMetrizableCustomMetric> const& metric) {
                           return metric->Dist(std::get<model::IMetrizableType const*>(type), a, b);
                       }},
            metric);
}
}  // namespace util::custom_metric_util
