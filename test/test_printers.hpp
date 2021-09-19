#pragma once

#include "arg_router/parsing.hpp"

#include <boost/test/unit_test.hpp>

namespace arg_router
{
namespace parsing
{
inline std::ostream& operator<<(std::ostream& stream, const token_type& token)
{
    return stream << to_string(token);
}

inline std::ostream& operator<<(std::ostream& stream, const match_result& mr)
{
    return stream << to_string(mr);
}
}  // namespace parsing
}  // namespace arg_router
