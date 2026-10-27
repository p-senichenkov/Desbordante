#include "tests/memory/algorithms.h"

#include <memory>
#include <utility>

#include "core/algorithms/algo_factory.h"
#include "core/algorithms/pac/model/idomain.h"
#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/histogram_ucc_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/domain_pac_verifier/domain_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/ucc_pac_verifier/ucc_pac_verifier.h"
#include "core/config/indices/type.h"
#include "core/config/names.h"
#include "core/parser/csv_parser/csv_parser.h"
#include "tests/memory/util.h"

using namespace config::names;

namespace memory {
void DomainPACVerifierMemory(CSVConfig const& input_table, config::IndicesType&& indices,
                             std::shared_ptr<pac::model::IDomain> domain) {
    algos::StdParamsMap options{{kCsvConfig, input_table},
                                {kDomain, std::move(domain)},
                                {kColumnIndices, std::move(indices)}};
    MeasuredRun<algos::pac_verifier::DomainPACVerifier>(options);
}

void FDPACVerifierMemory(CSVConfig const& input_table, config::IndicesType&& lhs_indices,
                         config::IndicesType&& rhs_indices) {
    algos::StdParamsMap options{{kCsvConfig, input_table},
                                {kLhsIndices, std::move(lhs_indices)},
                                {kRhsIndices, std::move(rhs_indices)}};
    MeasuredRun<algos::pac_verifier::FDPACVerifier>(options);
}

void UCCPACVerifierMemory(CSVConfig const& input_table, config::IndicesType&& indices) {
    algos::StdParamsMap options{{kCsvConfig, input_table}, {kColumnIndices, std::move(indices)}};
#if 0
    MeasuredRun<algos::pac_verifier::UCCPACVerifier>(options);
#else
    MeasuredRun<algos::pac_verifier::HistogramUCCPACVerifier>(options);
#endif
}
}  // namespace memory
