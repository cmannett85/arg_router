#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/policy/required.hpp"
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
class root_t : public tree_node<Params...>
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
    constexpr explicit root_t(Params... params) :
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
            // Find the anonymous mode, and call top_level_visitor on it
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
                                              tokens);
                    });

                if (!found_child) {
                    throw parse_exception{"Unknown argument", tokens.front()};
                }

                tokens.pop_front();
            }

            // If the hit mask shows that not all required args have been hit,
            // then throw.  Required arguments can not be top-level
            check_hit_mask(node, hit_mask, router_args);
        } else {
            static_assert(num_router_args == 1,
                          "Should only have a single router arg if child has "
                          "no children");

            process_token<0>(node, m_result, router_args, hit_mask, tokens);
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
                                   parsing::token_list& tokens)
    {
        process_token<I>(node, m_result, router_args, hit_mask, tokens);
    }

    template <std::size_t I, typename Node, typename RouterArgs, std::size_t N>
    static void process_token(const Node& node,
                              parsing::match_result m_result,
                              RouterArgs& router_args,
                              std::bitset<N>& hit_mask,
                              parsing::token_list& tokens)
    {
        using namespace utility::string_view_ops;
        using value_type = std::tuple_element_t<I, RouterArgs>;
        constexpr auto supports_multiple_values =
            traits::is_detected_v<parsing::has_push_back_checker, value_type>;

        if (m_result.has_argument) {
            tokens.pop_front();  // Drop the argument token, the value follows
            auto value = parse_token_argument(node, tokens.front().name);

            if constexpr (supports_multiple_values) {
                std::get<I>(router_args).push_back(std::move(value));
            } else {
                if (hit_mask[I]) {
                    throw parse_exception{"Argument has already been set",
                                          tokens.front()};
                }

                std::get<I>(router_args) = std::move(value);
            }
        } else if constexpr (std::is_same_v<value_type, bool>) {
            if (hit_mask[I]) {
                throw parse_exception{"Argument has already been set",
                                      tokens.front()};
            }

            // We have hit the flag, so set the value to true
            std::get<I>(router_args) = true;
        } else {
            // Counting flag
        }

        hit_mask[I] = true;
    }

    template <typename Node>
    static auto parse_token_argument(const Node& node, std::string_view token)
    {
        // If the node has a custom_parser-like policy, prefer it over the
        // global parse function
        if constexpr (traits::is_detected_v<parsing::has_custom_parser_checker,
                                            Node>) {
            return node.parse(token);
        } else {
            return arg_router::parse<typename Node::value_type>(token);
        }
    }

    template <typename Node, std::size_t N, typename RouterArgs>
    static void check_hit_mask(const Node& node,
                               std::bitset<N> hit_mask,
                               RouterArgs& router_args)
    {
        static_assert((std::tuple_size_v<typename Node::children_type>) > 0,
                      "Node must be top-level, as its children are checked");

        using namespace std::string_literals;
        using namespace utility::string_view_ops;

        utility::tuple_iterator(
            [&](auto i, const auto& child) {
                using node_type = std::decay_t<decltype(child)>;

                if constexpr (policy::is_required_v<node_type>) {
                    const auto name = parsing::node_name(child);
                    if (!hit_mask[i]) {
                        throw parse_exception{"Missing required argument: "s +
                                              name};
                    }
                } else if constexpr (traits::is_detected_v<
                                         parsing::has_default_value,
                                         node_type>) {
                    if (!hit_mask[i]) {
                        std::get<i>(router_args) = child.get_default_value();
                    }
                }
            },
            node.children());
    }
};

/** Constructs a root_t with the given policies.
 *
 * This is used for similarity with arg_t.
 * @tparam Params Policies and child node types for the mode
 * @param params Pack of policy and child node instances
 * @return Root instance
 */
template <typename... Params>
constexpr root_t<Params...> root(Params... params)
{
    return root_t{std::move(params)...};
}
}  // namespace arg_router
