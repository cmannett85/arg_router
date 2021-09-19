#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/utility/string_view_ops.hpp"

#include <bitset>
#include <utility>

namespace arg_router
{
namespace policy
{
namespace validation
{
template <typename... Rules>
class validator;
}
}  // namespace policy

/** The root of the parse tree.
 *
 * @tparam Params The top-level policies and child node types
 */
template <typename... Params>
class root : public tree_node<Params...>
{
public:
    using typename tree_node<Params...>::policies_type;

private:
    constexpr static auto validator_index =
        algorithm::find_specialisation_v<policy::validation::validator,
                                         policies_type>;
    static_assert(validator_index != std::tuple_size_v<policies_type>,
                  "Root must have a validator policy, use "
                  "policy::validation::default_validator unless you have "
                  "created a custom one");

public:
    /** Validator type. */
    // Initially I wanted the default_validator to be used if one isn't user
    // specified, but you get into a circular dependency as the validator needs
    // the root first
    using validator_type = std::tuple_element_t<validator_index, policies_type>;

    /** Constructor.
     *
     * @param params Policy and child instances
     */
    constexpr explicit root(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
        validator_type::template validate<std::decay_t<decltype(*this)>>();
    }

    /** Parse the command line arguments.
     *
     * @param argc Number of arguments
     * @param argv Array of char pointers to the command line tokens
     * @exception std::invalid_argument Thrown if parsing has failed
     */
    void parse(int argc, char* argv[]) const
    {
        using namespace utility::string_view_ops;

        // Trying to handle collapsed short form tokens without expanding them
        // is very painful...  Shame about the heap allocation though
        auto tokens =
            parsing::expand_arguments(argc, const_cast<const char**>(argv));

        // Match the initial token, the first token determines what type the
        // result tuple will be
        if (tokens.empty()) {
            throw parse_exception{"Default value support not added yet!"};
        }

        const auto found_child = parsing::visit_child(
            tokens.front(),
            this->children(),
            [&](auto /*i*/, const auto& child, auto result) {
                top_level_visitor(child, result, std::move(tokens));
            });

        if (!found_child) {
            throw parse_exception{"Unknown argument", tokens.front()};
        }
    }

private:
    template <typename Node>
    static void top_level_visitor(const Node& node,
                                  parsing::match_result m_result,
                                  parsing::token_list tokens)
    {
        using namespace utility::string_view_ops;

        using node_type = std::decay_t<Node>;
        constexpr auto child_count =
            std::tuple_size_v<typename node_type::children_type>;

        // Build results tuple
        using router_args_t = parsing::build_router_args_t<node_type>;
        constexpr auto num_router_args = std::tuple_size_v<router_args_t>;
        static_assert((child_count == num_router_args) ||
                          (child_count == 0 && num_router_args == 1),
                      "Unexpected router arg count, either it is the node's "
                      "child count or 1 if node has no children");

        auto router_args = router_args_t{};

        // We use a bitset to mark which router args have been process, this is
        // so we can catch duplicate tokens and determine which required flags
        // have been missed or non-required ones that need their default value
        // assigning
        auto hit_mask = std::bitset<num_router_args>{};

        if constexpr (child_count > 0) {
            // A mode-like type will have a match but no prefix type, we need to
            // remove its token.  A positional arg will never match so they
            // are not affected by this
            if (tokens.front().prefix == parsing::prefix_type::NONE &&
                m_result.matched) {
                tokens.pop_front();
            }

            // This node has children (i.e. it is mode-like), so go into its
            // children and process them
            while (!tokens.empty()) {
                const auto found_child = parsing::visit_child(
                    tokens.front(),
                    node.children(),
                    [&](auto i, const auto& child, auto result) {
                        mode_level_visitor<i>(child,
                                              result,
                                              router_args,
                                              hit_mask,
                                              tokens.front());
                    });

                if (!found_child) {
                    throw parse_exception{"Unknown argument", tokens.front()};
                }

                tokens.pop_front();
            }
        } else {
            static_assert(num_router_args == 1,
                          "Should only have a single router arg if child has "
                          "no children");

            process_token<0>(node,
                             m_result,
                             router_args,
                             hit_mask,
                             tokens.front());
            tokens.pop_front();
        }

        // If there are any tokens left over, then throw as they are unhandled
        if (!tokens.empty()) {
            throw parse_exception{"Unhandled argument", tokens.front()};
        }

        // Call the router, we should not get here without one as it is a rule
        std::apply(node, std::move(router_args));
    }

    template <std::size_t I, typename Node, typename RouterArgs, std::size_t N>
    static void mode_level_visitor(const Node& node,
                                   parsing::match_result m_result,
                                   RouterArgs& router_args,
                                   std::bitset<N>& hit_mask,
                                   const parsing::token_type& token)
    {
        process_token<I>(node, m_result, router_args, hit_mask, token);
    }

    template <std::size_t I, typename Node, typename RouterArgs, std::size_t N>
    static void process_token(const Node& /*node*/,
                              parsing::match_result m_result,
                              RouterArgs& router_args,
                              std::bitset<N>& hit_mask,
                              const parsing::token_type& token)
    {
        using namespace utility::string_view_ops;
        using value_type = std::tuple_element_t<I, RouterArgs>;

        if (m_result.has_argument) {
            // Parse when we support args
        } else if constexpr (std::is_same_v<value_type, bool>) {
            // Check that the flag has not already been set, if it has then
            // throw
            if (hit_mask[I]) {
                throw parse_exception{"Flag has already been set", token};
            }

            // We have hit the flag, so set the value to true
            std::get<I>(router_args) = true;
            hit_mask[I] = true;
        }
    }
};
}  // namespace arg_router
