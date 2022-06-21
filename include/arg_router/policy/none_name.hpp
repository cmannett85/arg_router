/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/policy/policy.hpp"

namespace arg_router
{
namespace policy
{
/** Represents the name of an argument that does not use any token prefix (i.e.
 * parsing::prefix_type == none).
 *
 * The only node that uses this in the library is mode_t.
 * @note Display names must not be empty
 * @tparam S compile_time_string
 */
template <typename S>
class none_name_t
{
public:
    /** String type. */
    using string_type = S;

    /** Returns the name.
     *
     * @return None name
     */
    [[nodiscard]] constexpr static std::string_view none_name() noexcept { return S::get(); }

private:
    static_assert(none_name().size() > 1, "None names must be longer than one character");
    static_assert(algorithm::is_alnum(none_name()[0]),
                  "None name must not start with a non-alphanumeric character");
    static_assert(!algorithm::contains_whitespace(none_name()),
                  "None names cannot contain whitespace");
};

/** Constant variable helper.
 *
 * @tparam S compile_time_string
 */
template <typename S>
constexpr auto none_name = none_name_t<S>{};

template <typename S>
struct is_policy<none_name_t<S>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
