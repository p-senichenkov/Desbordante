#include <gtest/gtest.h>

#include "all_csv_configs.h"
#include "algorithms/algo_factory.h"
#include "algorithms/dc/FastADC/fastadc.h"
#include "config/names.h"

namespace tests {

using namespace algos;
using namespace config::names;

TEST(HeavyDatasetsPerformanceTests, FastADC_Neighbors100k) {
    StdParamsMap param_map{
        {kCsvConfig, kNeighbors100k},
    };
    auto algo = CreateAndLoadAlgorithm<dc::FastADC>(param_map);

    algo->Execute();
    SUCCEED();
}

}  // namespace tests
