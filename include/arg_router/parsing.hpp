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
enum class prefix_type { LONG, SHORT, NONE };

/** Analyse @a token and return a pair consisting of the prefix type and
 * @a token stripped of the token.
 *
 * @param token Token to analyse
 * @return Prefix type and prefix stripped @a token
 */
constexpr std::pair<prefix_type, std::string_view>  //
get_prefix_type(std::string_view token)
{
    if (token.substr(0, config::long_prefix.size()) == config::long_prefix) {
        return {prefix_type::LONG, token.substr(config::long_prefix.size())};
    } else if (token.substr(0, config::short_prefix.size()) ==
               config::short_prefix) {
        return {prefix_type::SHORT, token.substr(config::short_prefix.size())};
    } else {
        return {prefix_type::NONE, token};
    }
}

inline std::deque<std::string> expand_arguments(int argc, const char* argv[])
{
    auto result = std::deque<std::string>{};

    for (auto i = 0; i < argc; ++i) {
        const auto token = std::string_view{argv[i]};
        const auto [prefix, stripped] = parsing::get_prefix_type(token);

        if (prefix == parsing::prefix_type::SHORT && token.size() > 2) {
            for (auto c : stripped) {
                result.push_back(std::string{"-"} + c);
            }
        } else {
            result.push_back(std::string{token});
        }
    }

    return result;
}

/** Result type when attempting to match a token. */
struct match_result {
    /** A nice label for the matching state. */
    enum match_type {
        NO_MATCH,  ///< Token has not matched
        MATCH      ///< Token has matched
    };

    /** A nice label for the requires-a-parse state. */
    enum argument_type {
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

    match_type matched;          ///< Match state
    argument_type has_argument;  ///< Argument state
};

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
using has_match_checker = decltype(T::match(std::declval<std::string_view>()));

/** The standard implementation of the match(std::string_view) method.
 *
 * This checks if a long_name() method exists and attempts to match @a token
 * against it, otherwise it tries the same with the short_name() method.
 * @tparam T Type to implement the method for
 * @param token The token to match against, must have had its prefix removed!
 * @return Match result
 */
template <typename T>
constexpr match_result default_match(std::string_view token)
{
    if constexpr (traits::is_detected_v<has_long_name_checker, T>) {
        if (token == T::long_name()) {
            return {match_result::MATCH, match_result::HAS_NO_ARGUMENT};
        }
    }
    if constexpr (traits::is_detected_v<has_short_name_checker, T>) {
        if ((token.size() == 1) && (token.front() == T::short_name())) {
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
 * template <typename Child
 * operator()(std::size_t i, Child&&, match_result) {}
 * @endcode
 * <TT>i</TT> is the index into the children tuple.  If child has a
 * match(std::string_view) method then the visitor is called if a match is
 * found.  If it doesn't have a match method and the prefix is prefix_type::NONE
 * then every child encountered that fits that description will have @a visitor
 * called on it - in that circumstance match_result::matched will be
 * match_result::NO_MATCH.
 * 
 * @tparam PType Prefix type
 * @tparam ChildrenTuple Tuple of child types
 * @tparam Fn Visitor Callable type
 * @param children Children tuple instance
 * @param name Token to search for in long or short form
 * @param visitor Visitor callable
 * @return True if a valid child was found
 */
template <prefix_type PType, typename ChildrenTuple, typename Fn>
constexpr bool visit_child(const ChildrenTuple& children,
                           std::string_view name,
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
                const auto result = child.match(name);
                if (result.matched) {
                    visitor(i, child, result);
                    found_child = true;
                }
            } else if constexpr (PType == prefix_type::NONE) {
                // A positional arg type will always accept a token
                visitor(
                    i,
                    child,
                    parsing::match_result{parsing::match_result::NO_MATCH,
                                          parsing::match_result::HAS_ARGUMENT});
                found_child = true;
            }

            // No need to check that it has matched again, as the compiler
            // has already done that
        },
        children);

    return found_child;
}
}  // namespace parsing
}  // namespace arg_router
