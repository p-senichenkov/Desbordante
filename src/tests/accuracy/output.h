#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include <boost/json/object.hpp>
#include <boost/json/src.hpp>

#include "tests/accuracy/epsilon_delta_error.h"

/*
Output format:
{
    "n": {
        "name": {
            "epsilon": {"baseline": float, "actual": float},
            "delta": {"baseline": float, "actual": float}
        },
        ...
    },
    ...
}
*/

namespace accuracy {
namespace detail {
inline boost::json::object SaveError(Error const& error) {
    return {{"baseline", error.baseline}, {"actual", error.actual}};
}

inline boost::json::object SaveError(EpsilonDeltaError const& error) {
    return {{"epsilon", SaveError(error.epsilon)}, {"delta", SaveError(error.delta)}};
}
}  // namespace detail

inline void Save(
        std::unordered_map<std::size_t, std::unordered_map<std::string, EpsilonDeltaError>> results,
        std::string_view const& filename) {
    boost::json::object top_level_object;

    for (auto const& [n, n_results] : results) {
        boost::json::object n_object;
        for (auto const& [name, result] : n_results) {
            n_object[name] = detail::SaveError(result);
        }
        top_level_object[std::to_string(n)] = std::move(n_object);
    }

    std::filesystem::path output_path{filename};
    std::ofstream output{output_path};
    output << top_level_object << '\n';
}
}  // namespace accuracy
