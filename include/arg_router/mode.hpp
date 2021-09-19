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
class mode : public tree_node<Params...>
{
public:
    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit mode(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }

    /** Match the token to the long form names in the children assigned to this
     * by its policies.
     *
     * @param token Command line token to match, stripped of prefix
     * @return Match result
     */
    parsing::match_result match(const parsing::token_type& token) const
    {
        // If this mode has a long name, then match it, otherwise look at the
        // children
        auto result = parsing::match_result{};
        if constexpr (traits::is_detected_v<parsing::has_long_name_checker,
                                            mode>) {
            if ((token.prefix == parsing::prefix_type::NONE) &&
                (token.name == mode::long_name())) {
                result.matched = parsing::match_result::MATCH;
            }
        } else {
            parsing::visit_child(
                token,
                this->children(),
                [&](auto /*i*/, auto&& /*child*/, auto mr) { result = mr; });
        }

        return result;
    }
};
}  // namespace arg_router
