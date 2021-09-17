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
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename... Policies>
class flag : public tree_node<Policies...>
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
    constexpr explicit flag(Policies... policies) :
        tree_node<Policies...>{std::move(policies)...}
    {
    }

    /** Match the token to the long form names assigned to this flag by its
     * policies.
     *
     * @param token Command line token to match, stripped of prefix
     * @return Match result
     */
    constexpr static parsing::match_result match(std::string_view token)
    {
        return parsing::default_match<flag>(token);
    }
};
}  // namespace arg_router
