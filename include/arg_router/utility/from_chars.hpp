// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/traits.hpp"
#include "arg_router/utility/utf8.hpp"

#include <charconv>

namespace arg_router::utility
{
/** Convenience wrapper for std::from_chars.
 *
 * @tparam T Arithmetic type
 * @param str String to convert
 * @return Converted arithmetic type or an empty optional if @a str could not be converted
 */
template <concepts::is_arithmetic T>
[[nodiscard]] std::optional<T> from_chars(std::string_view str) noexcept
{
    str = utf8::strip(str);
    if (str.empty()) {
        return {};
    }

    // std::from_chars doesn't support a leading + sign
    if (str.front() == '+') {
        str.remove_prefix(1);
    }
    if (str.empty()) {
        return {};
    }

    // std::from_chars doesn't support leading hex notation
    auto is_hex = false;
    if (str.substr(0, 2) == "0x" || str.substr(0, 2) == "0X") {
        str.remove_prefix(2);
        is_hex = true;
    }
    if (str.empty()) {
        return {};
    }

    auto result = T{};
    auto ec = std::errc{};
    if constexpr (std::is_integral_v<T>) {
        ec = std::from_chars(str.data(), str.data() + str.size(), result, is_hex ? 16 : 10).ec;
    } else {
        ec = std::from_chars(str.data(),
                             str.data() + str.size(),
                             result,
                             is_hex ? std::chars_format::hex : std::chars_format::general)
                 .ec;
    }

    if (ec != std::errc{}) {
        return {};
    }
    return result;
}
}  // namespace arg_router::utility
