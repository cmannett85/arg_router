#pragma once

#include "arg_router/config.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

#include <deque>

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
    token_type(prefix_type p, std::string n) : prefix{p}, name{std::move(n)} {}

    /** Short from name constructor.
     *
     * @param n Token name, stripped of prefix (if any)
     */
    token_type(char n) : prefix{prefix_type::SHORT}, name{n} {}

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

    prefix_type prefix;  ///< Prefix type
    std::string name;    ///< Token name, stripped of prefix (if any)
};

/** Creates a string representation of @a token, it effectively recreates the
 * original token on the command line.
 * 
 * @param token Token to convert
 * @return String representation of @a token
 */
std::string to_string(const token_type& token);

/** List of tokens. */
using token_list = std::deque<token_type>;

/** Analyse @a token and return a pair consisting of the prefix type and
 * @a token stripped of the token.
 *
 * @param token Token to analyse
 * @return Token type
 */
token_type get_token_type(std::string_view token);

/** Takes the main function arguments and creates a token_list from it.
 *
 * This function will ignore the first argument (program name) and expand out
 * any collapsed short form arguments.
 * @param argc Argument count
 * @param argv Argument string array
 * @return Token list
 */
token_list expand_arguments(int argc, const char* argv[]);

/** Result type when attempting to match a token. */
struct match_result {
    /** A nice label for the matching state. */
    enum match_type : std::uint8_t {
        NO_MATCH,  ///< Token has not matched
        MATCH      ///< Token has matched
    };

    /** A nice label for the requires-a-parse state. */
    enum argument_type : std::uint8_t {
        HAS_NO_ARGUMENT,  ///< No arguments are expected to follow
        HAS_ARGUMENT,     ///< One or more arguments is expected to follow
    };

    /** Constructor
     *
     * @param m Match state
     * @param a Argument state
     */
    constexpr match_result(match_type m = NO_MATCH,
                           argument_type a = HAS_NO_ARGUMENT) :
        matched{m},
        has_argument{a}
    {
    }

    /** Equality operator.
     *
     * @param other Instance to compare against
     * @return True if equal
     */
    bool operator==(const match_result& other) const
    {
        return matched == other.matched && has_argument == other.has_argument;
    }

    /** Inequality operator.
     *
     * @param other Instance to compare against
     * @return True if not equal
     */
    bool operator!=(const match_result& other) const
    {
        return !(*this == other);
    }

    match_type matched;          ///< Match state
    argument_type has_argument;  ///< Argument state
};

/** Creates a string representation of @a mr.
 * 
 * @param mr match_result to convert
 * @return String representation of @a mr
 */
std::string to_string(match_result mr);

/** Can be used by traits::is_detected to determine if a type has a long name.
 *
 * @tparam T Type to query
 */
template <typename T>
using has_long_name_checker = decltype(T::long_name());

/** Can be used by traits::is_detected to determine if a type has a short name.
 *
 * @tparam T Type to query
 */
template <typename T>
using has_short_name_checker = decltype(T::short_name());

/** Can be used by traits::is_detected to determine if a type has a
 * match(std::string_view) static method.
 *
 * @tparam T Type to query
 */
template <typename T>
using has_match_checker =
    decltype(std::declval<const T&>().match(std::declval<const token_type&>()));

/** The standard implementation of the match method.
 *
 * @tparam T Type to implement the method for
 * @param token The token to match against
 * @return Match result
 */
template <typename T>
match_result default_match(const token_type& token)
{
    if constexpr (traits::is_detected_v<has_long_name_checker, T>) {
        if ((token.prefix == prefix_type::LONG) &&
            (token.name == T::long_name())) {
            return {match_result::MATCH, match_result::HAS_NO_ARGUMENT};
        }
    }
    if constexpr (traits::is_detected_v<has_short_name_checker, T>) {
        if ((token.prefix == prefix_type::SHORT) &&
            (token.name.front() == T::short_name())) {
            return {match_result::MATCH, match_result::HAS_NO_ARGUMENT};
        }
    }

    return {match_result::NO_MATCH, match_result::HAS_NO_ARGUMENT};
}

/** Provides a tuple that can be passed to a router implementation as a part of
 * a <TT>std::apply</TT> call.
 *
 * @tparam Child Child type build for
 */
template <typename Child, typename = void>
struct build_router_args;

template <typename Child>
struct build_router_args<
    Child,
    std::enable_if_t<is_tree_node_v<Child> &&
                     (std::tuple_size_v<typename Child::children_type> > 0)>> {
    using type = boost::mp11::mp_transform<traits::get_value_type,
                                           typename Child::children_type>;
};

template <typename Child>
struct build_router_args<
    Child,
    std::enable_if_t<is_tree_node_v<Child> &&
                     (std::tuple_size_v<typename Child::children_type> == 0)>> {
    using type = std::tuple<typename Child::value_type>;
};

template <typename Child>
using build_router_args_t = typename build_router_args<Child>::type;

/** Visitation pattern to find a named child.
 *
 * Visitor needs to be equivalent to:
 * @code
 * template <typename Child>
 * operator()(std::size_t i, Child&&, match_result) {}
 * @endcode
 * <TT>i</TT> is the index into the children tuple.  If child has a
 * match(std::string_view) method then the visitor is called if a match is
 * found.  If it doesn't have a match method and the prefix is prefix_type::NONE
 * then every child encountered that fits that description will have @a visitor
 * called on it - in that circumstance match_result::matched will be
 * match_result::NO_MATCH.
 * 
 * @tparam ChildrenTuple Tuple of child types
 * @tparam Fn Visitor Callable type
 * @param token Token
 * @param children Children tuple instance
 * @param visitor Visitor callable
 * @return True if a valid child was found
 */
template <typename ChildrenTuple, typename Fn>
bool visit_child(const token_type& token,
                 const ChildrenTuple& children,
                 Fn&& visitor)
{
    // Iterate over the children and use the prefix type to use the correct
    // name method
    auto found_child = false;
    utility::tuple_iterator(
        [&](auto i, auto&& child) {
            using child_type = std::decay_t<decltype(child)>;

            if constexpr (traits::is_detected_v<parsing::has_match_checker,
                                                child_type>) {
                const auto result = child.match(token);
                if (result.matched) {
                    visitor(i, child, result);
                    found_child = true;
                }
            } else if (token.prefix == prefix_type::NONE) {
                // A positional arg type will always accept a token
                visitor(
                    i,
                    child,
                    parsing::match_result{parsing::match_result::NO_MATCH,
                                          parsing::match_result::HAS_ARGUMENT});
                found_child = true;
            }

            // No need to check that it has matched again, as the compiler
            // has already done that via a rule
        },
        children);

    return found_child;
}
}  // namespace parsing
}  // namespace arg_router
