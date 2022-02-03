/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/token_type.hpp"
#include "arg_router/utility/string_view_ops.hpp"

using namespace arg_router;
using namespace utility::string_view_ops;
using namespace std::string_literals;

namespace
{
template <typename ViewType>
[[nodiscard]] constexpr bool token_list_view_equality(ViewType lhs,
                                                      ViewType rhs) noexcept
{
    // For some insane reason, span doesn't have equality operators
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename ViewType>
[[nodiscard]] std::string token_list_view_to_string(ViewType view)
{
    auto str = ""s;
    for (auto i = 0u; i < view.size(); ++i) {
        str += to_string(view[i]);
        if (i != (view.size() - 1)) {
            str += ", ";
        }
    }
    return str;
}
}  // namespace

std::string parsing::to_string(const token_type& token)
{
    return std::string{to_string(token.prefix)} + token.name;
}

void parsing::token_list::swap(token_list& other) noexcept
{
    using std::swap;

    swap(data_, other.data_);
    swap(head_offset_, other.head_offset_);
}

bool parsing::operator==(token_list::pending_view_type lhs,
                         token_list::pending_view_type rhs) noexcept
{
    return token_list_view_equality(lhs, rhs);
}

bool parsing::operator==(token_list::processed_view_type lhs,
                         token_list::processed_view_type rhs) noexcept
{
    return token_list_view_equality(lhs, rhs);
}

std::string parsing::to_string(const token_list::pending_view_type& view)
{
    return token_list_view_to_string(view);
}

std::string parsing::to_string(const token_list::processed_view_type& view)
{
    return token_list_view_to_string(view);
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
