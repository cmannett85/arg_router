// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/parsing/global_parser.hpp"

namespace arg_router
{
[[nodiscard]] bool parser<bool>::parse(std::string_view token)
{
    using namespace std::string_view_literals;

    static auto true_tokens = std::array{
        "true"sv,
        "yes"sv,
        "y"sv,
        "on"sv,
        "1"sv,
        "enable"sv,
    };

    static auto false_tokens = std::array{
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

    throw multi_lang_exception{error_code::failed_to_parse,
                               parsing::token_type{parsing::prefix_type::none, token}};
}
}  // namespace arg_router
