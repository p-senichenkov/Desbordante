#pragma once

#include <memory>

#include "core/algorithms/pac/model/default_domains/ball.h"
#include "core/algorithms/pac/model/idomain.h"
#include "core/algorithms/pac/pac_verifier/domain_pac_verifier/domain_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/ucc_pac_verifier/ucc_pac_verifier.h"
#include "core/config/indices/type.h"
#include "core/config/names.h"
#include "core/parser/csv_parser/csv_parser.h"
#include "tests/benchmark/benchmark_comparer.h"
#include "tests/benchmark/benchmark_runner.h"
#include "tests/common/all_csv_configs.h"
#include "tests/common/csv_config_util.h"

namespace benchmark {
namespace detail {
inline void FDPACBenchmark(BenchmarkRunner& runner, CSVConfig const& csv_config) {
    using namespace config::names;

    runner.RegisterSimpleBenchmark<algos::pac_verifier::FDPACVerifier>(
            csv_config, {
                                {kLhsIndices, config::IndicesType{2, 3}},
                                {kRhsIndices, config::IndicesType{8, 9}},
                        });
}
}  // namespace detail

inline void PACBenchmark(BenchmarkRunner& runner, BenchmarkComparer&) {
    using namespace config::names;
    using namespace tests;

#if 0
    runner.RegisterSimpleBenchmark<algos::pac_verifier::DomainPACVerifier>(
            tests::kIowa650k,
            {
                    {kColumnIndices, config::IndicesType{2, 3, 8, 9}},
                    {kDomain,
                     std::shared_ptr<pac::model::IDomain>(new pac::model::Ball{
                             std::vector<std::string>{"3", "24 Минимаркет", "40", "John Doe"},
                             15})},
            });
#endif

#if 1
    detail::FDPACBenchmark(runner, kIowa5k);
#endif

#if 0
    detail::FDPACBenchmark(runner, tests::kIowa10k);
#endif
#if 0
    detail::FDPACBenchmark(runner, tests::kIowa20k);
#endif
#if 0
    detail::FDPACBenchmark(runner, tests::kIowa50k);
#endif
#if 0
    detail::FDPACBenchmark(runner, tests::kIowa100k);
#endif
#if 0
    detail::FDPACBenchmark(runner, tests::kIowa200k);
#endif
    // runner.RegisterSimpleBenchmark<algos::pac_verifier::UCCPACVerifier>(
    //         tests::kIowa650k, {{kColumnIndices, config::IndicesType{2, 3, 8, 7}}});
}
}  // namespace benchmark
