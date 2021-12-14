#include "arg_router/global_parser.hpp"

using namespace arg_router;
using namespace utility::string_view_ops;
using namespace std::string_literals;

bool arg_router::parser<bool>::parse(std::string_view token)
{
    using namespace utility::string_view_ops;
    using namespace std::string_literals;
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
    } else if (match(false_tokens)) {
        return false;
    }

    throw parse_exception{"Failed to parse: "s + token};
}
