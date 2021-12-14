#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace arg_router
{
/** Namespace containing types and functions to aid parsing. */
namespace parsing
{
/** Enum for the prefix type on a token. */
enum class prefix_type : std::uint8_t { LONG, SHORT, NONE };

/** Creates a string version of @a prefix.
 *
 * This uses config::long_prefix and config::short_prefix.
 * @param prefix Prefix type to convert
 * @return String version of @a prefix
 */
std::string_view to_string(prefix_type prefix);

/** Pair-like structure carrying the token's prefix type and the token itself
 * (stripped of prefix).
 */
struct token_type {
    /** Long form name constructor.
     *
     * @param p Prefix type
     * @param n Token name, stripped of prefix (if any)
     */
    constexpr token_type(prefix_type p, std::string_view n) : prefix{p}, name{n}
    {
    }

    /** Equality operator.
     *
     * @param other Instance to compare against
     * @return True if equal
     */
    bool operator==(const token_type& other) const
    {
        return prefix == other.prefix && name == other.name;
    }

    /** Inequality operator.
     *
     * @param other Instance to compare against
     * @return True if not equal
     */
    bool operator!=(const token_type& other) const { return !(*this == other); }

    prefix_type prefix;     ///< Prefix type
    std::string_view name;  ///< Token name, stripped of prefix (if any)
};

/** Creates a string representation of @a token, it effectively recreates the
 * original token on the command line.
 * 
 * @param token Token to convert
 * @return String representation of @a token
 */
std::string to_string(const token_type& token);

/** List of tokens. */
using token_list = std::vector<token_type>;

/** Creates a string representation of @a tokens.
 * 
 * @param tokens Tokens to convert
 * @return String representation of @a tokens
 */
std::string to_string(const token_list& tokens);

/** Analyse @a token and return a pair consisting of the prefix type and
 * @a token stripped of the token.
 *
 * @param token Token to analyse
 * @return Token type
 */
token_type get_token_type(std::string_view token);
}  // namespace parsing
}  // namespace arg_router
