#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <sstream>
#include <utility>

#include "core/algorithms/algo_factory.h"
#include "core/algorithms/pac/domain_pac.h"
#include "core/algorithms/pac/model/default_domains/ball.h"
#include "core/algorithms/pac/model/idomain.h"
#include "core/algorithms/pac/pac_verifier/domain_pac_verifier/domain_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/ucc_pac_verifier/ucc_pac_verifier.h"
#include "core/config/column_index/type.h"
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
#if 0
                 kIowa5k,
                 kIowa10k,
                 kIowa20k,
#endif
#if 1
                 kIowa50k,
#endif
#if 0
                 kIowa100k,
                 kIowa200k,
#endif
         }) {
        // runner.RegisterSimpleBenchmark<Algo>(iowa, std::move(options));
        runner.RegisterPACHighlightBenchmark<Algo>(iowa, std::move(options));
    }
}

template <typename Algo>
inline void BenchmarkLargeIowas(BenchmarkRunner& runner, algos::StdParamsMap options) {
    using namespace tests;

#if 0
    for (auto const& iowa : {
		    kIowa450k,
		    kIowa550k,
		    kIowa650k,
         }) {
        runner.RegisterSimpleBenchmark<Algo>(iowa, std::move(options));
    }
#endif
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
#if 0
    BenchmarkLargeIowas<algos::pac_verifier::DomainPACVerifier>(runner, domain_pac_options);
#endif
#endif

#if 0
    BenchmarkMediumIowas<algos::pac_verifier::FDPACVerifier>(runner, fd_pac_options);
#endif

#if 1
    BenchmarkMediumIowas<algos::pac_verifier::UCCPACVerifier>(runner, ucc_pac_options);
#endif
}

class AttributeBenchmarks {
private:
    inline static auto const kDataset = tests::kMushroom50k;
    inline static std::vector<std::size_t> const kArities = {2, 4, 6, 10, 15, 20};
    BenchmarkRunner& runner_;

    static config::IndicesType Range(std::size_t start, std::size_t end) {
        config::IndicesType result;
        result.reserve(end - start);
        for (; start < end; ++start) {
            result.push_back(start);
        }
        return result;
    }

    void BenchmarkDomainPACAttr(std::size_t attr_num) {
        using namespace config::names;

        auto indices = Range(0, attr_num);
        std::vector<std::string> centers;
        centers.reserve(attr_num);
        std::ranges::transform(indices, std::back_inserter(centers), [](config::IndexType idx) {
            return std::string{static_cast<char>('a' + idx)};
        });
        std::ostringstream name_suffix;
        name_suffix << attr_num << " attributes";
        runner_.RegisterSimpleBenchmark<algos::pac_verifier::DomainPACVerifier>(
                kDataset,
                {{kColumnIndices, std::move(indices)},
                 {kDomain, std::shared_ptr<pac::model::IDomain>(
                                   new pac::model::Ball(std::move(centers), 15))}},
                name_suffix.str());
    }

    void BenchmarkFDPACAttr(std::size_t attr_num) {
        using namespace config::names;

        auto lhs_indices = Range(0, (attr_num + 1) / 2);
        auto rhs_indices = Range((attr_num + 1) / 2, attr_num);
        std::ostringstream name_suffix;
        name_suffix << attr_num << " attributes";
        runner_.RegisterSimpleBenchmark<algos::pac_verifier::FDPACVerifier>(
                kDataset,
                {{kLhsIndices, std::move(lhs_indices)},
                 {config::names::kRhsIndices, std::move(rhs_indices)}},
                name_suffix.str());
    }

    void BenchmarkUCCPACAttr(std::size_t attr_num) {
        using namespace config::names;

        std::ostringstream name_suffix;
        name_suffix << attr_num << " attributes";
        runner_.RegisterSimpleBenchmark<algos::pac_verifier::UCCPACVerifier>(
                kDataset, {{kColumnIndices, Range(0, attr_num)}}, name_suffix.str());
    }

public:
    explicit AttributeBenchmarks(BenchmarkRunner& runner) : runner_(runner) {}

    void BenchmarkDomainPAC() {
        for (auto arity : kArities) {
            BenchmarkDomainPACAttr(arity);
        }
    }

    void BenchmarkFDPAC() {
        for (auto arity : kArities) {
            BenchmarkFDPACAttr(arity);
        }
    }

    void BenchmarkUCCPAC() {
        for (auto arity : kArities) {
            BenchmarkUCCPACAttr(arity);
        }
    }
};
}  // namespace detail

inline void PACBenchmark(BenchmarkRunner& runner, BenchmarkComparer&) {
    using namespace config::names;
    using namespace tests;

#if 0
    detail::BenchmarkIowas4attr(runner);
#endif

#if 1
    detail::AttributeBenchmarks attr_bench{runner};
#if 1
    attr_bench.BenchmarkDomainPAC();
#endif
#if 0
    attr_bench.BenchmarkFDPAC();
#endif
#if 0
    attr_bench.BenchmarkUCCPAC();
#endif
#endif

    // runner.RegisterSimpleBenchmark<algos::pac_verifier::DomainPACVerifier>(
    //         kMushroom50k, {{kColumnIndices, config::IndicesType{0, 1, 2, 3, 4, 5, 6}},
    //                        {kDomain, std::shared_ptr<pac::model::IDomain>{new pac::model::Ball(
    //                                          std::vector<std::string>(7, "a"), 15)}}});
}
}  // namespace benchmark
