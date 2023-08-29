// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/policy.hpp"
#include "arg_router/utility/compile_time_string.hpp"

namespace arg_router::policy
{
/** Represents the short name of a node.
 *
 * Although this type only accepts a single UTF-8 character, the parser expects it (or the short
 * name group it is a part of) to be preceded by the short prefix.
 *
 * @code
 * constexpr auto c = ar::policy::short_name_t{"h"_S};
 * @endcode
 * @tparam S Compile-time string
 */
template <typename S>
class short_name_t
{
public:
    /** String type. */
    using string_type = S;

    /** Constructor.
     *
     * @param str String instance
     */
    constexpr explicit short_name_t([[maybe_unused]] S str = {}) noexcept {}

    /** Returns the name.
     *
     * @return Short name
     */
    [[nodiscard]] constexpr static std::string_view short_name() noexcept { return S::get(); }

private:
    using full_name_type = AR_STRING_SV(config::short_prefix)::append_t<S>;

    static_assert(utility::utf8::count(short_name()) == 1, "Short name must only be one character");
    static_assert(full_name_type::get() != config::long_prefix,
                  "Short name with short prefix cannot match the long prefix");
};

template <typename S>
struct is_policy<short_name_t<S>> : std::true_type {
};
}  // namespace arg_router::policy
