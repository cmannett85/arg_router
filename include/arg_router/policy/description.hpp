// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Represents the description of a node.
 *
 * If using C++17 then use the template variable helper with the <TT>S_</TT> macro; for C++20 and
 * higher, use the constructor directly with a compile-time string literal:
 * @code
 * constexpr auto a = ar::policy::description<S_("hello")>;
 * constexpr auto b = ar::policy::description_t{"hello"_S};
 * @endcode
 * @note Descriptions must not be empty
 * @tparam S Compile-time string
 */
template <typename S>
class description_t
{
public:
    /** String type. */
    using string_type = S;

    /** Constructor.
     *
     * @param str String instance
     */
    constexpr explicit description_t([[maybe_unused]] S str = {}) noexcept {}

    /** Returns the description.
     *
     * @return Description
     */
    [[nodiscard]] constexpr static std::string_view description() noexcept { return S::get(); }

private:
    static_assert(!description().empty(), "Descriptions must not be empty");
};

/** Constant variable helper.
 *
 * @tparam S Compile-time string
 */
template <typename S>
constexpr auto description = description_t<S>{};

template <typename T>
struct is_policy<description_t<T>> : std::true_type {
};
}  // namespace arg_router::policy
