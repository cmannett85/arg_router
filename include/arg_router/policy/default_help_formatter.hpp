// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/policy/program_addendum.hpp"
#include "arg_router/policy/program_intro.hpp"
#include "arg_router/policy/program_name.hpp"
#include "arg_router/policy/program_version.hpp"
#include "arg_router/tree_node_fwd.hpp"
#include "arg_router/utility/compile_time_string.hpp"
#include "arg_router/utility/terminal.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

#include <iomanip>
#include <limits>

namespace arg_router::policy
{
/** Namespace for the built-in help formatter components.
 *
 * These components are used with default_help_formatter_t to control help output.  Users can take
 * their concepts to customise the help customise output without needing to write an entire new
 * help node.
 */
namespace help_formatter_component
{
/** The built-in line formatter.
 *
 * Prints each help argument line.  Entries at a higher depth in the tree are indented more.
 * @tparam Indent Number of spaces per 'level' of indentation
 */
template <std::size_t Indent>
class default_line_formatter
{
    static_assert(Indent > 0, "Indent must be greater than zero");

public:
    /** Formats the info in @a HelpData and writes it out to @a stream.
     *
     * @tparam DescStart Column at which the description should start
     * @tparam Depth Tree depth of the node @a HelpData is representing
     * @tparam HelpData Help data type from the node that needs printing
     * @param stream Output stream
     * @param columns Number of columns the terminal has, implementations can use this to perform
     * attractive line wrapping
     */
    template <std::size_t DescStart, std::size_t Depth, typename HelpData>
    void format(std::ostream& stream, std::size_t columns)
    {
        if constexpr (!HelpData::label::empty()) {
            constexpr auto indent = indent_size<Depth>();
            stream << utility::create_sequence_cts_t<indent, ' '>::get() << HelpData::label::get();

            if constexpr (!HelpData::description::empty()) {
                static_assert(HelpData::description::get().find('\t') == std::string_view::npos,
                              "Help descriptions cannot contain tabs");

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

            stream << '\n';
        }
    }

    /** Overload for runtime_help_data.
     *
     * @param stream Output stream
     * @param desc_start Column at which the description should start
     * @param depth Tree depth of the node @a help_data is representing
     * @param columns Number of columns the terminal has, implementations can use this to perform
     * attractive line wrapping
     * @param help_data Help data from the node that needs printing
     */
    void format(std::ostream& stream,
                std::size_t desc_start,
                std::size_t depth,
                std::size_t columns,
                const runtime_help_data& help_data)
    {
        if (!help_data.label.empty()) {
            const auto indent = depth * Indent;
            set_gap(stream, indent);
            stream << help_data.label;

            if (!help_data.description.empty()) {
                const auto gap =
                    desc_start - indent - utility::utf8::terminal_width(help_data.label);

                // Spacing between the end of the args label and start of description
                set_gap(stream, gap);

                // Print the description, breaking if a word will exceed the terminal width
                for (auto it =
                         utility::utf8::line_iterator{help_data.description, columns - desc_start};
                     it != utility::utf8::line_iterator{};) {
                    stream << *it;

                    // If there's more data to follow, then add the offset
                    if (++it != utility::utf8::line_iterator{}) {
                        stream << '\n';
                        set_gap(stream, desc_start);
                    }
                }
            }

            stream << '\n';
        }
    }

private:
    template <std::size_t Depth>
    [[nodiscard]] constexpr static std::size_t indent_size() noexcept
    {
        return Depth * Indent;
    }

    static void set_gap(std::ostream& stream, std::size_t num_chars)
    {
        if (num_chars > 0) {
            stream << std::setw(num_chars) << ' ';
        }
    }
};

/** Default preamble formatter */
class default_preamble_formatter
{
public:
    /** Formats any help-related policies in @a HelpNode (e.g. policy::program_name_t) and writes it
     * out to @a stream.
     *
     * @tparam HelpNode Help node controlling the help generation
     * @param stream Output stream
     */
    template <typename HelpNode>
    void format(std::ostream& stream)
    {
        [[maybe_unused]] constexpr auto name_index =
            algorithm::find_specialisation_v<policy::program_name_t,
                                             typename HelpNode::policies_type>;
        [[maybe_unused]] constexpr auto version_index =
            algorithm::find_specialisation_v<policy::program_version_t,
                                             typename HelpNode::policies_type>;
        [[maybe_unused]] constexpr auto intro_index =
            algorithm::find_specialisation_v<policy::program_intro_t,
                                             typename HelpNode::policies_type>;

        // Generate the preamble
        if constexpr (name_index != std::tuple_size_v<typename HelpNode::policies_type>) {
            stream << std::tuple_element_t<name_index,
                                           typename HelpNode::policies_type>::program_name();
            if constexpr (version_index != std::tuple_size_v<typename HelpNode::policies_type>) {
                stream << " "
                       << std::tuple_element_t<version_index,
                                               typename HelpNode::policies_type>::program_version();
            }
            stream << "\n"
                   << "\n";
        }
        if constexpr (intro_index != std::tuple_size_v<typename HelpNode::policies_type>) {
            stream << std::tuple_element_t<intro_index,
                                           typename HelpNode::policies_type>::program_intro()
                   << "\n"
                   << "\n";
        }
    }
};

/** Default addendum formatter */
class default_addendum_formatter
{
public:
    /** Formats an available program_addendum_t policy in @a HelpNode and writes it out to
     * @a stream.
     *
     * @tparam HelpNode Help node controlling the help generation
     * @param stream Output stream
     */
    template <typename HelpNode>
    void format(std::ostream& stream)
    {
        [[maybe_unused]] constexpr auto addendum_index =
            algorithm::find_specialisation_v<policy::program_addendum_t,
                                             typename HelpNode::policies_type>;

        if constexpr (addendum_index != std::tuple_size_v<typename HelpNode::policies_type>) {
            stream << "\n"
                   << std::tuple_element_t<addendum_index,
                                           typename HelpNode::policies_type>::program_addendum()
                   << "\n";
        }
    }
};
}  // namespace help_formatter_component

/** Default help formatter, used when none is specified when defining a help node.
 *
 * @tparam Indent Number of spaces per 'level' of indentation, defaults to 4
 * @tparam DescColumnOffset Minimum number of description columns remaining in a terminal line
 * needed to attempt proper line breaks i.e. if it is less than @a DescColumnOffset then the output
 * just overflows onto the next line, defaults to 8
 * @tparam LineFormatter Line formatter type
 * @tparam PreambleFormatter Controls preamble formatting i.e. the 'intro' part of the output that
 * goes before the argument output
 * @tparam AddendumFormatter Controls addendum formatting i.e. the part of the output that goes
 * after the argument output
 */
template <typename Indent = traits::integral_constant<std::size_t{4}>,
          typename DescColumnOffset = traits::integral_constant<Indent{} * 2>,
          typename LineFormatter = help_formatter_component::default_line_formatter<Indent{}>,
          typename PreambleFormatter = help_formatter_component::default_preamble_formatter,
          typename AddendumFormatter = help_formatter_component::default_addendum_formatter>
class default_help_formatter_t
{
    static_assert(traits::has_value_type_v<Indent>, "Indent must have a value_type");
    static_assert(Indent{} > 0, "Indent value_type must be greater than zero");
    static_assert(traits::has_value_type_v<DescColumnOffset>,
                  "DescColumnOffset must have a value_type");
    static_assert(DescColumnOffset{} > 0, "DescColumnOffset value_type must be greater than zero");

public:
    /** Generates the help string.
     *
     * Recurses through the parse tree, starting at @a Node, at compile time to build a string
     * representation of it.  The program name, version, and info are always generated if the
     * policies are available.
     * @tparam Node The node type to begin help output generation from (typically the root)
     * @tparam HelpNode Parent help node
     * @tparam Flatten True to display all nested help data
     * @param stream Output stream to write the output to
     */
    template <typename Node, typename HelpNode, bool Flatten>
    static void generate_help(std::ostream& stream)
    {
        generate_help_impl<Node, HelpNode, Flatten>(stream);
    }

