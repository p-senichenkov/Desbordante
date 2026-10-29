#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>

#include "tests/accuracy/epsilon_delta_error.h"

namespace accuracy {
using AccuracyTest = std::function<EpsilonDeltaError(std::size_t)>;

class AccuracyTestRunner {
private:
    std::unordered_map<std::string, AccuracyTest> tests_;

    using Results = std::unordered_map<std::string, EpsilonDeltaError>;
    using AllResults =
            std::unordered_map<std::size_t, std::unordered_map<std::string, EpsilonDeltaError>>;

public:
    void Register(std::string const& name, AccuracyTest test) {
        tests_[name] = test;
    }

    // N is arbitrary numeric characteristic of the test, displayed on axis X
    Results Run(std::size_t n) const {
        Results results;
        for (auto const& [name, test] : tests_) {
            results[name] = test(n);
        }
        return results;
    }

    AllResults RunAll(std::vector<std::size_t> const& numerics) const {
        AllResults results;
        for (auto const n : numerics) {
            results[n] = Run(n);
        }
        return results;
    }
};
}  // namespace accuracy
