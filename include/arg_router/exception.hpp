#pragma once

#include <stdexcept>

namespace arg_router
{
namespace parsing
{
struct token_type;
}

class parse_exception : public std::invalid_argument
{
public:
    /** Message-only constructor.
     *
     * @param message Error message
     */
    explicit parse_exception(const std::string& message);

    /** Token constructor
     *
     * @param message
     * @param token
     */
    explicit parse_exception(const std::string& message,
                             const parsing::token_type& token);

    virtual ~parse_exception() override = default;
};
}  // namespace arg_router
