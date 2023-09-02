// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/help_data.hpp"
#include "arg_router/parsing/parsing.hpp"

#include <boost/test/unit_test.hpp>

#include <optional>
#include <vector>

namespace arg_router
{
namespace parsing
{
inline std::ostream& operator<<(std::ostream& stream, prefix_type prefix)
{
    return stream << to_string(prefix);
}

inline std::ostream& operator<<(std::ostream& stream, const token_type& token)
{
    return stream << to_string(token);
}

inline std::ostream& operator<<(std::ostream& stream, pre_parse_action action)
{
    switch (action) {
    case pre_parse_action::skip_node: return stream << "skip_node";
    case pre_parse_action::valid_node: return stream << "valid_node";
    case pre_parse_action::skip_node_but_use_sub_targets:
        return stream << "skip_node_but_use_sub_targets";
    default: return stream << "<Unknown>";
    }
}

inline std::ostream& operator<<(std::ostream& stream, pre_parse_result result)
{
    if (result.has_error()) {
        return stream << "<exception>";
    }

    return stream << result.extract();
}
}  // namespace parsing

namespace help_data
{
inline std::ostream& operator<<(std::ostream& stream, const type& hd)
{
    stream << "{\"" << hd.label << "\", \"" << hd.description << "\", [";
    for (auto i = 0u; i < hd.children.size(); ++i) {
        stream << hd.children[i];
        if (i != (hd.children.size() - 1)) {
            stream << ", ";
        }
    }
    return stream << "]";
}
}  // namespace help_data

inline std::ostream& operator<<(std::ostream& stream, error_code ec)
{
    return stream << static_cast<std::underlying_type_t<error_code>>(ec);
}
}  // namespace arg_router

// Naughty, but only used in the test code
namespace std
{
template <typename T>
ostream& operator<<(ostream& stream, const optional<T>& o)
{
    if (o) {
        return stream << *o;
    }
    return stream << "{}";
}

template <typename T>
ostream& operator<<(ostream& stream, const std::vector<T>& v)
{
    stream << "{";
    for (auto&& item : v) {
        stream << item << ",";
    }
    return stream << "}";
}

template <typename T>
ostream& operator<<(ostream& stream, span<T> v)
{
    stream << "{";
    for (auto&& item : v) {
        stream << item << ",";
    }
    return stream << "}";
}
}  // namespace std
