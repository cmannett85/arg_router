/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include <cstddef>
#include <memory>
#include <string_view>

/** Build configuration-defined constants.
 *
 * There are a few core parts of arg_router that are configurable as compile defines, all are
 * available as CMake cache variables too (minus the "AR_" prefix).
 */
namespace arg_router::config
{
/** Long form argument prefix. */
constexpr auto long_prefix = std::string_view{AR_LONG_PREFIX};

/** Short form argument prefix. */
constexpr auto short_prefix = std::string_view{AR_SHORT_PREFIX};

static_assert(short_prefix.size() == 1, "Short prefix must be one character");

static_assert(long_prefix.size() >= short_prefix.size(),
              "Long prefix must be longer or the same as short prefix");

static_assert(short_prefix != long_prefix, "Short and long prefixes cannot be the same");

/** Allocator for all STL types.
 *
 * @tparam T Type to allocate for
 */
template <typename T>
using allocator = AR_ALLOCATOR<T>;
}  // namespace arg_router::config
