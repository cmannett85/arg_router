#pragma once

#include "arg_router/parsing.hpp"

namespace arg_router
{
/** Represents a flag in the command line.
 *
 * A flag is a boolean indicator, it has no value assigned on the command line,
 * its presence represents a positive boolean value.
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
class flag_t : public tree_node<Policies...>
{
    static_assert(policy::is_all_policies_v<std::tuple<Policies...>>,
                  "Flags must only contain policies (not other nodes)");

public:
    /** Flags always represent boolean values. */
    using value_type = bool;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit flag_t(Policies... policies) :
        tree_node<Policies...>{std::move(policies)...}
    {
    }

    /** Match the token to the long or short form names assigned to this flag by
     * its policies.
     *
     * @param token Command line token to match
     * @return Match result
     */
    parsing::match_result match(const parsing::token_type& token) const
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