    /** Overload for runtime_help_data.
     *
     * @tparam Node The node type to begin help output generation from (typically the root)
     * @tparam HelpNode Parent help node
     * @tparam Flatten True to display all nested help data
     * @param stream Output stream to write the output to
     * @param help_data Parent runtime help data
     */
    template <typename Node, typename HelpNode, bool Flatten>
    static void generate_help(std::ostream& stream, const runtime_help_data& help_data)
    {
        generate_help_impl<Node, HelpNode, Flatten>(stream, &help_data);
    }

private:
    template <typename Node, typename HelpNode, bool Flatten>
    static void generate_help_impl(std::ostream& stream,
                                   const runtime_help_data* help_data = nullptr)
    {
        static_assert(traits::has_help_data_type_v<Node>,
                      "Node must have a help_data_type to generate help from");

        using help_data_type = typename Node::template help_data_type<Flatten>;

        // Write out the preamble
        auto preamble_formatter = PreambleFormatter{};
        preamble_formatter.template format<HelpNode>(stream);

        // Calculate description offset
        constexpr auto desc_column = description_column_start<0, help_data_type>(0);

        // Get the current number of console columns, so we can wrap the description nicely.  If
        // the call fails (it returns zero), or the description field start column plus a fixed
        // offset exceeds the column count, then don't attempt to wrap
        const auto columns = utility::terminal::columns();

        // Generate the args output
        auto line_formatter = LineFormatter{};
        if (help_data) {
            line_formatter_dispatch(stream,
                                    desc_column,
                                    0,
                                    columns >= (desc_column + DescColumnOffset{}) ?
                                        columns :
                                        std::numeric_limits<std::size_t>::max(),
                                    line_formatter,
                                    *help_data);
        } else {
            line_formatter_dispatch<desc_column, 0, help_data_type>(
                stream,
                columns >= (desc_column + DescColumnOffset{}) ?
                    columns :
                    std::numeric_limits<std::size_t>::max(),
                line_formatter);
        }

        // Write out the addendum
        auto addendum_formatter = AddendumFormatter{};
        addendum_formatter.template format<HelpNode>(stream);
    }

