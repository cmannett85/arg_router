// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Indicates that a node or derived policy's owning node does not create a return value when
 * parsed.
 *
 * Policies or nodes can derive from this.
 */
template <typename = void>  // This is needed due so it can be used in
struct no_result_value {    // template template parameters
};

/** Evaluates to true if @a T uses no_result_value.
 *
 * @tparam T Type to test
 */
template <typename T>
using has_no_result_value = std::is_base_of<no_result_value<>, T>;

/** Helper variable for has_no_result_value.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool has_no_result_value_v = has_no_result_value<T>::value;

template <>
struct is_policy<no_result_value<>> : std::true_type {
};
}  // namespace arg_router::policy
