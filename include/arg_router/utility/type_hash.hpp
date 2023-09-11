// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string_view>

namespace arg_router::utility
{
/** Hash generation for a type.
 *
 * This is intended to replace <TT>typeid(T).hash_code()</TT>, as the use of typeid causes huge
 * amounts of class name data to be put into the binary's static data storage - which isn't used!
 *
 * @note Aliases are resolved by the compiler in a pre-processing stage before this, so
 * <TT>type_hash<std::uint64_t>() == type_hash<unsigned long>()</TT> (on a 64bit system)
 *
 * Anyone reading the implementation will see that this is @em not a hash function, it just takes
 * the address of the instantiated function.  Originally this used the hash of the
 * <TT>__PRETTY_FUNCTION__</TT> output and therefore could be used at compile-time.  Unfortunately
 * that was ruined when C++20 compile-time string support was added, due to a bug in gcc
 * (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108269) which produced incorrect output when
 * <TT>std::array</TT> was used inside an NTTP.
 * @param T Type to generate a hash code for
 * @return Hash code of @a T
 */
template <typename T>
[[nodiscard]] constexpr std::size_t type_hash() noexcept
{
    return reinterpret_cast<std::size_t>(&type_hash<T>);
}
}  // namespace arg_router::utility
