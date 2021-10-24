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
}  // namespace parsing
}  // namespace arg_router
