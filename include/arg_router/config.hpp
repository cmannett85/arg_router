// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/utility/utf8.hpp"

/** Build configuration-defined constants.
 *
 * There are a few core parts of arg_router that are configurable as compile defines, all are
 * available as CMake cache variables too (minus the "AR_" prefix).
 */
namespace arg_router::config
{
/** Long form argument prefix.
 *
 * UTF-8 supporting.  Must be the same or longer than short_prefix (in terms of characters).
 */
constexpr auto long_prefix = std::string_view{AR_LONG_PREFIX};

/** Short form argument prefix.
 *
 * UTF-8 supporting.  Must be one characters long.
 */
constexpr auto short_prefix = std::string_view{AR_SHORT_PREFIX};

static_assert(utility::utf8::count(short_prefix) == 1, "Short prefix must be one character");

static_assert(utility::utf8::count(long_prefix) >= utility::utf8::count(short_prefix),
              "Long prefix must be longer or the same as short prefix");

/** Allocator for all STL types.
 *
 * @tparam T Type to allocate for
 */
template <typename T>
using allocator = AR_ALLOCATOR<T>;

#if (__cplusplus >= 202002L) && !AR_DISABLE_CPP20_STRINGS
#    define AR_ENABLE_CPP20_STRINGS
#endif
}  // namespace arg_router::config
