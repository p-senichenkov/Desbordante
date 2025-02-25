#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "algorithms/algo_factory.h"
#include "algorithms/md/decision_boundary.h"
#include "algorithms/md/hymd/hymd.h"
#include "algorithms/md/hymd/preprocessing/column_matches/levenshtein.h"
#include "all_csv_configs.h"
#include "config/names.h"
#include "config/thread_number/type.h"

namespace tests {

using namespace algos;
using namespace hymd;
using namespace config::names;

constexpr static model::md::DecisionBoundary kMinSimilarity = 0.7;

TEST(HeavyDatasetsPerformanceTests, HyMD_CIPublicHighway5attr20k) {
    config::InputTable table = std::make_unique<CSVParser>(kCIPublicHighway5attr20k);

    HyMD::ColumnMatches column_matches_option;
    std::size_t const number_of_columns = table->GetNumberOfColumns();
    column_matches_option.reserve(number_of_columns);
    for (size_t i = 0; i != number_of_columns; ++i) {
        std::string const column_name = table->GetColumnName(i);
        column_matches_option.push_back(
                std::make_shared<preprocessing::column_matches::Levenshtein>(i, i, kMinSimilarity));
    }

    StdParamsMap param_map{
            {kLeftTable, table},
            {kThreads, static_cast<config::ThreadNumType>(1)},
            {kColumnMatches, column_matches_option},
    };
    auto algo = CreateAndLoadAlgorithm<HyMD>(param_map);

    algo->Execute();
    SUCCEED();
}

}  // namespace tests
