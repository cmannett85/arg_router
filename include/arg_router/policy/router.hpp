#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/policy/policy.hpp"

#include <utility>

namespace arg_router
{
namespace policy
{
/** Provides a Callable that is executed on a successful parse.
 *
 * @tparam Fn Callable function object type
 */
template <typename Fn>
class router
{
public:
    /** Callable function object type alias. */
    using callable_type = Fn;

    /** Constructor.
     *
     * @param f Callable that is executed on a successful parse
     */
    explicit router(Fn f) : f_{std::move(f)} {}

    /** Execute the result of the parsed command line arguments.
     *
     * @tparam Args Argument types, must be implicitly convertible to the types
     * in callable_args_type, in the order and number specified by 
     * callable_args_type
     * @param args Argument values
     */
    template <typename... Args>
    void operator()(Args&&... args) const
    {
        f_(std::forward<Args>(args)...);
    }

protected:
    /** Executes the result of the parsed command line arguments.
     * 
     * @tparam Args Argument types, must be implicitly convertible to the types
     * in callable_args_type, in the order and number specified by 
     * callable_args
     * @param tokens Token list, if there any entries remaining then an
     * exception is thrown
     * @param args Argument values
     * @exception parse_exception Thrown if @a tokens is not empty
     */
    template <typename... Args>
    void routing_phase(const parsing::token_list& tokens, Args&&... args) const
    {
        if (!tokens.empty()) {
            throw parse_exception{"Unhandled arguments", tokens};
        }
        f_(std::forward<Args>(args)...);
    }

private:
    Fn f_;
};

template <typename Fn>
struct is_policy<router<Fn>> : std::true_type {
};
}  // namespace policy
}  // namespace arg_router
