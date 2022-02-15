/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/parsing.hpp"
#include "arg_router/policy/flatten_help.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/policy/program_intro.hpp"
#include "arg_router/policy/program_name.hpp"
#include "arg_router/policy/program_version.hpp"
#include "arg_router/tree_node.hpp"

#include <iostream>
#include <sstream>

namespace arg_router
{
/** Generates the help output.
 *
 * Create with the help(Policies...) function for consistency with arg_t.
 * @tparam Policies Pack of policies that define its behaviour
 */
template <typename... Policies>
class help_t : public tree_node<policy::no_result_value<>, Policies...>
{
    static_assert(policy::is_all_policies_v<std::tuple<Policies...>>,
                  "Help must only contain policies (not other nodes)");

    static_assert(traits::has_long_name_method_v<help_t> ||
                      traits::has_short_name_method_v<help_t>,
                  "Help must have a long and/or short name policy");
    static_assert(!traits::has_display_name_method_v<help_t>,
                  "Help must not have a display name policy");
    static_assert(!traits::has_none_name_method_v<help_t>,
                  "Help must not have a none name policy");

    using parent_type = tree_node<policy::no_result_value<>, Policies...>;

    constexpr static auto indent_spaces = 4u;

public:
    using typename parent_type::policies_type;

    /** Help data type.
     *
     * This may seem surprising but of course help is a flag-like type.
     */
    template <bool Flatten>
    using help_data_type =
        typename parent_type::template default_leaf_help_data_type<Flatten>;

