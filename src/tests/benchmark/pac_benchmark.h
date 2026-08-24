#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
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
inline void BenchmarkIowaLowArities(BenchmarkRunner& runner, CSVConfig const& csv_config) {
    using namespace config::names;

    constexpr static auto kSuffix = "low arity";

    runner.RegisterSimpleBenchmark<algos::pac_verifier::DomainPACVerifier>(
            csv_config,
            {{kColumnIndices, config::IndicesType{2, 3, 8, 9}},
             {kDomain,
              std::shared_ptr<pac::model::IDomain>(new pac::model::Ball{
                      std::vector<std::string>{"3", "24 Минимаркет", "40", "John Doe"}, 15})}},
            kSuffix);

    runner.RegisterSimpleBenchmark<algos::pac_verifier::FDPACVerifier>(
            csv_config,
            {{kLhsIndices, config::IndicesType{2, 3}}, {kRhsIndices, config::IndicesType{8, 9}}},
            kSuffix);

    runner.RegisterSimpleBenchmark<algos::pac_verifier::UCCPACVerifier>(
            csv_config, {{kColumnIndices, config::IndicesType{2, 3, 8, 7}}}, kSuffix);
}

inline void BenchmarkIowasLowArities(BenchmarkRunner& runner) {
    using namespace tests;

    for (auto const& iowa : {
                 kIowa5k,
                 kIowa10k,
                 kIowa20k,
                 kIowa50k,
                 kIowa100k,
                 kIowa200k,
                 kIowa450k,
                 kIowa550k,
                 kIowa650k,
         }) {
        BenchmarkIowaLowArities(runner, iowa);
    }
}

// --- TODO: cut line ---

#if 0
class AttributeBenchmarks {
private:
    inline static auto const kDataset = tests::kUniformWide;
    inline static std::vector<std::size_t> const kArities = {2, 4, 6, 10, 15, 20};
    inline static std::vector<std::size_t> const kLargeArities = {30, 40, 50, 60};
    inline static std::vector<std::size_t> const kLARGEArities = {200, 250, 300};

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
        std::ranges::transform(indices, std::back_inserter(centers),
                               [](config::IndexType idx) { return std::to_string(idx); });
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

    void BenchmarkLargeDomainPAC() {
        for (auto arity : kArities) {
            BenchmarkDomainPACAttr(arity);
        }
    }

    void BenchmarkLARGEDomainPAC() {
        for (auto arity : kLARGEArities) {
            BenchmarkDomainPACAttr(arity);
        }
    }

    void BenchmarkFDPAC() {
        for (auto arity : kArities) {
            BenchmarkFDPACAttr(arity);
        }
    }

    void BenchmarkLargeFDPAC() {
        for (auto arity : kLargeArities) {
            BenchmarkFDPACAttr(arity);
        }
    }

    void BenchmarkLARGEFDPAC() {
        for (auto arity : kLARGEArities) {
            BenchmarkFDPACAttr(arity);
        }
    }

    void BenchmarkUCCPAC() {
        for (auto arity : kArities) {
            BenchmarkUCCPACAttr(arity);
        }
    }
};
#endif
}  // namespace detail

inline void PACBenchmark(BenchmarkRunner& runner, BenchmarkComparer&) {
    using namespace config::names;
    using namespace tests;

    detail::BenchmarkIowasLowArities(runner);
}
}  // namespace benchmark
