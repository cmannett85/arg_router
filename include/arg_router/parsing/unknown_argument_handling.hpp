// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/utility/utf8/levenshtein_distance.hpp"

namespace arg_router::parsing
{
/** Throws a multi_lang_exception with either a error_code::unknown_argument or a
 * error_code::unknown_argument_with_suggestion if the data is available.
 *
 * @tparam Node Node type throwing the exception
 * @param node Node throwing the exception, used as a source for the
 * utility::utf8::closest_matching_child_node(const Node& node, parsing::token_type token)
 * @param unknown_token The token that caused the exception to be thrown
 */
template <typename Node>
[[noreturn]] void unknown_argument_exception(const Node& node, token_type unknown_token)
{
    auto matching_node_and_parents =
        utility::utf8::closest_matching_child_node(node, unknown_token);
    if (matching_node_and_parents.empty()) {
        throw multi_lang_exception{error_code::unknown_argument, unknown_token};
    } else {
        auto tokens = vector<parsing::token_type>{unknown_token};
        tokens.reserve(matching_node_and_parents.size() + 1);
        tokens.insert(tokens.end(),
                      matching_node_and_parents.rbegin(),
                      matching_node_and_parents.rend());

        throw multi_lang_exception{error_code::unknown_argument_with_suggestion, tokens};
    }
}
}  // namespace arg_router::parsing
