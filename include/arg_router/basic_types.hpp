// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/config.hpp"

#include <sstream>
#include <vector>

#if __cplusplus >= 202000L
#    include <span>
#else
#    define span_CONFIG_CONTRACT_LEVEL_OFF
#    include <nonstd/span.hpp>
#endif

namespace arg_router
{
/** arg_router string.
 *
 * Same as a std::string, except that it uses the config::allocator.
 */
using string = std::basic_string<char, std::char_traits<char>, config::allocator<char>>;

/** arg_router ostringstream.
 *
 * Same as a std::ostringstream, except that it uses the config::allocator.
 */
using ostringstream =
    std::basic_ostringstream<char, std::char_traits<char>, config::allocator<char>>;

/** arg_router vector.
 *
 * Same as a std::vector, except that it uses the config::allocator.
 */
template <typename T>
using vector = std::vector<T, config::allocator<T>>;

/** An alias for std::span if compiling with >= C++20 support, otherwise
 * nonstd::span_lite::span.
 */
#if __cplusplus >= 202000L
template <typename T, auto Extent = std::dynamic_extent>
using span = std::span<T, Extent>;
#else
template <typename T, auto Extent = nonstd::span_lite::dynamic_extent>
using span = nonstd::span_lite::span<T, Extent>;
#endif
}  // namespace arg_router
