#include "arg_router/token_type.hpp"
#include "arg_router/config.hpp"
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
    return std::string{to_string(token.prefix)} + token.name;
}

std::string parsing::to_string(const token_list& tokens)
{
    auto str = ""s;
    for (auto i = 0u; i < tokens.size(); ++i) {
        str += to_string(tokens[i]);
        if (i != (tokens.size() - 1)) {
            str += ", ";
        }
    }
    return str;
}

parsing::token_type parsing::get_token_type(std::string_view token)
{
    using namespace config;

    if (token.substr(0, long_prefix.size()) == long_prefix) {
        token.remove_prefix(long_prefix.size());
        return {prefix_type::LONG, token};
    } else if (token.substr(0, short_prefix.size()) == short_prefix) {
        token.remove_prefix(short_prefix.size());
        return {prefix_type::SHORT, token};
    } else {
        return {prefix_type::NONE, token};
    }
}
