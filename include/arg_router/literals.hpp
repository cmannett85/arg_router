// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/utility/compile_time_string.hpp"

namespace arg_router::literals
{
/** String literal to generate a compile-time string instance.
 *
 * @tparam S Internal storage type
 * @return Compile-time string instance
 */
template <utility::detail::compile_time_string_storage S>
[[nodiscard]] constexpr utility::str<S> operator""_S() noexcept
{
    return utility::str<S>{};
}
}  // namespace arg_router::literals
