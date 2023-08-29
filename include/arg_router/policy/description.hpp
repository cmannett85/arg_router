// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Represents the description of a node.
 *
 * @code
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

template <typename T>
struct is_policy<description_t<T>> : std::true_type {
};
}  // namespace arg_router::policy
