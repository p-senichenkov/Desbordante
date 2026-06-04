#pragma once

#include <memory>

#include "core/algorithms/pac/model/default_domains/ball.h"
#include "core/algorithms/pac/model/idomain.h"
#include "core/algorithms/pac/pac_verifier/domain_pac_verifier/domain_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/ucc_pac_verifier/ucc_pac_verifier.h"
#include "core/config/indices/type.h"
#include "core/config/names.h"
#include "tests/benchmark/benchmark_comparer.h"
#include "tests/benchmark/benchmark_runner.h"
#include "tests/common/all_csv_configs.h"

namespace benchmark {
inline void PACBenchmark(BenchmarkRunner& runner, BenchmarkComparer&) {
    using namespace config::names;

    runner.RegisterSimpleBenchmark<algos::pac_verifier::DomainPACVerifier>(
            tests::kIowa650k,
            {
                    {kColumnIndices, config::IndicesType{2, 3, 8, 9}},
                    {kDomain,
                     std::shared_ptr<pac::model::IDomain>(new pac::model::Ball{
                             std::vector<std::string>{"3", "24 Минимаркет", "40", "John Doe"},
                             15})},
            });
    runner.RegisterSimpleBenchmark<algos::pac_verifier::FDPACVerifier>(
            tests::kIowa650k, {
                                      {kLhsIndices, config::IndicesType{2, 3}},
                                      {kRhsIndices, config::IndicesType{8, 9}},
                              });
    runner.RegisterSimpleBenchmark<algos::pac_verifier::UCCPACVerifier>(
            tests::kIowa650k, {{kColumnIndices, config::IndicesType{2, 3, 8, 8}}});
}
}  // namespace benchmark
