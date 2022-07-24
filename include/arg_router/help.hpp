/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/policy/flatten_help.hpp"
#include "arg_router/policy/min_max_count.hpp"
#include "arg_router/policy/no_result_value.hpp"
#include "arg_router/policy/program_intro.hpp"
#include "arg_router/policy/program_name.hpp"
#include "arg_router/policy/program_version.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/terminal.hpp"
#include "arg_router/utility/utf8.hpp"

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
class help_t :
    public tree_node<policy::no_result_value<>,
                     std::decay_t<decltype(policy::min_count<0>)>,
                     std::decay_t<Policies>...>
{
    static_assert(policy::is_all_policies_v<std::tuple<std::decay_t<Policies>...>>,
                  "Help must only contain policies (not other nodes)");

    static_assert(traits::has_long_name_method_v<help_t> || traits::has_short_name_method_v<help_t>,
                  "Help must have a long and/or short name policy");
    static_assert(!traits::has_display_name_method_v<help_t>,
                  "Help must not have a display name policy");
    static_assert(!traits::has_none_name_method_v<help_t>, "Help must not have a none name policy");

    using parent_type = tree_node<policy::no_result_value<>,
                                  std::decay_t<decltype(policy::min_count<0>)>,
                                  std::decay_t<Policies>...>;

    constexpr static auto indent_spaces = 4u;

public:
    using typename parent_type::policies_type;

    /** Help data type.
     *
     * This may seem surprising but of course help is a flag-like type.
     */
    template <bool Flatten>
    using help_data_type = typename parent_type::template default_leaf_help_data_type<Flatten>;

    /** Generates the help string.
     *
     * Recurses through the parse tree, starting at @a Node, at compile time to build a string
     * representation of it.  The program name, version, and info are always generated if the
     * policies are available.
     * @tparam Node The node type to begin help output generation from
     * @tparam Flatten True to display all nested help data, defaults to true if
     * policy::flatten_help is present in the policies list
     * @param stream Output stream to write the output to
     * @return void
     */
    template <typename Node,
              bool Flatten = algorithm::has_specialisation_v<policy::flatten_help_t,
                                                             typename parent_type::policies_type>>
    static void generate_help(std::ostream& stream)
    {
        static_assert(traits::has_help_data_type_v<Node>,
                      "Node must have a help_data_type to generate help from");

        using help_data_type = typename Node::template help_data_type<Flatten>;

        [[maybe_unused]] constexpr auto name_index =
            algorithm::find_specialisation_v<policy::program_name_t,
                                             typename parent_type::policies_type>;
        [[maybe_unused]] constexpr auto version_index =
            algorithm::find_specialisation_v<policy::program_version_t,
                                             typename parent_type::policies_type>;
        [[maybe_unused]] constexpr auto intro_index =
            algorithm::find_specialisation_v<policy::program_intro_t,
                                             typename parent_type::policies_type>;

        // Generate the preamble
        if constexpr (name_index != std::tuple_size_v<typename parent_type::policies_type>) {
            stream << std::tuple_element_t<name_index,
                                           typename parent_type::policies_type>::program_name();
            if constexpr (version_index != std::tuple_size_v<typename parent_type::policies_type>) {
                stream
                    << " "
                    << std::tuple_element_t<version_index,
                                            typename parent_type::policies_type>::program_version();
            }
            stream << "\n"
                   << "\n";
        }
        if constexpr (intro_index != std::tuple_size_v<typename parent_type::policies_type>) {
            stream << std::tuple_element_t<intro_index,
                                           typename parent_type::policies_type>::program_intro()
                   << "\n"
                   << "\n";
        }

        // Calculate description offset
        constexpr auto desc_column = description_column_start<0, help_data_type>(0);

        // Get the current number of console columns, so we can wrap the description nicely.  If
        // the call fails (it returns zero), or the description field start column plus a fixed
        // offset exceeds the column count, then don't attempt to wrap
        const auto columns = utility::terminal::columns();
        constexpr auto desc_column_offset = std::size_t{8};  // Completely arbitrary

        // Generate the args output
        generate_help<desc_column, 0, help_data_type>(
            stream,
            columns >= (desc_column + desc_column_offset) ?
                columns :
                std::numeric_limits<std::size_t>::max());
    }

    /** Constructor.
     *
     * @param policies Policy instances
     */
    constexpr explicit help_t(Policies... policies) noexcept :
        parent_type{policy::no_result_value<>{}, policy::min_count<0>, std::move(policies)...}
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
                generate_help<node_type, flatten>(std::cout);
                std::exit(EXIT_SUCCESS);
            } else {
                auto stream = ostringstream{};
                generate_help<node_type, flatten>(stream);

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

    template <std::size_t Depth>
    [[nodiscard]] constexpr static std::size_t indent_size() noexcept
    {
        return Depth * indent_spaces;
    }

    template <std::size_t Depth, typename HelpData>
    [[nodiscard]] constexpr static std::size_t description_column_start(
        std::size_t current_max) noexcept
    {
        constexpr auto this_row_start = (Depth * indent_spaces) +
                                        utility::utf8::terminal_width(HelpData::label::get()) +
                                        indent_spaces;
        current_max = std::max(current_max, this_row_start);

        utility::tuple_type_iterator<typename HelpData::children>([&](auto i) {
            using child_type = std::tuple_element_t<i, typename HelpData::children>;

            current_max = description_column_start<Depth + 1, child_type>(current_max);
        });

        return current_max;
    }

    template <std::size_t DescStart, std::size_t Depth, typename HelpData>
    static void generate_help(std::ostream& stream, std::size_t columns)
    {
        if constexpr (!HelpData::label::empty()) {
            constexpr auto indent = indent_size<Depth>();
            stream << utility::create_sequence_cts_t<indent, ' '>::get() << HelpData::label::get();

            if constexpr (!HelpData::description::empty()) {
                constexpr auto gap =
                    DescStart - indent - utility::utf8::terminal_width(HelpData::label::get());

                // Spacing between the end of the args label and start of description
                stream << utility::create_sequence_cts_t<gap, ' '>::get();

                // Print the description, breaking if a word will exceed the terminal width
                for (auto it = utility::utf8::line_iterator{HelpData::description::get(),
                                                            columns - DescStart};
                     it != utility::utf8::line_iterator{};) {
                    stream << *it;

                    // If there's more data to follow, then add the offset
                    if (++it != utility::utf8::line_iterator{}) {
                        stream << '\n' << utility::create_sequence_cts_t<DescStart, ' '>::get();
                    }
                }
            }

            stream << "\n";
        }

        utility::tuple_type_iterator<typename HelpData::children>([&](auto i) {
            using child_type = std::tuple_element_t<i, typename HelpData::children>;

            generate_help<DescStart, Depth + 1, child_type>(stream, columns);
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
[[nodiscard]] constexpr auto help(Policies... policies) noexcept
{
    return help_t{std::move(policies)...};
}
}  // namespace arg_router
