// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Represents the error name of an argument.
 *
 * An error name is a label given to a node such that when it throws an error, this label is used
 * to represent the node.  This policy is typically not for use by users, it is for node
 * developers to tune their node's representation in error output.
 *
 * @code
 * constexpr auto b = ar::policy::error_name_t{"hello"_S};
 * @endcode
 * @note Error names must not be empty
 * @tparam S Compile-time string
 */
template <typename S>
class error_name_t
{
public:
    /** String type. */
    using string_type = S;

    /** Constructor.
     *
     * @param str String instance
     */
    constexpr explicit error_name_t([[maybe_unused]] S str = {}) noexcept {}

    /** Returns the name.
     *
     * @return Display name
     */
    [[nodiscard]] constexpr static std::string_view error_name() noexcept { return S::get(); }

private:
    static_assert(!error_name().empty(), "Error name must not be empty");
};

template <typename S>
struct is_policy<error_name_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
