#pragma once

#include <memory>

#include "core/algorithms/pac/model/idomain.h"
#include "core/config/indices/type.h"
#include "core/parser/csv_parser/csv_parser.h"

namespace memory {
void DomainPACVerifierMemory(CSVConfig const& input_table, config::IndicesType&& indices,
                             std::shared_ptr<pac::model::IDomain> domain);
void FDPACVerifierMemory(CSVConfig const& input_table, config::IndicesType&& lhs_indices,
                         config::IndicesType&& rhs_indices);
void UCCPACVerifierMemory(CSVConfig const& input_table, config::IndicesType&& indices);
}  // namespace memory
