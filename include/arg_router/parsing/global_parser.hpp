// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/traits.hpp"
#include "arg_router/utility/string_view_ops.hpp"

#include <boost/lexical_cast/try_lexical_convert.hpp>

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
 * @exception parse_exception Thrown if parsing failed
 */
template <typename T, typename Enable = void>
struct parser {
    [[noreturn]] constexpr static T parse([[maybe_unused]] std::string_view token) noexcept
    {
        static_assert(traits::always_false_v<T>,
                      "No parse function for this type, use a custom_parser policy "
                      "or define a parser<T>::parse(std::string_view) specialisation");
    }
};

template <typename T>
struct parser<T, typename std::enable_if_t<std::is_arithmetic_v<T>>> {
    [[nodiscard]] static T parse(std::string_view token)
    {
        using namespace utility::string_view_ops;
        using namespace std::string_view_literals;

        auto result = T{};
        if (!boost::conversion::try_lexical_convert(token, result)) {
            throw parse_exception{"Failed to parse: "sv + token};
        }

        return result;
    }
};

template <typename T>
struct parser<T, typename std::enable_if_t<std::is_constructible_v<T, std::string_view>>> {
    [[nodiscard]] static T parse(std::string_view token) { return T{token}; }
};

template <>
struct parser<bool> {
    [[nodiscard]] static inline bool parse(std::string_view token)
    {
        using namespace utility::string_view_ops;
        using namespace std::string_view_literals;

        constexpr auto true_tokens = std::array{
            "true"sv,
            "yes"sv,
            "y"sv,
            "on"sv,
            "1"sv,
            "enable"sv,
        };

        constexpr auto false_tokens = std::array{
            "false"sv,
            "no"sv,
            "n"sv,
            "off"sv,
            "0"sv,
            "disable"sv,
        };

        const auto match = [&](const auto& list) {
            return std::find(list.begin(), list.end(), token) != list.end();
        };

        if (match(true_tokens)) {
            return true;
        }
        if (match(false_tokens)) {
            return false;
        }

        throw parse_exception{"Failed to parse: "sv + token};
    }
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
struct parser<
    T,
    typename std::enable_if_t<traits::has_push_back_method_v<T> && !std::is_same_v<T, string>>> {
    [[nodiscard]] static typename T::value_type parse(std::string_view token)
    {
        return parser<typename T::value_type>::parse(token);
    }
};
}  // namespace arg_router
