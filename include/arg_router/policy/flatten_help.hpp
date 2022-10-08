// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Used to set a help node to display all command line options in one large tree.
 *
 * The default behaviour for a help node is to only show the command line options for the specified
 * level, this instructs to displsy all the options.
 */
template <typename = void>  // This is needed due so it can be used in
struct flatten_help_t {     // template template parameters
};

/** Constant variable helper. */
constexpr auto flatten_help = flatten_help_t<>{};

template <>
struct is_policy<flatten_help_t<>> : std::true_type {
};
}  // namespace arg_router::policy
