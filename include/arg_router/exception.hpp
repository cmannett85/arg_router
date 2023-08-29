// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/basic_types.hpp"
#include "arg_router/traits.hpp"
#include "arg_router/utility/exception_formatter.hpp"

#include <exception>

namespace arg_router
{
/** Error code type.
 *
 * These are used as keys into an error string translation table, either a default internal en_GB
 * one, or user provided.
 *
 * It can be extended by node or policy developers, but to avoid clashes please start at 1000 or
 * higher, eg:
 * @code
 * enum class my_error_code : std::size_t {
 *     some_error = 1000,
 *     ...
 * };
 * // Or if there is only one, this may be more desirable:
 * constexpr auto some_error = std::size_t{1000};
 *
 * some_ec_sink(static_cast<ar::error_code>(some_error));
 * @endcode
 */
enum class error_code : std::size_t {
    // Common
    unknown_argument = 0,  ///< A token was passed on the command line that cannot be consumed
                           ///< by any node
    unhandled_arguments,   ///< All tokens were matched to nodes, but not all tokens were processed
                           ///< by them
    argument_has_already_been_set,  ///< A token has been matched to a node that has already
                                    ///< accepted one or more tokens and cannot accept any more
    failed_to_parse,                ///< A value token could not be converted into its target value
    no_arguments_passed,        ///< No tokens were passed to the root when one or more nodes were
                                ///< expecting them
    minimum_value_not_reached,  ///< Parsed value is below the minimum
    maximum_value_exceeded,     ///< Parsed value is above the maximum
    minimum_count_not_reached,  ///< Minimum number of value tokens for node not reached
    maximum_count_exceeded,     ///< Maximum number of value tokens exceeded
    unknown_argument_with_suggestion,  ///< Same as unknown_argument, but also gives the nearest
                                       ///< node name as a suggestion

    // Builtin node specific
    mode_requires_arguments = 100,  ///< The mode only consists of child modes, but there  are no
                                    ///< more tokens to consume
    missing_required_argument,      ///< A node has been marked as required but no token matches it
    too_few_values_for_alias,       ///< An alias for a fixed sized node has not been given enough
                                    ///< value tokens
    dependent_argument_missing,     ///< A node has dependency, but that dependency is not specified
                                    ///< on the command line
    one_of_selected_type_mismatch,  ///< A one_of child node has already been selected, and it does
                                    ///< not match the current selection
    missing_value_separator,        ///< A token that expects a value separator character was not
                                    ///< given one on the command line
};

/** Default error code translations in en_GB.
 */
struct default_error_code_translations {
    using error_code_translations = std::tuple<
        std::pair<traits::integral_constant<error_code::unknown_argument>, str<"Unknown argument">>,
        std::pair<traits::integral_constant<error_code::unhandled_arguments>,
                  str<"Unhandled arguments">>,
        std::pair<traits::integral_constant<error_code::argument_has_already_been_set>,
                  str<"Argument has already been set">>,
        std::pair<traits::integral_constant<error_code::failed_to_parse>, str<"Failed to parse">>,
        std::pair<traits::integral_constant<error_code::no_arguments_passed>,
                  str<"No arguments passed">>,
        std::pair<traits::integral_constant<error_code::minimum_value_not_reached>,
                  str<"Minimum value not reached">>,
        std::pair<traits::integral_constant<error_code::maximum_value_exceeded>,
                  str<"Maximum value exceeded">>,
        std::pair<traits::integral_constant<error_code::minimum_count_not_reached>,
                  str<"Minimum count not reached">>,
        std::pair<traits::integral_constant<error_code::maximum_count_exceeded>,
                  str<"Maximum count exceeded">>,
        std::pair<traits::integral_constant<error_code::unknown_argument_with_suggestion>,
                  str<"Unknown argument: {}. Did you mean { }?">>,
        std::pair<traits::integral_constant<error_code::mode_requires_arguments>,
                  str<"Mode requires arguments">>,
        std::pair<traits::integral_constant<error_code::missing_required_argument>,
                  str<"Missing required argument">>,
        std::pair<traits::integral_constant<error_code::too_few_values_for_alias>,
                  str<"Too few values for alias">>,
        std::pair<traits::integral_constant<error_code::dependent_argument_missing>,
                  str<"Dependent argument missing (needs to be before the "
                      "requiring token on the command line)">>,
        std::pair<traits::integral_constant<error_code::one_of_selected_type_mismatch>,
                  str<"Only one argument from a \"One Of\" can be used at once">>,
        std::pair<traits::integral_constant<error_code::missing_value_separator>,
                  str<"Expected a value separator">>>;
};

/** Used internally by the library (and node developers) to indicate failure.
 *
 * Rather than carry an error message, this exception type carries an error code that then maps
 * to a translated message at runtime.  The translated message is then put into a parse_exception
 * and re-thrown in the root, so the user should never this exception type.
 */
class multi_lang_exception : public std::exception
{
public:
    /** Error code constructor.
     *
     * @param ec Error code
     */
    explicit multi_lang_exception(error_code ec) noexcept : ec_{ec} {}

    /** Token constructor.
     *
     * @param ec Error code
     * @param token Token that caused the error
     */
    multi_lang_exception(error_code ec, const parsing::token_type& token) : ec_{ec}, tokens_{token}
    {
    }

    /** Token list constructor.
     *
     * @param ec Error code
     * @param tokens Tokens that caused the error
     */
    multi_lang_exception(error_code ec, std::vector<parsing::token_type> tokens) noexcept :
        ec_{ec}, tokens_(std::move(tokens))
    {
    }

    /** @return Error code
     */
    [[nodiscard]] error_code ec() const noexcept { return ec_; }

    /** @return Token list
     */
    [[nodiscard]] const std::vector<parsing::token_type>& tokens() const noexcept
    {
        return tokens_;
    }

private:
    error_code ec_;
    std::vector<parsing::token_type> tokens_;
};

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
    /** Token list constructor.
     *
     * @param message Error message
     * @param tokens Tokens that caused the error
     */
    explicit parse_exception(const std::string& message,
                             const std::vector<parsing::token_type>& tokens = {}) :
        message_{message + (tokens.empty() ? "" : ": " + parsing::to_string(tokens))}
    {
    }

    /** Token constructor.
     *
     * @param message Error message
     * @param token Token that caused the error
     */
    parse_exception(const std::string& message, const parsing::token_type& token) :
        message_{message + ": " + parsing::to_string(token)}
    {
    }

    /** Formatter constructor.
     *
     * @tparam S Compile-time string type
     * @param cts Compile-time string instance
     * @param tokens Tokens to pass to the formatter's placeholders
     */
    template <typename S>
    explicit parse_exception(utility::exception_formatter<S> cts,
                             const std::vector<parsing::token_type>& tokens = {}) :
        message_{cts.format(tokens)}
    {
    }

    [[nodiscard]] const char* what() const noexcept override { return message_.data(); }

private:
    std::string message_;
};
}  // namespace arg_router
