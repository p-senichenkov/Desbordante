#pragma once

#include <memory>
#include <utility>

#include "core/algorithms/algo_factory.h"
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

template <typename Algo>
inline void BenchmarkMediumIowas(BenchmarkRunner& runner, algos::StdParamsMap options) {
    using namespace tests;

    for (auto const& iowa : {
                 kIowa5k,
                 kIowa10k,
                 kIowa20k,
                 kIowa50k,
                 kIowa100k,
                 kIowa200k,
         }) {
        runner.RegisterSimpleBenchmark<Algo>(iowa, std::move(options));
    }
}

inline void BenchmarkIowas4attr(BenchmarkRunner& runner) {
    using namespace config::names;

    algos::StdParamsMap domain_pac_options{
            {kColumnIndices, config::IndicesType{2, 3, 8, 9}},
            {kDomain,
             std::shared_ptr<pac::model::IDomain>(new pac::model::Ball{
                     std::vector<std::string>{"3", "24 Минимаркет", "40", "John Doe"}, 15})}};
    algos::StdParamsMap fd_pac_options{{kLhsIndices, config::IndicesType{2, 3}},
                                       {kRhsIndices, config::IndicesType{8, 9}}};
    algos::StdParamsMap ucc_pac_options{{kColumnIndices, config::IndicesType{2, 3, 8, 7}}};

#if 0
    BenchmarkMediumIowas<algos::pac_verifier::DomainPACVerifier>(runner, domain_pac_options);
#endif

#if 0
    BenchmarkMediumIowas<algos::pac_verifier::FDPACVerifier>(runner, fd_pac_options);
#endif

#if 1
    BenchmarkMediumIowas<algos::pac_verifier::UCCPACVerifier>(runner, ucc_pac_options);
#endif
}
}  // namespace detail

inline void PACBenchmark(BenchmarkRunner& runner, BenchmarkComparer&) {
    using namespace config::names;
    using namespace tests;

    detail::BenchmarkIowas4attr(runner);
}
}  // namespace benchmark