    /** Generates the help string.
     *
     * Recurses through the parse tree, starting at @a Node, at compile time to
     * build a string representation of it.  The program name, version, and
     * info are always generated if the policies are available.
     * @tparam Node The node type to begin help output generation from
     * @tparam Flatten True to display all nested help data, defaults to true
     * if policy::flatten_help is present in the policies list
     * @param stream Output stream to write the output to
     * @return void
     */
    template <typename Node,
              bool Flatten = algorithm::has_specialisation_v<
                  policy::flatten_help_t,
                  typename parent_type::policies_type>>
    static void generate_help(std::ostream& stream)
    {
        static_assert(traits::has_help_data_type_v<Node>,
                      "Node must have a help_data_type to generate help from");

        using help_data_type = typename Node::template help_data_type<Flatten>;

        [[maybe_unused]] constexpr auto name_index =
            algorithm::find_specialisation_v<
                policy::program_name_t,
                typename parent_type::policies_type>;
        [[maybe_unused]] constexpr auto version_index =
            algorithm::find_specialisation_v<
                policy::program_version_t,
                typename parent_type::policies_type>;
        [[maybe_unused]] constexpr auto intro_index =
            algorithm::find_specialisation_v<
                policy::program_intro_t,
                typename parent_type::policies_type>;

        // Generate the preamble
        if constexpr (name_index !=
                      std::tuple_size_v<typename parent_type::policies_type>) {
            stream << std::tuple_element_t<
                name_index,
                typename parent_type::policies_type>::program_name();
            if constexpr (version_index !=
                          std::tuple_size_v<
                              typename parent_type::policies_type>) {
                stream << " "
                       << std::tuple_element_t<
                              version_index,
                              typename parent_type::policies_type>::
                              program_version();
            }
            stream << "\n"
                   << "\n";
        }
        if constexpr (intro_index !=
                      std::tuple_size_v<typename parent_type::policies_type>) {
            stream << std::tuple_element_t<
                          intro_index,
                          typename parent_type::policies_type>::program_intro()
                   << "\n"
                   << "\n";
        }

        // Calculate description offset
        constexpr auto desc_column =
            description_column_start<0, help_data_type>(0);

        // Generate the args output
        generate_help<desc_column, 0, help_data_type>(stream);
    }

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit help_t(Policies... policies) noexcept :
        parent_type{policy::no_result_value<>{}, std::move(policies)...}
    {
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
    constexpr bool match(const parsing::token_type& token,
                         const Fn& visitor) const
    {
        if (parsing::default_match<help_t>(token)) {
            visitor(*this);
            return true;
        }

        return false;
    }

    /** Parse function.
     * 
     * Unless a routing policy is specified, then when parsed the help output is
     * sent to <TT>std::cout</TT> and <TT>std::exit(EXIT_SUCCESS)</TT> is
     * called. If a routing policy is called the generated help output is passed
     * to it for further processing and the parse call returns.
     * @tparam Parents Pack of parent tree nodes in ascending ancestry order
     * @param tokens Token list
     * @param parents Parents instances pack
     * @exception parese_exception Thrown if the requested mode cannot be found
     */
    template <typename... Parents>
    void parse(parsing::token_list& tokens, const Parents&... parents) const
    {
        static_assert(sizeof...(Parents) > 0,
                      "Help must have at least one parent");

        // Remove this node's name
        tokens.mark_as_processed();

        const auto& root = std::get<sizeof...(Parents) - 1>(  //
                               std::tuple{std::cref(parents)...})
                               .get();

        find_help_target(tokens, root, [&](const auto& target_node) {
            using root_type = std::decay_t<decltype(root)>;
            using node_type = std::decay_t<decltype(target_node)>;

            // If the user has specified a help target then enable flattening
            // otherwise the output is a bit useless...
            constexpr auto flatten = algorithm::has_specialisation_v<
                                         policy::flatten_help_t,
                                         typename parent_type::policies_type> ||
                                     !std::is_same_v<root_type, node_type>;

            // If there is a routing policy then delegate to it, otherwise print
            // the help output to std::cout and exit
            using routing_policy =
                typename parent_type::template phase_finder_t<
                    policy::has_routing_phase_method>;
            if constexpr (std::is_void_v<routing_policy>) {
                generate_help<node_type, flatten>(std::cout);
                std::exit(EXIT_SUCCESS);
            } else {
                auto stream = std::ostringstream{};
                generate_help<node_type, flatten>(stream);

                this->routing_policy::routing_phase(tokens, std::move(stream));
            }
        });
    }

private:
    static_assert(!parent_type::template any_phases_v<
                      bool,  // Type doesn't matter, as long as it isn't void
                      policy::has_pre_parse_phase_method,
                      policy::has_parse_phase_method,
                      policy::has_validation_phase_method,
                      policy::has_missing_phase_method>,
                  "Help only supports policies with a routing phase");

    template <typename Node, typename TargetFn>
    static void find_help_target(parsing::token_list& tokens,
                                 const Node& node,
                                 const TargetFn& fn)
    {
        if (tokens.pending_view().empty()) {
            fn(node);
            return;
        }

        const auto token = tokens.pending_view().front();
        const auto result =
            node.find(token, [&](auto /*i*/, const auto& child) {
                tokens.mark_as_processed();
                find_help_target(tokens, child, fn);
            });
        if (!result) {
            throw parse_exception{"Unknown argument", token};
        }
    }

    template <std::size_t Depth>
    [[nodiscard]] constexpr static std::size_t indent_size() noexcept
    {
        return Depth * indent_spaces;
    }

    template <std::size_t Depth, typename HelpData>
    [[nodiscard]] constexpr static std::size_t description_column_start(
        std::size_t current_max) noexcept
    {
        constexpr auto this_row_start =
            (Depth * indent_spaces) + HelpData::label::size() + indent_spaces;
        current_max = std::max(current_max, this_row_start);

        utility::tuple_type_iterator<typename HelpData::children>([&](auto i) {
            using child_type =
                std::tuple_element_t<i, typename HelpData::children>;

            current_max =
                description_column_start<Depth + 1, child_type>(current_max);
        });

        return current_max;
    }

    template <std::size_t DescStart, std::size_t Depth, typename HelpData>
    static void generate_help(std::ostream& stream)
    {
        if constexpr (!HelpData::label::empty()) {
            constexpr auto indent = indent_size<Depth>();
            stream << utility::create_sequence_cts_t<indent, ' '>::get()
                   << HelpData::label::get();

            if constexpr (!HelpData::description::empty()) {
                constexpr auto gap = DescStart - indent -  //
                                     HelpData::label::size();
                stream << utility::create_sequence_cts_t<gap, ' '>::get()
                       << HelpData::description::get();
            }

            stream << "\n";
        }

        utility::tuple_type_iterator<typename HelpData::children>([&](auto i) {
            using child_type =
                std::tuple_element_t<i, typename HelpData::children>;

            generate_help<DescStart, Depth + 1, child_type>(stream);
        });
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
[[nodiscard]] constexpr help_t<Policies...> help(Policies... policies) noexcept
{
    return help_t{std::move(policies)...};
}
}  // namespace arg_router
