#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/policy/policy.hpp"

namespace arg_router
{
namespace policy
{
/** Represents the display name of an argument.
 *
 * A display name is a label given to an argument that appears in the help and
 * error output, but is not used in the token parsing.
 * 
 * In the default validator, this policy is not allowed to be used with
 * long_name and short_name - as we shouldn't be trying to confuse the user...
 * @note Display names must not be empty
 * @tparam S compile_time_string
 */
template <typename S>
class display_name_t
{
public:
    /** String type. */
    using string_type = S;

    /** Returns the name.
     *
     * @return Display name
     */
    constexpr static std::string_view display_name() { return S::get(); }

private:
    static_assert(!display_name().empty(), "Display name must not be empty");
};

/** Constant variable helper.
 *
 * @tparam S compile_time_string
 */
template <typename S>
constexpr auto display_name = display_name_t<S>{};

template <typename S>
struct is_policy<display_name_t<S>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
