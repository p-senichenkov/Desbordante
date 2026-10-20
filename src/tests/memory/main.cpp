#include "core/algorithms/algo_factory.h"
#include "core/config/indices/type.h"
#include "core/config/names.h"
#include "tests/common/all_csv_configs.h"
#include "tests/memory/algorithms.h"
#include "tests/memory/util.h"

int main() {
    using namespace config::names;
    using namespace tests;
    using namespace memory;

    algos::StdParamsMap options{
            {kCsvConfig, kIowa20k},
            {kLhsIndices, config::IndicesType{2, 3}},
            {kRhsIndices, config::IndicesType{8, 9}},

    };

    MeasuredRun<FDPACVerifier>(options);
}
