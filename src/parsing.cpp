#include "arg_router/parsing.hpp"
#include "arg_router/utility/string_view_ops.hpp"

using namespace arg_router;
using namespace utility::string_view_ops;
using namespace std::string_literals;

std::string_view parsing::to_string(prefix_type prefix)
{
    switch (prefix) {
    case prefix_type::LONG: return config::long_prefix;
    case prefix_type::SHORT: return config::short_prefix;
    default: return "";
    }
}

std::string parsing::to_string(const token_type& token)
{
    return to_string(token.prefix) + token.name;
}

parsing::token_type parsing::get_token_type(std::string_view token)
{
    using namespace config;

    if (token.substr(0, long_prefix.size()) == long_prefix) {
        token.remove_prefix(long_prefix.size());
        return {prefix_type::LONG, std::string{token.data(), token.size()}};
    } else if (token.substr(0, short_prefix.size()) == short_prefix) {
        token.remove_prefix(short_prefix.size());
        return {prefix_type::SHORT, std::string{token.data(), token.size()}};
    } else {
        return {prefix_type::NONE, std::string{token.data(), token.size()}};
    }
}

parsing::token_list parsing::expand_arguments(int argc, const char* argv[])
{
    auto result = token_list{};

    // Start at 1 to skip the program name
    for (auto i = 1; i < argc; ++i) {
        const auto token = std::string_view{argv[i]};
        const auto [prefix, stripped] = parsing::get_token_type(token);

        if (prefix == parsing::prefix_type::SHORT && token.size() > 2) {
            for (auto c : stripped) {
                result.emplace_back(c);
            }
        } else {
            result.emplace_back(prefix, std::move(stripped));
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
