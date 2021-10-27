#pragma once

#include "arg_router/parsing.hpp"
#include "arg_router/policy/default_value.hpp"

namespace arg_router
{
/** Represents a flag in the command line.
 *
 * A flag is a boolean indicator, it has no value assigned on the command line,
 * its presence represents a positive boolean value.  It has a default value of
 * false.
 * 
 * Flags with shortnames can be concatenated or 'collapsed' on the command line,
 * e.g.
 * @code
 * foo -a -b -c
 * foo -abc
 * @endcode
 * 
 * Create with the flag(Policies...) function for consistency with arg_t.
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename... Policies>
class flag_t : public tree_node<policy::default_value<bool>, Policies...>
{
    static_assert(policy::is_all_policies_v<std::tuple<Policies...>>,
                  "Flags must only contain policies (not other nodes)");

public:
    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit flag_t(Policies... policies) :
        tree_node<policy::default_value<bool>, Policies...>{
            policy::default_value<bool>{false},
            std::move(policies)...}
    {
    }

    /** Match the token to the long or short form names assigned to this flag by
     * its policies.
     *
     * @param token Command line token to match
     * @return Match result
     */
    bool match(const parsing::token_type& token) const
    {
        return parsing::default_match<flag_t>(token);
    }
};

/** Constructs a flag_t with the given policies.
 *
 * This is used for similarity with arg_t.
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Flag instance
 */
template <typename... Policies>
constexpr flag_t<Policies...> flag(Policies... policies)
{
    return flag_t{std::move(policies)...};
}
}  // namespace arg_router
