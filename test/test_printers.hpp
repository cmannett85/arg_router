#pragma once

#include "arg_router/parsing.hpp"

#include <boost/test/unit_test.hpp>

#include <optional>
#include <vector>

namespace arg_router
{
namespace parsing
{
inline std::ostream& operator<<(std::ostream& stream, const token_type& token)
{
    return stream << to_string(token);
}

inline std::ostream& operator<<(std::ostream& stream, const token_list& list)
{
    return stream << to_string(list);
}

inline std::ostream& operator<<(std::ostream& stream,
                                const token_list::pending_view_type& view)
{
    return stream << to_string(view);
}
}  // namespace parsing
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
    stream << "}";
    for (auto&& item : v) {
        stream << item << ",";
    }
    return stream << "}";
}
}  // namespace std
