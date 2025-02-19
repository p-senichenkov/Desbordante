#include <gtest/gtest.h>

#include "algorithms/algo_factory.h"
#include "algorithms/dd/split/split.h"
#include "all_csv_configs.h"
#include "config/names.h"

namespace tests {

using namespace algos;
using namespace config::names;

TEST(HeavyDatasetsPerformanceTests, Split_Adult60k) {
    StdParamsMap param_map{
            {kCsvConfig, kAdult60k},
            {kNumRows, 10U},
            {kNumColumns, 10U},
    };
    auto algo = CreateAndLoadAlgorithm<dd::Split>(param_map);

    algo->Execute();
    SUCCEED();
}

}  // namespace tests
