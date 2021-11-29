#pragma once

#include "arg_router/parsing.hpp"

#include <boost/test/unit_test.hpp>

#include <optional>

namespace arg_router
{
namespace parsing
{
inline std::ostream& operator<<(std::ostream& stream, const token_type& token)
{
    return stream << to_string(token);
}
}  // namespace parsing
}  // namespace arg_router

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
}  // namespace std
