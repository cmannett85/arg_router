/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/basic_types.hpp"
#include "arg_router/token_type.hpp"

#include <stdexcept>

namespace arg_router
{
/** An exception that represents a parsing failure.
 */
class parse_exception : public std::invalid_argument
{
public:
    /** Message-only constructor.
     *
     * @param message Error message
     */
    explicit parse_exception(const string& message) :
        std::invalid_argument{message}
    {
    }

    /** Token constructor.
     *
     * @param message Error message
     * @param token Token that caused the error
     */
    explicit parse_exception(const string& message,
                             const parsing::token_type& token) :
        std::invalid_argument{message + ": " + parsing::to_string(token)}
    {
    }

    /** Token list constructor.
     *
     * @param message Error message
     * @param tokens Tokens that caused the error
     */
    explicit parse_exception(const string& message,
                             const parsing::token_list& tokens) :
        std::invalid_argument{message + ": " + parsing::to_string(tokens)}
    {
    }

    /** Token list constructor.
     *
     * @param message Error message
     * @param tokens Tokens that caused the error
     */
    explicit parse_exception(const string& message,
                             const vector<parsing::token_type>& tokens) :
        std::invalid_argument{message + ": " + parsing::to_string(tokens)}
    {
    }

    virtual ~parse_exception() override = default;
};
}  // namespace arg_router
