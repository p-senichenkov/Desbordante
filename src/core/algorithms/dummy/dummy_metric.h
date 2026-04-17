#pragma once

#include <any>
#include <cstddef>
#include <functional>

#include "core/model/types/type.h"
#include "core/util/logger.h"

class DummyMetric {
public:
    virtual ~DummyMetric() = default;

    virtual double operator()(std::byte const* l, std::byte const* r) const = 0;

    virtual void SetType(model::Type const*) {}
};

class AnyDummyMetric : public DummyMetric {
private:
    model::Type const* type_;
    std::function<double(std::any const&, std::any const&)> metric_;

public:
    AnyDummyMetric(std::function<double(std::any const&, std::any const&)>&& metric)
        : metric_(std::move(metric)) {}

    void SetType(model::Type const* type) override {
        type_ = type;
    }

    double operator()(std::byte const*, std::byte const*) const override {
        LOG_WARN("Not implemented");
        return 0;
    }
};
