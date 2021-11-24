#include "arg_router/parsing.hpp"
#include "arg_router/utility/string_view_ops.hpp"

using namespace arg_router;
using namespace utility::string_view_ops;
using namespace std::string_literals;

parsing::token_list parsing::expand_arguments(int argc, const char* argv[])
{
    auto result = token_list{};

    // Start at 1 to skip the program name
    for (auto i = 1; i < argc; ++i) {
        const auto token = std::string_view{argv[i]};
        const auto [prefix, stripped] = parsing::get_token_type(token);

        if (prefix == parsing::prefix_type::SHORT) {
            for (auto i = 0u; i < stripped.size(); ++i) {
                result.emplace_back(prefix,
                                    std::string_view{&(stripped[i]), 1});
            }
        } else {
            result.emplace_back(prefix, stripped);
        }
    }

    return result;
}

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
