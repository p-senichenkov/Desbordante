#include "core/util/custom_metric/custom_metric_holder.h"

#include <cassert>
#include <cstddef>

#include "core/util/custom_metric/custom_metric_util.h"

using namespace util;

void CustomMetricHolder::SetType(model::Type const* type) {
    type_ = custom_metric_util::ConvertType(type, metric_);
}

double CustomMetricHolder::Dist(std::byte const* a, std::byte const* b) const {
    assert(((void)"Custom metric holder must be initialized first",
            std::visit([](auto const* p) { return p != nullptr; }, type_)));
    assert(((void)"Metric cannot be nullptr, do you use CustomMetricOption?",
            std::visit([](auto const& p) { return p != nullptr; }, metric_)));

    return custom_metric_util::Dist(type_, metric_, a, b);
}
