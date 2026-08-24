#include "core/algorithms/algo_factory.h"
#include "core/config/indices/type.h"
#include "core/config/names.h"
#include "tests/common/all_csv_configs.h"
#include "tests/memory/util.h"

#include "core/algorithms/pac/pac_verifier/domain_pac_verifier/domain_pac_verifier.h"
#include "core/algorithms/pac/model/idomain.h"
#include "core/algorithms/pac/model/default_domains/ball.h"

int main() {
   using namespace config::names;
   using namespace tests;

   algos::StdParamsMap options{
             {kCsvConfig, kIowa5k},
       {kColumnIndices, config::IndicesType{2, 3, 8, 9}},
       {kDomain,
        std::shared_ptr<pac::model::IDomain>(
           new pac::model::Ball{std::vector<std::string>{"3", "Good Goods", "40", "John Doe"}, 15}
        )
       },
   };

  MeasuredRun<algos::pac_verifier::DomainPACVerifier>(options);
}
