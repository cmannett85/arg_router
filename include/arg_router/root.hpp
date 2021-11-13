#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/policy/required.hpp"

#include <utility>
#include <variant>

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
    using typename tree_node<Params...>::children_type;

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

        parsing::find_target_node(*this,
                                  tokens,
                                  [](auto const& child, auto& tokens) {
                                      top_level_visitor(child,
                                                        std::move(tokens));
                                  });
    }

private:
    template <typename Node>
    using non_mode_children_type =
        boost::mp11::mp_remove_if<typename Node::children_type,
                                  node_category::is_generic_mode_like>;

    template <typename Node>
    constexpr static bool supports_multiple_values()
    {
        // The logic is that Node supports multiple values if supports multiple
        // value tokens, and it has either no count policy or a count policy
        // that allows for multiple values
        if constexpr (policy::has_value_tokens_v<Node>) {
            if constexpr (traits::has_minimum_count_method_v<Node> &&
                          traits::has_maximum_count_method_v<Node>) {
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
        if constexpr (traits::has_maximum_count_method_v<Node>) {
            return std::min(Node::maximum_count(), tokens.size());
        } else {
            return tokens.size();
        }
    }

    template <typename Node>
    static void top_level_visitor(const Node& node, parsing::token_list tokens)
    {
        using node_type = std::decay_t<Node>;

        // From this point on, we don't care about the non-mode children, so
        // filter them out
        const auto non_mode_children =
            algorithm::tuple_filter_and_construct<  //
                boost::mp11::mp_bind<boost::mp11::mp_not,
                                     boost::mp11::mp_bind<  //
                                         node_category::is_generic_mode_like,
                                         boost::mp11::_1>>::template fn>(
                node.children());

        // Build results tuple
        auto router_args = parsing::optional_router_args_t<node_type>{};

        // Mode-like types need handling separately
        constexpr auto non_mode_children_size =
            std::tuple_size_v<std::decay_t<decltype(non_mode_children)>>;
        if constexpr (non_mode_children_size > 0) {
            constexpr auto router_args_size =
                std::tuple_size_v<std::decay_t<decltype(router_args)>>;
            static_assert(non_mode_children_size == router_args_size,
                          "Non-mode children count must match router arg size");

            // Process the tokens
            while (!tokens.empty()) {
                const auto found_child = parsing::visit_child(
                    tokens.front(),
                    non_mode_children,
                    router_args,
                    [&](auto i, const auto& child) {
                        process_token<i, Node>(child, router_args, tokens);
                    });

                if (!found_child) {
                    throw parse_exception{"Unknown argument", tokens.front()};
                }
            }

            auto fra = filter_aliased_results<Node>(router_args);
            check_hits(node, fra);
            call_router(node, fra, tokens);
        } else {
            process_token<0, Node>(node, router_args, tokens);
            call_router(node, router_args, tokens);
        }
    }

    template <std::size_t I,
              typename Parent,
              typename Node,
              typename RouterArgs>
    static void process_token(const Node& node,
                              RouterArgs& router_args,
                              parsing::token_list& tokens)
    {
        auto& arg = std::get<I>(router_args);

        if constexpr (policy::has_value_tokens_v<Node>) {
            // Drop the token if present, the value follows
            if constexpr (traits::has_match_method_v<Node>) {
                tokens.pop_front();
            }
            auto value = parse_token_argument(node, tokens.front().name);

            if constexpr (traits::has_aliased_policies_type_v<Node>) {
                process_aliased_token<Node, Parent>(value, router_args);
            } else if constexpr (supports_multiple_values<Node>()) {
                if (!arg) {
                    arg = {std::move(value)};
                } else {
                    arg->push_back(std::move(value));
                }

                // Positional args have contiguous value tokens
                if constexpr (policy::has_contiguous_value_tokens_v<Node>) {
                    const auto count = contiguous_token_count<Node>(tokens);
                    for (auto i = 1u; i < count; ++i) {
                        tokens.pop_front();
                        value = parse_token_argument(node, tokens.front().name);
                        arg->push_back(std::move(value));
                    }
                }
            } else {
                if (arg) {
                    throw parse_exception{"Argument has already been set",
                                          tokens.front()};
                }

                arg = std::move(value);
            }
        } else if constexpr (node_category::is_flag_like_v<Node>) {
            if constexpr (traits::has_aliased_policies_type_v<Node>) {
                process_aliased_token<Node, Parent>(true, router_args);
            } else {
                if (arg) {
                    throw parse_exception{"Argument has already been set",
                                          tokens.front()};
                }

                // We have hit the flag, so set the value to true
                arg = true;
            }
        } else if constexpr (node_category::is_counting_flag_like_v<Node>) {
            // Counting flag
        } else {
            static_assert(traits::always_false_v<Node>,
                          "Unhandled node category");
        }

        tokens.pop_front();
    }

    template <typename Node>
    static auto parse_token_argument(const Node& node, std::string_view token)
    {
        // If the node has a custom_parser-like policy, prefer it over the
        // global parse function
        if constexpr (traits::has_parse_method_v<Node>) {
            return node.parse(token);
        } else {
            return arg_router::parser<typename Node::value_type>::parse(token);
        }
    }

    template <typename Node,
              typename Parent,
              typename ArgValue,
              typename RouterArgs>
    static void process_aliased_token(const ArgValue& arg_value,
                                      RouterArgs& router_args)
    {
        using parent_alias_indices =
            typename Node::template aliased_node_indices<Parent>;

        // Assign the alias value to the aliased router args
        std::apply(
            [&](auto... i) {
                if constexpr (supports_multiple_values<Node>()) {
                    constexpr auto pusher = [&](auto j) {
                        auto& arg = std::get<j>(router_args);
                        if (!arg) {
                            arg = arg_value;
                        } else {
                            arg->push_back(arg_value);
                        }
                    };
                    (pusher(i), ...);
                } else {
                    const auto hit_checker = [&](auto j) {
                        if (std::get<j>(router_args)) {
                            using child_type = std::tuple_element_t<
                                j,
                                typename Parent::children_type>;
                            throw parse_exception{
                                "Argument has already been set",
                                parsing::node_token_type<child_type>()};
                        }
                    };
                    (hit_checker(i), ...);
                    (std::get<i>(router_args) = ... = arg_value);
                }
            },
            parent_alias_indices{});
    }

    template <typename T>
    using is_not_an_alias = boost::mp11::mp_not<
        traits::has_aliased_policies_type<boost::mp11::mp_second<T>>>;

    template <typename T>
    using is_not_an_alias_or_mode = boost::mp11::mp_and<
        is_not_an_alias<T>,
        boost::mp11::mp_not<
            node_category::is_generic_mode_like<boost::mp11::mp_second<T>>>>;

    template <typename ChildrenTuple, template <typename...> typename Filter>
    using indices_extractor = typename algorithm::unzip<  //
        boost::mp11::mp_filter<                           //
            Filter,
            algorithm::zip_t<  //
                boost::mp11::mp_iota_c<std::tuple_size_v<ChildrenTuple>>,
                ChildrenTuple>>>::first_type;

    template <typename Node, typename RouterArgs>
    static auto filter_aliased_results(RouterArgs& router_args)
    {
        static_assert((std::tuple_size_v<typename Node::children_type>) > 0,
                      "Node must be top-level, as its children are checked");

        // Filter any alias children from the router args
        using indices =
            indices_extractor<non_mode_children_type<Node>, is_not_an_alias>;
        return std::apply(
            [&](auto... i) {
                return std::tuple{std::move(std::get<i>(router_args))...};
            },
            indices{});
    }

    template <typename Node, typename RouterArgs>
    static void check_hits(const Node& node, RouterArgs& router_args)
    {
        static_assert((std::tuple_size_v<typename Node::children_type>) > 0,
                      "Node must have children");

        // Filter any aliases and modes from children
        using indices = indices_extractor<typename Node::children_type,
                                          is_not_an_alias_or_mode>;

        utility::tuple_iterator(
            [&](auto i, auto orig_index) {
                using node_type =
                    std::tuple_element_t<orig_index,
                                         typename Node::children_type>;
                auto& arg = std::get<i>(router_args);

                if constexpr (policy::is_required_v<node_type>) {
                    if (!arg) {
                        throw parse_exception{
                            "Missing required argument",
                            parsing::node_token_type<node_type>()};
                    }
                } else if constexpr (traits::has_default_value_method_v<
                                         node_type>) {
                    if (!arg) {
                        arg = std::get<orig_index>(node.children())
                                  .get_default_value();
                    }
                } else {
                    if (!arg) {
                        arg =
                            typename std::decay_t<decltype(arg)>::value_type{};
                    }
                }

                // No need to check for max count as visit_child will skip
                // positional_arg-like types that are full
                if constexpr (traits::has_minimum_count_method_v<node_type>) {
                    if constexpr (traits::has_push_back_method_v<
                                      typename node_type::value_type>) {
                        if (std::size(*arg) < node_type::minimum_count()) {
                            throw parse_exception{
                                "Minimum count not reached",
                                parsing::node_token_type<node_type>()};
                        }
                    } else {
                        // Counting flag
                    }
                }
            },
            indices{});
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

        // Convert the tuple of optionals into a tuple of values as all
        // elements now have values
        auto extracted_router_args = std::apply(
            [&](auto... i) {
                return std::tuple{std::move(*std::get<i>(router_args))...};
            },
            boost::mp11::mp_rename<
                boost::mp11::mp_iota_c<std::tuple_size_v<RouterArgs>>,
                std::tuple>{});

        // Call the router, we should not get here without one as it is a rule
        std::apply(node, std::move(extracted_router_args));
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
