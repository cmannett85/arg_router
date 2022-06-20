/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

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

template <typename T>
std::ostream& operator<<(std::ostream& stream, span<T> v)
{
    stream << "{";
    for (auto&& item : v) {
        stream << item << ",";
    }
    return stream << "}";
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
ostream& operator<<(ostream& stream, const vector<T>& v)
{
    stream << "{";
    for (auto&& item : v) {
        stream << item << ",";
    }
    return stream << "}";
}
}  // namespace std
