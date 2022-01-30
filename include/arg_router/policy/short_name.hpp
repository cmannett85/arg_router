#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/policy/policy.hpp"

namespace arg_router
{
namespace policy
{
/** Represents the short name of an argument.
 * 
 * Although this type only accepts a single character, the parser expects it
 * (or the short name group it is a part of) to be preceded by a hyphen.
 * @tparam S Integral constant that can be implicitly converted to a char
 */
template <typename S>
class short_name_t
{
    static_assert(std::is_convertible_v<S, char>,
                  "Short name type must be implicitly convertible to char");
    static_assert(algorithm::is_alnum(S::value),
                  "Short name character must be alphanumeric");

    constexpr static auto value = S::value;

public:
    /** String type. */
    using string_type = S;

    /** Returns the name.
     *
     * @return Short name
     */
    [[nodiscard]] constexpr static std::string_view short_name() noexcept
    {
        return std::string_view{&value, 1};
    }
};

/** Constant variable helper.
 *
 * @tparam S Short name character
 */
template <char S>
constexpr auto short_name = short_name_t<traits::integral_constant<S>>{};

template <typename S>
struct is_policy<short_name_t<S>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
