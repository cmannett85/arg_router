/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/parsing.hpp"

using namespace arg_router;

parsing::token_list parsing::expand_arguments(int argc, const char* argv[])
{
    auto result = token_list{};

    // Start at 1 to skip the program name
    for (auto i = 1; i < argc; ++i) {
        const auto token = std::string_view{argv[i]};
        const auto [prefix, stripped] = parsing::get_token_type(token);

        if (prefix == parsing::prefix_type::SHORT) {
            for (auto j = 0u; j < stripped.size(); ++j) {
                result.emplace_pending(prefix,
                                       std::string_view{&(stripped[j]), 1});
            }
        } else {
            result.emplace_pending(prefix, stripped);
        }
    }

    return result;
}
