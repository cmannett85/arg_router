// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/parsing/token_type.hpp"

namespace arg_router::parsing
{
std::string to_string(const std::vector<token_type>& view)
{
    auto str = std::string{};
    for (auto i = 0u; i < view.size(); ++i) {
        str += to_string(view[i]);
        if (i != (view.size() - 1)) {
            str += ", ";
        }
    }
    return str;
}

token_type get_token_type(std::string_view token)
{
    using namespace config;

    if (token.substr(0, long_prefix.size()) == long_prefix) {
        token.remove_prefix(long_prefix.size());
        return {prefix_type::long_, token};
    }
    if (token.substr(0, short_prefix.size()) == short_prefix) {
        token.remove_prefix(short_prefix.size());
        return {prefix_type::short_, token};
    }
    return {prefix_type::none, token};
}
}  // namespace arg_router::parsing