    template <std::size_t Depth, typename HelpData>
    [[nodiscard]] constexpr static std::size_t description_column_start(
        std::size_t current_max) noexcept
    {
        constexpr auto this_row_start =
            (Depth * Indent{}) + utility::utf8::terminal_width(HelpData::label::get()) + Indent{};
        current_max = std::max(current_max, this_row_start);

        utility::tuple_type_iterator<typename HelpData::children>([&](auto i) {
            using child_type = std::tuple_element_t<i, typename HelpData::children>;

            current_max = description_column_start<Depth + 1, child_type>(current_max);
        });

        return current_max;
    }

    template <std::size_t DescStart, std::size_t Depth, typename HelpData>
    static void line_formatter_dispatch(std::ostream& stream,
                                        std::size_t columns,
                                        LineFormatter& line_formatter)
    {
        line_formatter.template format<DescStart, Depth, HelpData>(stream, columns);

        utility::tuple_type_iterator<typename HelpData::children>([&](auto i) {
            using child_type = std::tuple_element_t<i, typename HelpData::children>;

            line_formatter_dispatch<DescStart, Depth + 1, child_type>(stream,
                                                                      columns,
                                                                      line_formatter);
        });
    }

    static void line_formatter_dispatch(std::ostream& stream,
                                        std::size_t desc_start,
                                        std::size_t depth,
                                        std::size_t columns,
                                        LineFormatter& line_formatter,
                                        const runtime_help_data& help_data)
    {
        line_formatter.format(stream, desc_start, depth, columns, help_data);

        for (const auto& child_help_data : help_data.children) {
            line_formatter_dispatch(stream,
                                    desc_start,
                                    depth + 1,
                                    columns,
                                    line_formatter,
                                    child_help_data);
        }
    }
};

/** Constant variable helper. */
constexpr auto default_help_formatter = default_help_formatter_t<>{};

template <typename Indent,
          typename DescColumnOffset,
          typename LineFormatter,
          typename PreambleFormatter>
struct is_policy<
    default_help_formatter_t<Indent, DescColumnOffset, LineFormatter, PreambleFormatter>> :
    std::true_type {
};
}  // namespace arg_router::policy
