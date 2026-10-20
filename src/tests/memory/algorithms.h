#pragma once

#include "core/algorithms/pac/pac_verifier/domain_pac_verifier/domain_pac_verifier.h"
#include "core/algorithms/pac/pac_verifier/fd_pac_verifier/fd_pac_verifier.h"

namespace memory {
using DomainPACVerifier = algos::pac_verifier::DomainPACVerifier;
using FDPACVerifier = algos::pac_verifier::FDPACVerifier;

#if 1
#include "core/algorithms/pac/pac_verifier/dim_ucc_pac_verifier/histogram_ucc_pac_verifier.h"
using UCCPACVerifier = algos::pac_verifier::HistogramUCCPACVerifier;
#else
#include "core/algorithms/pac/pac_verifier/ucc_pac_verifier/ucc_pac_verifier.h"
using UCCPACVerifier = algos::pac_verifier::UCCPACVerifier;
#endif
}  // namespace memory
