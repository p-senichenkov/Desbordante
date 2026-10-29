#pragma once

namespace accuracy {
namespace detail {
struct Error {
    double baseline;
    double actual;
};
}  // namespace detail

struct EpsilonDeltaError {
    detail::Error epsilon;
    detail::Error delta;
};
}  // namespace accuracy
