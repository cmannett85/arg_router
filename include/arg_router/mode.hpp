#pragma once

#include "arg_router/parsing.hpp"
#include "arg_router/tree_node.hpp"

namespace arg_router
{
/** Allows the grouping of nodes to define operational modes for a program.
 *
 * If no long name policy is provided, then the node is regarded as 'anonymous',
 * and there can only be one in the parse tree.  Conversely, if any mode is
 * named, then there can only be named modes in the parse tree.
 * @tparam Params Policies and child node types for the mode
 */
template <typename... Params>
class mode_t : public tree_node<Params...>
{
public:
    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit mode_t(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }

    /** Match the token to the long form names in the children assigned to this
     * by its policies.
     *
     * @param token Command line token to match, stripped of prefix
     * @return Match result
     */
    bool match(const parsing::token_type& token) const
    {
        // If this mode has a long name, then match it, otherwise check if there
        // is a child that matches
        if constexpr (traits::is_detected_v<parsing::has_long_name_checker,
                                            mode_t>) {
            return (token.prefix == parsing::prefix_type::NONE) &&
                   (token.name == mode_t::long_name());
        } else {
            return parsing::visit_child(token,
                                        this->children(),
                                        [&](auto /*i*/, auto&& /*child*/) {});
        }
    }
};

/** Constructs a mode_t with the given policies.
 *
 * This is used for similarity with arg_t.
 * @tparam Params Policies and child node types for the mode
 * @param params Pack of policy and child node instances
 * @return Mode instance
 */
template <typename... Params>
constexpr mode_t<Params...> mode(Params... params)
{
    return mode_t{std::move(params)...};
}
}  // namespace arg_router
