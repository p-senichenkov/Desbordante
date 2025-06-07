#pragma once

#include "algorithms/dd/split/split.h"
#include "all_csv_configs.h"
#include "benchmark_controller.h"

namespace benchmark {

inline void DDBenchmark(BenchmarkController& controller) {
    controller.RegisterSimpleTest<algos::dd::Split>(tests::kAdult9attr);
}

}  // namespace benchmark
