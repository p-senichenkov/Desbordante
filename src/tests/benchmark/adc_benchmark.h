#pragma once

#include "algorithms/dc/FastADC/fastadc.h"
#include "all_csv_configs.h"
#include "benchmark_controller.h"

namespace benchmark {

inline void ADCBenchmark(BenchmarkController& controller) {
    controller.RegisterSimpleTest<algos::dc::FastADC>(tests::kNeighbors120k);
}

}  // namespace benchmark
