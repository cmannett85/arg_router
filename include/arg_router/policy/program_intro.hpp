#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/policy/policy.hpp"

namespace arg_router
{
namespace policy
{
/** Represents the program introduction.
 *
 * Used by help nodes to display a brief description about the program.
 * @note Must be greater than one character
 * @tparam S compile_time_string
 */
template <typename S>
class program_intro_t
{
public:
    /** String type. */
    using string_type = S;

    /** Returns the program name.
     *
     * @return Program name
     */
    [[nodiscard]] constexpr static std::string_view program_intro() noexcept
    {
        return S::get();
    }

private:
    static_assert(program_intro().size() > 1,
                  "Program intro must be longer than one character");
};

/** Constant variable helper.
 *
 * @tparam S compile_time_string
 */
template <typename S>
constexpr auto program_intro = program_intro_t<S>{};

template <typename S>
struct is_policy<program_intro_t<S>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
