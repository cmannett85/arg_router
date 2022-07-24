/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/compile_time_string.hpp"
#include "arg_router/utility/utf8.hpp"

namespace arg_router::policy
{
/** Represents the short name of an argument.
 *
 * Although this type only accepts a single character, the parser expects it (or the short name
 * group it is a part of) to be preceded by the short prefix.
 * @tparam S Compile-time string
 */
template <typename S>
class short_name_t
{
public:
    /** String type. */
    using string_type = S;

    /** Returns the name.
     *
     * @return Short name
     */
    [[nodiscard]] constexpr static std::string_view short_name() noexcept { return S::get(); }

private:
    using full_name_type = S_(config::short_prefix)::append_t<S>;

    static_assert(utility::utf8::count(short_name()) == 1, "Short name must only be one character");
    static_assert(full_name_type::get() != config::long_prefix,
                  "Short name with short prefix cannot match the long prefix");
};

/** Constant variable helper.
 *
 * @tparam S Short name character
 */
template <char S>
constexpr auto short_name = short_name_t<S_(S)>{};

/** Constant variable helper that supports UTF-8 code points.
 *
 * @tparam S Compile-time string
 */
template <typename S>
constexpr auto short_name_utf8 = short_name_t<S>{};

template <typename S>
struct is_policy<short_name_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
