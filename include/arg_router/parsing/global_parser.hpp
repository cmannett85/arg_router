// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/utility/from_chars.hpp"

namespace arg_router
{
/** Global parsing struct.
 *
 * If you want to provide custom parsing for an entire @em type, then you should specialise this.
 * If you want to provide custom parsing for a particular type just for a single argument, it is
 * usually more convenient to use a policy::custom_parser and define the conversion function inline.
 * @tparam T Type to parse @a token into
 * @param token Command line token to parse
 * @return The parsed instance
 * @exception multi_lang_exception Thrown if parsing failed
 */
template <typename T>
struct parser {
    [[noreturn]] constexpr static T parse([[maybe_unused]] std::string_view token) noexcept
    {
        static_assert(traits::always_false_v<T>,
                      "No parse function for this type, use a custom_parser policy "
                      "or define a parser<T>::parse(std::string_view) specialisation");
    }
};

template <concepts::is_arithmetic T>
struct parser<T> {
    [[nodiscard]] static T parse(std::string_view token)
    {
        const auto result = utility::from_chars<T>(token);
        if (!result) {
            throw multi_lang_exception{error_code::failed_to_parse,
                                       parsing::token_type{parsing::prefix_type::none, token}};
        }

        return *result;
    }
};

template <std::constructible_from<std::string_view> T>
struct parser<T> {
    [[nodiscard]] static T parse(std::string_view token) { return T{token}; }
};

template <>
struct parser<bool> {
    [[nodiscard]] static bool parse(std::string_view token);
};

template <typename T>
struct parser<std::optional<T>> {
    [[nodiscard]] static std::optional<T> parse(std::string_view token)
    {
        return parser<T>::parse(token);
    }
};

// The default vector-like container parser just forwards onto the value_type parser, this is
// because an argument that can be parsed as a complete container will need a custom parser.  In
// other words, this is only used for positional arg parsing
template <typename T>
requires concepts::has_push_back_method<T> &&
    (!concepts::is_specialisation_of<std::basic_string, T>)struct parser<T> {
    [[nodiscard]] static typename T::value_type parse(std::string_view token)
    {
        return parser<typename T::value_type>::parse(token);
    }
};
}  // namespace arg_router
