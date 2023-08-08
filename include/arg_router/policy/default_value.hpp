// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/config.hpp"
#include "arg_router/policy/policy.hpp"

namespace arg_router::policy
{
/** Provides a default value for non-required arguments.
 *
 * @tparam T Value type
 */
template <typename T>
class default_value
{
public:
    /** Alias of @a T. */
    using value_type = T;

    /** Policy priority. */
    constexpr static auto priority = std::size_t{500};

    /** Constructor
     *
     * @param value Default value
     */
    constexpr explicit default_value(value_type value) noexcept : value_{std::move(value)} {}

    /** Called when the owning node's token (if any) is missing from the command line, this will
     * return the default value.
     *
     * @tparam ValueType Parsed value type, must be implicitly constructible from value_type
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param parents Parents instances pack
     * @return Default value
     */
    template <typename ValueType, typename... Parents>
    [[nodiscard]] constexpr ValueType missing_phase(
        [[maybe_unused]] const Parents&... parents) const noexcept
    {
        return value_;
    }

private:
    value_type value_;
};

template <typename T>
struct is_policy<default_value<T>> : std::true_type {
};
}  // namespace arg_router::policy
