// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>

/** Namespace for terminal utilities. */
namespace arg_router::utility::terminal
{
#ifdef UNIT_TEST_BUILD
[[nodiscard]] std::size_t& test_columns_value() noexcept;
#endif

/** Returns the current number columns in the terminal.
 *
 * @return Column count
 */
[[nodiscard]] std::size_t columns() noexcept;
}  // namespace arg_router::utility::terminal
