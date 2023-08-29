// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

/** @file
 *
 * Preprocessor definitions.  These should not be used directly but via the equivalent constants in
 * the config namespace.  They can be overridden by defining them before the first arg_router
 * include in end-user projects, or by setting the equivalent CMake variables in cmake/config.cmake
 */

// If changing these or adding new ones, reflect those changes in config.cmake
#ifndef AR_LONG_PREFIX
#    define AR_LONG_PREFIX "--"
#endif

#ifndef AR_SHORT_PREFIX
#    define AR_SHORT_PREFIX "-"
#endif

#ifndef AR_UTF8_TRAILING_WINDOW_SIZE
#    define AR_UTF8_TRAILING_WINDOW_SIZE 16
#endif

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

/** There's a bizarre issue that appeared in MSVC 1936 where quoted metafunctions appeared to stop
 * working due to the <TT>Q::template fn</TT> definition not being accepted.
 */
#if (!defined(__clang__) && defined(_MSC_VER) && (_MSC_VER >= 1936))
#    define MSVC_1936_WORKAROUND 1
#endif
}  // namespace arg_router::config
