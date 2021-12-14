#pragma once

#include "arg_router/parsing.hpp"
#include "arg_router/policy/count.hpp"
#include "arg_router/policy/default_value.hpp"

namespace arg_router
{
/** Represents a flag in the command line.
 *
 * A flag is a boolean indicator, it has no value assigned on the command line,
 * its presence represents a positive boolean value.  It has a default value of
 * false and a fixed count of 0.
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
class flag_t :
    public tree_node<policy::default_value<bool>,
                     policy::count_t<std::integral_constant<std::size_t, 0>>,
                     Policies...>
{
    static_assert(policy::is_all_policies_v<std::tuple<Policies...>>,
                  "Flags must only contain policies (not other nodes)");

    using parent_type =
        tree_node<policy::default_value<bool>,
                  policy::count_t<std::integral_constant<std::size_t, 0>>,
                  Policies...>;

public:
    using typename parent_type::policies_type;

    /** Flag value type. */
    using value_type = bool;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit flag_t(Policies... policies) :
        parent_type{policy::default_value<bool>{false},
                    policy::count<0>,
                    std::move(policies)...}
    {
    }

    /** Match the token to the long or short form names assigned to this flag by
     * its policies.
     *
     * @param token Command line token to match
     * @return Match result
     */
    bool match_old(const parsing::token_type& token) const
    {
        return parsing::default_match<flag_t>(token);
    }

    /** Returns true and calls @a visitor if @a token matches the name of this
     * node.
     * 
     * @a visitor needs to be equivalent to:
     * @code
     * [](const auto& node) { ... }
     * @endcode
     * <TT>node</TT> will be a reference to this node.
     * @tparam Fn Visitor type
     * @param token Command line token to match
     * @param visitor Visitor instance
     * @return Match result
     */
    template <typename Fn>
    bool match(const parsing::token_type& token, const Fn& visitor) const
    {
        if (parsing::default_match<flag_t>(token)) {
            visitor(*this);
            return true;
        }

        return false;
    }

    /** Parse function.
     * 
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Token list
     * @param parents Parents instances pack
     * @return Parsed result
     * @exception parse_exception Thrown if parsing failed
     */
    template <typename... Parents>
    value_type parse(parsing::token_list& tokens,
                     const Parents&... parents) const
    {
        // Remove this node's name
        tokens.erase(tokens.begin());

        auto view = utility::span<parsing::token_type>{tokens};

        // Pre-parse
        utility::tuple_type_iterator<policies_type>([&](auto /*i*/, auto ptr) {
            using policy_type = std::remove_pointer_t<decltype(ptr)>;
            if constexpr (policy::has_pre_parse_phase_method_v<policy_type,
                                                               flag_t,
                                                               Parents...>) {
                this->policy_type::pre_parse_phase(tokens,
                                                   view,
                                                   *this,
                                                   parents...);
            }
        });

        // No real parse, post-parse, or validation phase as presence of the
        // flag yields a constant true
        const auto result = true;

        // Routing phase
        using routing_policy = typename parent_type::
            template phase_finder<policy::has_routing_phase_method, bool>::type;
        if constexpr (!std::is_void_v<routing_policy>) {
            this->routing_policy::routing_phase(tokens, result);
        }

        return result;
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
