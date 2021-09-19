#include "arg_router/exception.hpp"
#include "arg_router/parsing.hpp"
#include "arg_router/utility/string_view_ops.hpp"

using namespace arg_router;
using namespace utility::string_view_ops;

parse_exception::parse_exception(const std::string& message) :
    std::invalid_argument{message}
{
}

parse_exception::parse_exception(const std::string& message,
                                 const parsing::token_type& token) :
    std::invalid_argument{message + ": " + parsing::to_string(token)}
{
}
