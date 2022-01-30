#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/policy/policy.hpp"

namespace arg_router
{
namespace policy
{
/** Represents the program name.
 *
 * Used by help nodes to produce their output, though in principle can be used
 * by anything that wants to.
 * @note Names must be greater than one character and cannot contain any
 * whitespace characters
 * @tparam S compile_time_string
 */
template <typename S>
class program_name_t
{
public:
    /** String type. */
    using string_type = S;

    /** Returns the program name.
     *
     * @return Program name
     */
    constexpr static std::string_view program_name() { return S::get(); }

private:
    static_assert(program_name().size() > 1,
                  "Program names must be longer than one character");
    static_assert(
        algorithm::is_alnum(program_name()[0]),
        "Program name must not start with a non-alphanumeric character");
    static_assert(!algorithm::contains_whitespace(program_name()),
                  "Program names cannot contain whitespace");
};

/** Constant variable helper.
 *
 * @tparam S compile_time_string
 */
template <typename S>
constexpr auto program_name = program_name_t<S>{};

template <typename S>
struct is_policy<program_name_t<S>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
