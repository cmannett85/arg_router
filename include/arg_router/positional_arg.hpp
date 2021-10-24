#pragma once

#include "arg_router/policy/has_contiguous_value_tokens.hpp"
#include "arg_router/tree_node.hpp"

namespace arg_router
{
/** Represents a positional argument on the command line that has potentially
 * multiple values that need parsing.
 *
 * @tparam T Argument value type, must have a <TT>push_back(..)</TT> method
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename T, typename... Policies>
class positional_arg_t :
    public tree_node<policy::has_contiguous_value_tokens_t, Policies...>
{
    static_assert(
        policy::is_all_policies_v<std::tuple<Policies...>>,
        "Positional args must only contain policies (not other nodes)");

public:
    /** Argument value type. */
    using value_type = T;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit positional_arg_t(Policies... policies) :
        tree_node<policy::has_contiguous_value_tokens_t, Policies...>{
            policy::has_contiguous_value_tokens_t{},
            std::move(policies)...}
    {
    }
};

/** Constructs an positional_arg_t with the given policies and value type.
 *
 * This is necessary due to CTAD being required for all template parameters or
 * none, and unfortunately in our case we need @a T to be explicitly set by the
 * user whilst @a Policies need to be deduced.
 * @tparam T Argument value type, must have a <TT>push_back(..)</TT> method
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Argument instance
 */
template <typename T, typename... Policies>
constexpr positional_arg_t<T, Policies...> positional_arg(Policies... policies)
{
    return positional_arg_t<T, std::decay_t<Policies>...>{
        std::move(policies)...};
}
}  // namespace arg_router
