// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/basic_types.hpp"
#include "arg_router/parsing/token_type.hpp"

#include <exception>

namespace arg_router
{
/** An exception that represents a parsing failure.
 *
 * @note Because the standard library exception types that accept a <TT>std::basic_string</TT> in
 * their constructor (e.g. <TT>std::logic_error</TT>) do not have an allocator template parameter,
 * we cannot use them even though they would be easier in this circumstance.  Because of this we
 * have to carry our own string member which may throw upon copying, which means that
 * @em technically this exception type isn't <TT>std::exception</TT> compatible as the copy
 * constructor cannot be marked as <TT>nothrow</TT>
 */
class parse_exception : public std::exception
{
public:
    /** Message-only constructor.
     *
     * @param message Error message
     */
    explicit parse_exception(string message) noexcept : message_{std::move(message)} {}

    /** Token constructor.
     *
     * @param message Error message
     * @param token Token that caused the error
     */
    parse_exception(const string& message, const parsing::token_type& token) :
        message_{message + ": " + parsing::to_string(token)}
    {
    }

    /** Token list constructor.
     *
     * @param message Error message
     * @param tokens Tokens that caused the error
     */
    parse_exception(const string& message, const vector<parsing::token_type>& tokens) :
        message_{message + ": " + parsing::to_string(tokens)}
    {
    }

    ~parse_exception() override = default;

    const char* what() const noexcept override { return message_.data(); }

private:
    string message_;
};
}  // namespace arg_router
