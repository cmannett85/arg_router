/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/basic_types.hpp"

#include <string_view>

/** Namespace for std::string_view operators. */
namespace arg_router::utility::string_view_ops
{
/** Concatentation operator between std::string and std::string_view.
 *
 * @tparam CharT Character type
 * @tparam Traits Character traits
 * @tparam Allocator Allocator type
 * @param lhs std::string instance
 * @param rhs std::string_view instance
 * @return std::string of @a lhs and @a rhs concatenated together
 */
template <typename CharT,
          typename Traits = std::char_traits<CharT>,
          typename Allocator = std::allocator<CharT>>
[[nodiscard]] constexpr std::basic_string<CharT, Traits, Allocator> operator+(
    std::basic_string<CharT, Traits, Allocator> lhs,
    std::basic_string_view<CharT, Traits> rhs)
{
    return lhs += rhs;
}

/** Overload that accepts reversed arguments.
 *
 * @tparam CharT Character type
 * @tparam Traits Character traits
 * @tparam Allocator Allocator type
 * @param lhs std::string_view instance
 * @param rhs std::string instance
 * @return std::string of @a lhs and @a rhs concatenated together
 */
template <typename CharT,
          typename Traits = std::char_traits<CharT>,
          typename Allocator = std::allocator<CharT>>
[[nodiscard]] constexpr std::basic_string<CharT, Traits, Allocator> operator+(
    std::basic_string_view<CharT, Traits> lhs,
    std::basic_string<CharT, Traits, Allocator> rhs)
{
    return rhs.insert(0, lhs.data(), lhs.size());
}

/** Returns a std::string using two std::string_views.
 *
 * The returned std::string uses the library allocator
 * (<TT>config::allocator</TT>).
 * @tparam CharT Character type
 * @tparam Traits Character traits
 * @param lhs std::string_view instance
 * @param rhs std::string instance
 * @return std::string of @a lhs and @a rhs concatenated together
 */
template <typename CharT, typename Traits = std::char_traits<CharT>>
[[nodiscard]] constexpr std::basic_string<CharT, Traits, config::allocator<CharT>> operator+(
    std::basic_string_view<CharT, Traits> lhs,
    std::basic_string_view<CharT, Traits> rhs)
{
    return std::basic_string<CharT, Traits, config::allocator<CharT>>(lhs) + rhs;
}
}  // namespace arg_router::utility::string_view_ops
