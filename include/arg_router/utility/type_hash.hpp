/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include <string_view>

namespace arg_router::utility
{
namespace detail
{
// constexpr hash combine implementations, based on Boost's equivalent:
// https://github.com/boostorg/container_hash/blob/boost-1.74.0/include/boost/container_hash/hash.hpp#L316
[[nodiscard]] constexpr std::size_t hash_combine(std::size_t a, std::size_t b) noexcept
{
    static_assert((sizeof(std::size_t) == 4) || (sizeof(std::size_t) == 8),
                  "std::size_t must be 32 or 64 bits");

    // Personally I hate this and previously had it as two separate overloads, but that caused
    // ambiguity on AppleClang hence approach
    if constexpr (sizeof(std::size_t) == 4) {
        constexpr auto c1 = std::uint32_t{0xcc9e2d51};
        constexpr auto c2 = std::uint32_t{0x1b873593};

        // NOLINTBEGIN(readability-magic-numbers)
        b *= c1;
        b = (b << 15) | (b >> (32 - 15));  // ROTL 15 bits
        b *= c2;

        a ^= b;
        a = (a << 13) | (a >> (32 - 13));  // ROTL 13 bits
        a = a * 5 + 0xe6546b64;
        // NOLINTEND(readability-magic-numbers)

        return a;
    } else {
        constexpr auto m = std::uint64_t{0xc6a4a7935bd1e995};
        constexpr auto r = 47;

        b *= m;
        b ^= b >> r;
        b *= m;

        a ^= b;
        a *= m;

        // Completely arbitrary number, to prevent 0's from hashing to 0.
        // NOLINTNEXTLINE(readability-magic-numbers)
        a += 0xe6546b64;

        return a;
    }
}

// Do this rather than just put the generate() body into type_hash(), because otherwise the compiler
// puts the pretty function output into static storage, needlessly bloating exe size
template <typename T>
class type_hash_t
{
    [[nodiscard]] constexpr static std::size_t generate() noexcept
    {
        // Because we can't guarantee default construction support for T and reinterpret_cast is not
        // allowed in constant evaluation, we'll resort to the (valid, but) dirty hack of using the
        // function signature (which contains the fully qualified name of T)
#ifdef _MSC_VER
        constexpr auto sig = std::string_view{__FUNCSIG__};
#else
        constexpr auto sig = std::string_view{static_cast<const char*>(__PRETTY_FUNCTION__)};
#endif

        auto result = std::size_t{0};
        for (auto c : sig) {
            result = detail::hash_combine(result, c);
        }

        return result;
    }

public:
    constexpr static auto value = generate();
};
}  // namespace detail

/** Compile-time hash generation for a type.
 *
 * This is intended to replace <TT>typeid(T).hash_code()</TT>, as the use of typeid causes huge
 * amounts of class name data to be put into the binary's static data storage - which isn't used!
 * @note Aliases are resolved by the compiler in a pre-processing stage before this, so
 * <TT>type_hash<std::uint64_t>() == type_hash<unsigned long>()</TT> (on a 64bit system)
 * @param T Type to generate a hash code for
 * @return Hash code of @a T
 */
template <typename T>
[[nodiscard]] constexpr std::size_t type_hash() noexcept
{
    return detail::type_hash_t<T>::value;
}
}  // namespace arg_router::utility
