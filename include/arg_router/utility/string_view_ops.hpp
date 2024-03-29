// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/basic_types.hpp"

#include <string_view>

/** Namespace for string_view operators. */
namespace arg_router::utility::string_view_ops
{
/** Concatentation operator between string and string_view.
 *
 * The returned string uses the library allocator (<TT>config::allocator</TT>) by default.
 * @tparam CharT Character type
 * @tparam Traits Character traits
 * @tparam Allocator Allocator type
 * @param lhs string instance
 * @param rhs string_view instance
 * @return string of @a lhs and @a rhs concatenated together
 */
template <typename CharT,
          typename Traits = std::char_traits<CharT>,
          typename Allocator = config::allocator<CharT>>
[[nodiscard]] constexpr std::basic_string<CharT, Traits, Allocator> operator+(
    std::basic_string<CharT, Traits, Allocator> lhs,
    std::basic_string_view<CharT, Traits> rhs)
{
    return lhs += rhs;
}

/** Overload that accepts reversed arguments.
 *
 * The returned string uses the library allocator (<TT>config::allocator</TT>) by default.
 * @tparam CharT Character type
 * @tparam Traits Character traits
 * @tparam Allocator Allocator type
 * @param lhs string_view instance
 * @param rhs string instance
 * @return string of @a lhs and @a rhs concatenated together
 */
template <typename CharT,
          typename Traits = std::char_traits<CharT>,
          typename Allocator = config::allocator<CharT>>
[[nodiscard]] constexpr std::basic_string<CharT, Traits, Allocator> operator+(
    std::basic_string_view<CharT, Traits> lhs,
    std::basic_string<CharT, Traits, Allocator> rhs)
{
    rhs.insert(rhs.begin(), lhs.begin(), lhs.end());
    return rhs;
}

/** Returns a string using two string_views.
 *
 * The returned string uses the library allocator (<TT>config::allocator</TT>).
 * @tparam CharT Character type
 * @tparam Traits Character traits
 * @param lhs string_view instance
 * @param rhs string instance
 * @return string of @a lhs and @a rhs concatenated together
 */
template <typename CharT, typename Traits = std::char_traits<CharT>>
[[nodiscard]] constexpr std::basic_string<CharT, Traits, config::allocator<CharT>> operator+(
    std::basic_string_view<CharT, Traits> lhs,
    std::basic_string_view<CharT, Traits> rhs)
{
    return std::basic_string<CharT, Traits, config::allocator<CharT>>{lhs} += rhs;
}
}  // namespace arg_router::utility::string_view_ops
