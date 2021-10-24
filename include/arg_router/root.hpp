#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/policy/has_contiguous_value_tokens.hpp"
#include "arg_router/policy/required.hpp"

#include <optional>
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
        // Trying to handle collapsed short form tokens without expanding them
        // is very painful...  Shame about the heap allocation though, hopefully
        // SSO will eliminate it for most cases
        auto tokens =
            parsing::expand_arguments(argc, const_cast<const char**>(argv));

        // If there are no tokens, then there must an anonymous mode that has
        // no required arguments in it - otherwise it is an error
        if (tokens.empty()) {
            handle_empty_args();
            return;
        }

        // Match the initial token, the first token determines what type the
        // result tuple will be
        const auto found_child = parsing::visit_child(
            tokens.front(),
            this->children(),
            [&](auto /*i*/, const auto& child) {
                top_level_visitor(child, std::move(tokens));
            });

        if (!found_child) {
            throw parse_exception{"Unknown argument", tokens.front()};
        }
    }

private:
    template <typename Node>
    constexpr static bool supports_multiple_values()
    {
        // The logic is that Node supports multiple values if supports multiple
        // value tokens, and it has either no count policy or a count policy
        // that allows for multiple values
        if constexpr (policy::has_value_tokens_v<Node>) {
            if constexpr (traits::is_detected_v<
                              parsing::has_minimum_count_checker,
                              Node> &&
                          traits::is_detected_v<
                              parsing::has_maximum_count_checker,
                              Node>) {
                return !((Node::minimum_count() == Node::maximum_count()) &&
                         (Node::minimum_count() == 1));
            }

            return true;
        }

        return false;
    }

    template <typename Node>
    static std::size_t contiguous_token_count(const parsing::token_list& tokens)
    {
        // If Node has a fixed count then use it (maxed to tokens size) or use
        // the token size
        if constexpr (traits::is_detected_v<parsing::has_maximum_count_checker,
                                            Node>) {
            return std::min(Node::maximum_count(), tokens.size());
        } else {
            return tokens.size();
        }
    }

    void handle_empty_args() const
    {
        // To support custom modes, we don't just look for a mode_t, we need to
        // find a top-level tree node that has no names - only mode-like types
        // are allowed to be anonymous
        auto found = false;
        utility::tuple_iterator(
            [&](auto /*i*/, auto& child) {
                using child_type = std::decay_t<decltype(child)>;

                if constexpr (is_tree_node_v<child_type> &&
                              !traits::is_detected_v<
                                  parsing::has_long_name_checker,
                                  child_type> &&
                              !traits::is_detected_v<
                                  parsing::has_short_name_checker,
                                  child_type>) {
                    found = true;
                    top_level_visitor(child, parsing::token_list{});
                }
            },
            this->children());

        if (!found) {
            throw parse_exception{"No arguments provided"};
        }
    }

    template <typename Node>
    static void top_level_visitor(const Node& node, parsing::token_list tokens)
    {
        using node_type = std::decay_t<Node>;
        constexpr auto is_named_mode_like_type =
            !policy::has_value_tokens_v<node_type> &&
            traits::is_detected_v<parsing::has_match_checker, node_type> &&
            traits::is_detected_v<parsing::has_long_name_checker, node_type> &&
            !(traits::is_detected_v<parsing::has_maximum_count_checker,
                                    node_type> ||
              traits::is_detected_v<parsing::has_minimum_count_checker,
                                    node_type>);

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

        // We use a bitset to mark which router args have been processed, this
        // is so we can catch duplicate tokens and determine which required
        // flags have been missed or non-required ones that need their default
        // value assigning
        auto hit_mask = std::bitset<num_router_args>{};

        if constexpr (child_count > 0) {
            // Non-anonymous mode-like types need to have their labels removed
            if (!tokens.empty() &&
                tokens.front().prefix == parsing::prefix_type::NONE &&
                is_named_mode_like_type) {
                tokens.pop_front();
            }

            // This node has children (i.e. it is mode-like), so go into its
            // children and process them
            while (!tokens.empty()) {
                const auto found_child =
                    parsing::visit_child(tokens.front(),
                                         node.children(),
                                         router_args,
                                         hit_mask,
                                         [&](auto i, const auto& child) {
                                             process_token<i, Node>(child,
                                                                    router_args,
                                                                    hit_mask,
                                                                    tokens);
                                         });

                if (!found_child) {
                    throw parse_exception{"Unknown argument", tokens.front()};
                }
            }

            auto [fra, fhm] =
                filter_aliased_results<Node>(hit_mask, router_args);
            check_hit_mask(node, fhm, fra);

            call_router(node, fra, tokens);
        } else {
            static_assert(num_router_args == 1,
                          "Should only have a single router arg if child has "
                          "no children");

            process_token<0, void>(node, router_args, hit_mask, tokens);

            call_router(node, router_args, tokens);
        }
    }

    template <std::size_t I,
              typename Parent,
              typename Node,
              typename RouterArgs>
    static void process_token(
        const Node& node,
        RouterArgs& router_args,
        std::bitset<std::tuple_size_v<RouterArgs>>& hit_mask,
        parsing::token_list& tokens)
    {
        using value_type = std::tuple_element_t<I, RouterArgs>;

        if (policy::has_value_tokens_v<Node>) {
            // Drop the token if present, the value follows
            if constexpr (traits::is_detected_v<parsing::has_match_checker,
                                                Node>) {
                tokens.pop_front();
            }
            auto value = parse_token_argument(node, tokens.front().name);

            if constexpr (traits::is_detected_v<
                              parsing::has_aliased_node_indices,
                              Node>) {
                process_aliased_token<Node, Parent>(value,
                                                    router_args,
                                                    hit_mask);
            } else if constexpr (supports_multiple_values<Node>()) {
                std::get<I>(router_args).push_back(std::move(value));

                // Positional args have contiguous value tokens
                if constexpr (policy::has_contiguous_value_tokens_v<Node>) {
                    const auto count = contiguous_token_count<Node>(tokens);
                    for (auto i = 1u; i < count; ++i) {
                        tokens.pop_front();
                        value = parse_token_argument(node, tokens.front().name);
                        std::get<I>(router_args).push_back(std::move(value));
                    }
                }
            } else {
                if (hit_mask[I]) {
                    throw parse_exception{"Argument has already been set",
                                          tokens.front()};
                }

                std::get<I>(router_args) = std::move(value);
            }
        } else if constexpr (std::is_same_v<value_type, bool>) {
            if constexpr (traits::is_detected_v<
                              parsing::has_aliased_node_indices,
                              Node>) {
                process_aliased_token<Node, Parent>(true,
                                                    router_args,
                                                    hit_mask);
            } else {
                if (hit_mask[I]) {
                    throw parse_exception{"Argument has already been set",
                                          tokens.front()};
                }

                // We have hit the flag, so set the value to true
                std::get<I>(router_args) = true;
            }
        } else {
            // Counting flag
        }

        hit_mask[I] = true;
        tokens.pop_front();
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
            return arg_router::parser<typename Node::value_type>::parse(token);
        }
    }

    template <typename Node,
              typename Parent,
              typename ArgValue,
              typename RouterArgs>
    static void process_aliased_token(
        const ArgValue& arg_value,
        RouterArgs& router_args,
        std::bitset<std::tuple_size_v<RouterArgs>>& hit_mask)
    {
        using parent_alias_indices =
            typename Node::template aliased_node_indices<Parent>;

        // Assign the alias value to the aliased router args, and update the
        // hitmask
        std::apply(
            [&](auto... i) {
                if constexpr (supports_multiple_values<Node>()) {
                    (std::get<i>(router_args).push_back(arg_value), ...);
                } else {
                    (std::get<i>(router_args) = ... = arg_value);
                }

                // We could use another fold expression here, but I need the
                // index of the already-hit arg for the error message, so we
                // need to iterate
                utility::tuple_iterator(
                    [&](auto /*i*/, auto index) {
                        if (hit_mask[index]) {
                            using child_type = std::tuple_element_t<
                                index,
                                typename Parent::children_type>;
                            throw parse_exception{
                                "Argument has already been set",
                                parsing::node_token_type<child_type>()};
                        }
                    },
                    parent_alias_indices{});

                (hit_mask[i] = ... = true);
            },
            parent_alias_indices{});
    }

    template <typename T>
    struct is_second_an_alias {
        constexpr static auto value =
            traits::is_detected_v<parsing::has_aliased_node_indices,
                                  boost::mp11::mp_second<T>>;
    };

    template <typename T>
    using is_second_not_an_alias = boost::mp11::mp_not<is_second_an_alias<T>>;

    template <typename Node, template <typename...> typename Filter>
    using alias_indices = typename algorithm::unzip<  //
        boost::mp11::mp_filter<                       //
            Filter,
            algorithm::zip_t<  //
                boost::mp11::mp_iota_c<
                    std::tuple_size_v<typename Node::children_type>>,
                typename Node::children_type>>>::first_type;

    template <typename Node, typename RouterArgs>
    static auto filter_aliased_results(
        std::bitset<std::tuple_size_v<RouterArgs>> hit_mask,
        RouterArgs& router_args)
    {
        // Filter any alias children from the router args
        using non_alias_indices = alias_indices<Node, is_second_not_an_alias>;
        auto updated_router_args = std::apply(
            [&](auto... I) {
                return std::tuple{std::move(std::get<I>(router_args))...};
            },
            non_alias_indices{});

        // Reverse sort the indices so they can be used to remove the hit mask
        // entries that are for alias entries
        using sorted_alias_indices = boost::mp11::mp_sort<
            alias_indices<Node, is_second_an_alias>,
            boost::mp11::mp_not_fn<boost::mp11::mp_less>::template fn>;
        auto hit_mask_value = hit_mask.to_ullong();
        std::apply(
            [&](auto... I) {
                ((hit_mask_value = algorithm::remove_bit<I>(hit_mask_value)),
                 ...);
            },
            sorted_alias_indices{});

        return std::pair{
            std::move(updated_router_args),
            std::bitset<std::tuple_size_v<non_alias_indices>>{hit_mask_value}};
    }

    template <typename Node, typename RouterArgs>
    static void check_hit_mask(
        const Node& node,
        std::bitset<std::tuple_size_v<RouterArgs>> hit_mask,
        RouterArgs& router_args)
    {
        static_assert((std::tuple_size_v<typename Node::children_type>) > 0,
                      "Node must be top-level, as its children are checked");

        using non_alias_indices = alias_indices<Node, is_second_not_an_alias>;

        utility::tuple_iterator(
            [&](auto i, auto orig_index) {
                using node_type =
                    std::tuple_element_t<orig_index,
                                         typename Node::children_type>;

                if constexpr (policy::is_required_v<node_type>) {
                    if (!hit_mask[i]) {
                        throw parse_exception{
                            "Missing required argument",
                            parsing::node_token_type<node_type>()};
                    }
                } else if constexpr (traits::is_detected_v<
                                         parsing::has_default_value_checker,
                                         node_type>) {
                    if (!hit_mask[i]) {
                        std::get<i>(router_args) =
                            std::get<orig_index>(node.children())
                                .get_default_value();
                    }
                }

                // No need to check for max count as visit_child will skip
                // positional_arg-like types that are full
                if constexpr (traits::is_detected_v<
                                  parsing::has_minimum_count_checker,
                                  node_type>) {
                    if constexpr (traits::is_detected_v<
                                      parsing::has_push_back_checker,
                                      typename node_type::value_type>) {
                        if (std::size(std::get<i>(router_args)) <
                            node_type::minimum_count()) {
                            throw parse_exception{
                                "Minimum count not reached",
                                parsing::node_token_type<node_type>()};
                        }
                    } else {
                        // Counting flag
                    }
                }
            },
            non_alias_indices{});
    }

    template <typename Node, typename RouterArgs>
    static void call_router(const Node& node,
                            RouterArgs& router_args,
                            parsing::token_list& tokens)
    {
        // If there are any tokens left over, then throw as they are unhandled
        if (!tokens.empty()) {
            throw parse_exception{"Unhandled argument", tokens.front()};
        }

        // Call the router, we should not get here without one as it is a rule
        std::apply(node, std::move(router_args));
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
