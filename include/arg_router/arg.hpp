#pragma once

#include "arg_router/parsing.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/has_value_tokens.hpp"

namespace arg_router
{
/** Represents an argument on the command line that has a value that needs
 * parsing.
 *
 * @tparam T Argument value type
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename T, typename... Policies>
class arg_t :
    public tree_node<policy::has_value_tokens_t,
                     policy::count_t<std::integral_constant<std::size_t, 1>>,
                     Policies...>
{
    static_assert(policy::is_all_policies_v<std::tuple<Policies...>>,
                  "Args must only contain policies (not other nodes)");

public:
    /** Argument value type. */
    using value_type = T;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit arg_t(Policies... policies) :
        tree_node<policy::has_value_tokens_t,
                  policy::count_t<std::integral_constant<std::size_t, 1>>,
                  Policies...>{policy::has_value_tokens_t{},
                               policy::count<1>,
                               std::move(policies)...}
    {
    }

    /** Match the token to the long or short form names assigned to this arg by
     * its policies.
     *
     * @param token Command line token to match
     * @return Match result
     */
    bool match(const parsing::token_type& token) const
    {
        return parsing::default_match<arg_t>(token);
    }
};

/** Constructs an arg_t with the given policies and value type.
 *
 * This is necessary due to CTAD being required for all template parameters or
 * none, and unfortunately in our case we need @a T to be explicitly set by the
 * user whilst @a Policies need to be deduced.
 * @tparam T Argument value type
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Argument instance
 */
template <typename T, typename... Policies>
constexpr arg_t<T, Policies...> arg(Policies... policies)
{
    return arg_t<T, std::decay_t<Policies>...>{std::move(policies)...};
}
}  // namespace arg_router
