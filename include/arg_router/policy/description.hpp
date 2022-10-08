// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"

#include <string_view>

namespace arg_router::policy
{
/** Represents the description of an argument.
 *
 * @note Descriptions must not be empty
 * @tparam S compile_time_string
 */
template <typename S>
class description_t
{
public:
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
 * @tparam S compile_time_string
 */
template <typename S>
constexpr auto description = description_t<S>{};

template <typename T>
struct is_policy<description_t<T>> : std::true_type {
};
}  // namespace arg_router::policy
