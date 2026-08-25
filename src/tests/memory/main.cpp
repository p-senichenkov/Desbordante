#include "core/algorithms/algo_factory.h"
#include "core/config/indices/type.h"
#include "core/config/names.h"
#include "tests/common/all_csv_configs.h"
#include "tests/memory/util.h"

#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_verifier.h"

int main() {
   using namespace config::names;
   using namespace tests;

   algos::StdParamsMap options{
             {kCsvConfig, kIowa20k},
{kLhsIndices, config::IndicesType{2, 3}},
{kRhsIndices, config::IndicesType{8, 9}},

   };

  MeasuredRun<algos::pac_verifier::FDPACVerifier>(options);
}
