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
    /** Formats the info in @a HelpData and writes it out to @a stream.
     *
     * @tparam DescStart Column at which the description should start
     * @tparam Depth Tree depth of the node @a HelpData is representing
     * @param stream Output stream
     * @param columns Number columns the terminal has, implementations can use this to perform
     * attractive line wrapping
     */
    template <std::size_t DescStart, std::size_t Depth, typename HelpData>
    void format(std::ostream& stream, std::size_t columns)
    {
        if constexpr (!HelpData::label::empty()) {
            constexpr auto indent = indent_size<Depth>();
            stream << red_colour << utility::create_sequence_cts_t<indent, ' '>::get()
                   << HelpData::label::get();

            if constexpr (!HelpData::description::empty()) {
                static_assert(HelpData::description::get().find('\t') == std::string_view::npos,
                              "Help descriptions cannot contain tabs");

                constexpr auto gap =
                    DescStart - indent - utility::utf8::terminal_width(HelpData::label::get());

                // Spacing between the end of the args label and start of description
                stream << green_colour << utility::create_sequence_cts_t<gap, ' '>::get();

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

            stream << "\n" << reset_colour;
        }
    }

private:
    template <std::size_t Depth>
    [[nodiscard]] constexpr static std::size_t indent_size() noexcept
    {
        return Depth * Indent;
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
