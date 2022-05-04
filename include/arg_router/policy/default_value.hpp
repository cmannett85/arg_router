/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/config.hpp"
#include "arg_router/policy/policy.hpp"

#include <optional>

namespace arg_router
{
namespace policy
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
    constexpr explicit default_value(value_type value) noexcept :
        value_{std::move(value)}
    {
    }

    /** Returns the default value.
     *
     * @return Default value, a reference to it if the object is larger than a
     * cache line
     */
    [[nodiscard]] constexpr auto get_default_value() const noexcept
        -> std::conditional_t<config::l1_cache_size() >= sizeof(value_type),
                              value_type,
                              const value_type&>
    {
        return value_;
    }

    /** Called when the owning node's token (if any) is missing from the command
     * line, this will return the default value.
     * 
     * @tparam ValueType Parsed value type, must be implicitly constructible
     * from value_type
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
}  // namespace policy
}  // namespace arg_router
