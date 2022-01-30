#pragma once

#include "arg_router/exception.hpp"
#include "arg_router/traits.hpp"
#include "arg_router/utility/string_view_ops.hpp"

#include <charconv>

namespace arg_router
{
/** Global parsing struct.
 *
 * If you want to provide custom parsing for an entire @em type, then you should
 * specialise this.  If you want to provide custom parsing for a particular type
 * just for a single argument, it is usually more convenient to use a
 * policy::custom_parser and define the conversion function inline.
 * @tparam T Type to parse @a token into
 * @param token Command line token to parse
 * @return The parsed instance
 * @exception parse_exception Thrown if parsing failed
 */
template <typename T, typename Enable = void>
struct parser {
    [[noreturn]] constexpr static T parse(
        [[maybe_unused]] std::string_view token) noexcept
    {
        static_assert(
            traits::always_false_v<T>,
            "No parse function for this type, use a custom_parser policy "
            "or define a parser<T>::parse(std::string_view) specialisation");
    }
};

template <typename T>
struct parser<T, typename std::enable_if_t<std::is_arithmetic_v<T>>> {
    [[nodiscard]] static T parse(std::string_view token)
    {
        using namespace utility::string_view_ops;
        using namespace std::string_literals;

        if (token.front() == '+') {
            token.remove_prefix(1);
        }

        auto value = T{};
        const auto result = std::from_chars(token.begin(), token.end(), value);

        if (result.ec == std::errc{}) {
            return value;
        }

        if (result.ec == std::errc::result_out_of_range) {
            throw parse_exception{"Value out of range for argument: "s + token};
        }

        throw parse_exception{"Failed to parse: "s + token};
    }
};

template <>
struct parser<std::string_view> {
    [[nodiscard]] constexpr static inline std::string_view parse(
        std::string_view token) noexcept
    {
        return token;
    }
};

template <>
struct parser<bool> {
    [[nodiscard]] static bool parse(std::string_view token);
};

// The default vector-like container parser just forwards onto the value_type
// parser, this is because an argument that can be parsed as a complete
// container will need a custom parser.  In other words, this is only used for
// positional arg parsing
template <typename T>
struct parser<T, typename std::enable_if_t<traits::has_push_back_method_v<T>>> {
    [[nodiscard]] static typename T::value_type parse(std::string_view token)
    {
        return parser<typename T::value_type>::parse(token);
    }
};
}  // namespace arg_router
