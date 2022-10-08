// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/default_help_formatter.hpp"
#include "arg_router/policy/flatten_help.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/tree_node.hpp"

#include <iostream>
#include <sstream>

namespace arg_router
{
namespace detail
{
template <typename... Policies>
class add_missing_formatter_policy
{
    using policies_tuple = std::tuple<std::decay_t<Policies>...>;
    using default_formatter_type = policy::default_help_formatter_t<>;

public:
    constexpr static auto has_formatter =
        boost::mp11::mp_find_if<policies_tuple, traits::has_generate_help_method>::value !=
        std::tuple_size_v<policies_tuple>;

    using type = std::conditional_t<
        has_formatter,
        boost::mp11::mp_rename<policies_tuple, tree_node>,
        boost::mp11::mp_rename<boost::mp11::mp_push_front<policies_tuple, default_formatter_type>,
                               tree_node>>;
};
}  // namespace detail

/** Generates the help output.
 *
 * Create with the help(Policies...) function for consistency with arg_t.
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename... Policies>
class help_t :
    public detail::add_missing_formatter_policy<policy::no_result_value<>,
                                                std::decay_t<decltype(policy::min_count<0>)>,
                                                std::decay_t<Policies>...>::type
{
private:
    static_assert(policy::is_all_policies_v<std::tuple<std::decay_t<Policies>...>>,
                  "Help must only contain policies (not other nodes)");

    static_assert(traits::has_long_name_method_v<help_t> || traits::has_short_name_method_v<help_t>,
                  "Help must have a long and/or short name policy");
    static_assert(!traits::has_display_name_method_v<help_t>,
                  "Help must not have a display name policy");
    static_assert(!traits::has_none_name_method_v<help_t>, "Help must not have a none name policy");

    using parent_type =
        typename detail::add_missing_formatter_policy<policy::no_result_value<>,
                                                      std::decay_t<decltype(policy::min_count<0>)>,
                                                      std::decay_t<Policies>...>::type;

    constexpr static auto formatter_index =
        boost::mp11::mp_find_if<typename parent_type::policies_type,
                                traits::has_generate_help_method>::value;
    static_assert(formatter_index != std::tuple_size_v<typename parent_type::policies_type>,
                  "Help node must have a formatter policy");
    using formatter_type =
        std::tuple_element_t<formatter_index, typename parent_type::policies_type>;

public:
    using typename parent_type::policies_type;
    using parent_type::generate_help;

    /** Help data type.
     *
     * This may seem surprising but of course help is a flag-like type.
     */
    template <bool Flatten>
    using help_data_type = typename parent_type::template default_leaf_help_data_type<Flatten>;

    /** Constructor.
     *
     * @param policies Policy instances
     */
    template <auto has_formatter = detail::add_missing_formatter_policy<Policies...>::has_formatter>
    constexpr explicit help_t(Policies... policies,
                              // NOLINTNEXTLINE(*-named-parameter)
                              std::enable_if_t<has_formatter>* = nullptr) noexcept :
        parent_type{policy::no_result_value<>{}, policy::min_count<0>, std::move(policies)...}
    {
    }

    template <auto has_formatter = detail::add_missing_formatter_policy<Policies...>::has_formatter>
    constexpr explicit help_t(Policies... policies,
                              // NOLINTNEXTLINE(*-named-parameter)
                              std::enable_if_t<!has_formatter>* = nullptr) noexcept :
        parent_type{policy::default_help_formatter,
                    policy::no_result_value<>{},
                    policy::min_count<0>,
                    std::move(policies)...}
    {
    }

    template <typename Validator, bool HasTarget, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const

    {
        static_assert((sizeof...(Parents) >= 1), "At least one parent needed for help");

        return parent_type::pre_parse(pre_parse_data, *this, parents...);
    }

    /** Parse function.
     *
     * Unless a routing policy is specified, then when parsed the help output is sent to
     * <TT>std::cout</TT> and <TT>std::exit(EXIT_SUCCESS)</TT> is called. If a routing policy is
     * called the generated help output is passed to it for further processing and the parse call
     * returns.
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param target Parse target
     * @param parents Parents instances pack
     * @exception parese_exception Thrown if the requested mode cannot be found
     */
    template <typename... Parents>
    void parse(parsing::parse_target&& target, const Parents&... parents) const
    {
        const auto& root = std::get<sizeof...(Parents) - 1>(  //
                               std::tuple{std::cref(parents)...})
                               .get();

        find_help_target(target.tokens(), root, [&](const auto& target_node) {
            using root_type = std::decay_t<decltype(root)>;
            using node_type = std::decay_t<decltype(target_node)>;

            // If the user has specified a help target then enable flattening otherwise the output
            // is a bit useless...
            constexpr auto flatten =
                algorithm::has_specialisation_v<policy::flatten_help_t,
                                                typename parent_type::policies_type> ||
                !std::is_same_v<root_type, node_type>;

            // If there is a routing policy then delegate to it, otherwise print the help output to
            // std::cout and exit
            using routing_policy =
                typename parent_type::template phase_finder_t<policy::has_routing_phase_method>;
            if constexpr (std::is_void_v<routing_policy>) {
                formatter_type::template generate_help<node_type, help_t, flatten>(std::cout);
                std::exit(EXIT_SUCCESS);
            } else {
                auto stream = ostringstream{};
                formatter_type::template generate_help<node_type, help_t, flatten>(stream);
                this->routing_policy::routing_phase(std::move(stream));
            }
        });
    }

private:
    static_assert(
        !parent_type::template any_phases_v<bool,  // Type doesn't matter, as long as it isn't void
                                            policy::has_parse_phase_method,
                                            policy::has_validation_phase_method,
                                            policy::has_missing_phase_method>,
        "Help only supports policies with pre-parse and routing phases");

    template <typename Node, typename TargetFn>
    static void find_help_target(vector<parsing::token_type>& tokens,
                                 const Node& node,
                                 const TargetFn& fn)
    {
        if (tokens.empty()) {
            fn(node);
            return;
        }

        // Help tokens aren't pre-parsed by the target nodes (as they would fail if missing any
        // required value tokens), so we have just use the prefix to generate a token_type from
        // them, as they all of a prefix_type of none
        const auto token = parsing::get_token_type(tokens.front().name);

        auto result = false;
        utility::tuple_iterator(
            [&](auto /*i*/, const auto& child) {
                using child_type = std::decay_t<decltype(child)>;

                if (!result && parsing::match<child_type>(token)) {
                    result = true;
                    tokens.erase(tokens.begin());
                    find_help_target(tokens, child, fn);
                }
            },
            node.children());

        if (!result) {
            throw parse_exception{"Unknown argument", token};
        }
    }
};

/** Constructs a help_t with the given policies.
 *
 * This is used for similarity with arg_t.
 * @tparam Policies Pack of policies that define its behaviour
 * @param policies Pack of policy instances
 * @return Help instance
 */
template <typename... Policies>
[[nodiscard]] constexpr auto help(Policies... policies) noexcept
{
    return help_t<std::decay_t<Policies>...>{std::move(policies)...};
}
}  // namespace arg_router
