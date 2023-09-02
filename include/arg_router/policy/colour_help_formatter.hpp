// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/policy/default_help_formatter.hpp"

namespace arg_router::policy
{
namespace help_formatter_component
{
/** The built-in colour line formatter.
 *
 * Same format as the default_help_formatter_t output, but the argument labels are red and the
 * descriptions green.
 * @tparam Indent Number of spaces per 'level' of indentation
 */
template <std::size_t Indent>
class colour_line_formatter
{
    static_assert(Indent > 0, "Indent must be greater than zero");

    constexpr static auto reset_colour = std::string_view{"\033[0m"};
    constexpr static auto red_colour = std::string_view{"\033[31m"};
    constexpr static auto green_colour = std::string_view{"\033[32m"};

public:
    /** Formats the per-entry data.
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
                const help_data::type& help_data)
    {
        if (!help_data.label.empty()) {
            const auto indent = depth * Indent;
            stream << red_colour << std::setw(indent) << ' ' << help_data.label;

            if (!help_data.description.empty()) {
                const auto gap =
                    desc_start - indent - utility::utf8::terminal_width(help_data.label);

                // Spacing between the end of the args label and start of description
                stream << green_colour << std::setw(gap) << ' ';

                // Print the description, breaking if a word will exceed the terminal width
                for (auto it =
                         utility::utf8::line_iterator{help_data.description, columns - desc_start};
                     it != utility::utf8::line_iterator{};) {
                    stream << *it;

                    // If there's more data to follow, then add the offset
                    if (++it != utility::utf8::line_iterator{}) {
                        stream << '\n' << std::setw(desc_start) << ' ';
                    }
                }
            }

            stream << "\n" << reset_colour;
        }
    }
};
}  // namespace help_formatter_component

/** Same format as the default_help_formatter_t output, but the argument labels are red and the
 * descriptions green.
 *
 * @tparam Indent Number of spaces per 'level' of indentation, defaults to 4
 * @tparam DescColumnOffset Minimum number of description columns remaining in a terminal line
 * needed to attempt proper line breaks i.e. if it is less than @a DescColumnOffset then the output
 * just overflows onto the next line, defaults to 8
 * @tparam PreambleFormatter Controls preamble formatting i.e. the 'intro' part of the output that
 * goes before the argument output
 * @tparam AddendumFormatter Controls addendum formatting i.e. the part of the output that goes
 * after the argument output
 */
template <typename Indent = traits::integral_constant<std::size_t{4}>,
          typename DescColumnOffset = traits::integral_constant<Indent{} * 2>,
          typename PreambleFormatter = help_formatter_component::default_preamble_formatter,
          typename AddendumFormatter = help_formatter_component::default_addendum_formatter>
using colour_help_formatter_t =
    default_help_formatter_t<Indent,
                             DescColumnOffset,
                             help_formatter_component::colour_line_formatter<Indent{}>,
                             PreambleFormatter,
                             AddendumFormatter>;

/** Constant variable helper. */
constexpr auto colour_help_formatter = colour_help_formatter_t<>{};
}  // namespace arg_router::policy
