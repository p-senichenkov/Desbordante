#pragma once

#include <memory>

#include "core/algorithms/algorithm.h"
#include "core/algorithms/dummy/dummy_metric.h"

namespace algos::dummy {
class DummyAlgorithm final : public Algorithm {
private:
    std::shared_ptr<DummyMetric> metric_;

public:
    DummyAlgorithm() : Algorithm() {
        RegisterOption(
                config::Option{&metric_, "dummy_metric", "Dummy metric for Dummy algorithm"});

        MakeOptionsAvailable({"dummy_metric"});
    }

    void ResetState() override {};

    void LoadDataInternal() override {};

    unsigned long long ExecuteInternal() override {
        return 0;
    };
};
}  // namespace algos::dummy
