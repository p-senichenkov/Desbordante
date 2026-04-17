#pragma once

#include <pybind11/pybind11.h>

#include "core/algorithms/dummy/dummy_metric.h"
#include "core/util/logger.h"

namespace python_bindigns {
pybind11::handle GetPyValue(std::byte const*, model::Type const*) {
    LOG_WARN("Not implemented");
    Py_RETURN_NONE;
}

class PyDummyMetric : public DummyMetric {
private:
    model::Type const* type_;
    pybind11::object metric_;

public:
    PyDummyMetric(pybind11::object&& metric) : metric_(std::move(metric)) {}

    void SetType(model::Type const* type) override {
        type_ = type;
    }

    double operator()(std::byte const* l, std::byte const* r) const override {
        return pybind11::cast<double>(metric_(GetPyValue(l, type_), GetPyValue(r, type_)));
    }
};
}  // namespace python_bindigns
