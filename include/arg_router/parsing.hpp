#pragma once

#include "arg_router/config.hpp"
#include "arg_router/exception.hpp"
#include "arg_router/policy/has_value_tokens.hpp"
#include "arg_router/token_type.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/string_view_ops.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/core/ignore_unused.hpp>

#include <bitset>
#include <charconv>

namespace arg_router
{
/** Namespace containing types and functions to aid parsing. */
namespace parsing
{
/** Takes the main function arguments and creates a token_list from it.
 *
 * This function will ignore the first argument (program name) and expand out
 * any collapsed short form arguments.
 * @param argc Argument count
 * @param argv Argument string array
 * @return Token list
 */
token_list expand_arguments(int argc, const char* argv[]);

/** The standard implementation of the match method.
 *
 * @tparam T Type to implement the method for
 * @param token The token to match against
 * @return True if token matches the long or short form name
 */
template <typename T>
bool default_match(const token_type& token)
{
    if constexpr (traits::has_long_name_method_v<T>) {
        if ((token.prefix == prefix_type::LONG) &&
            (token.name == T::long_name())) {
            return true;
        }
    }
    if constexpr (traits::has_short_name_method_v<T>) {
        if ((token.prefix == prefix_type::SHORT) &&
            (token.name == T::short_name())) {
            return true;
        }
    }

    return false;
}

/** Returns the node name, the long form name is preferred.
 *
 * @note If @a Node does not have a long or short name, it is a compliation
 * failure
 * @tparam Node Node type
 * @return Node name
 */
template <typename Node>
constexpr std::string_view node_name()
{
    if constexpr (traits::has_long_name_method_v<Node>) {
        return Node::long_name();
    } else if constexpr (traits::has_short_name_method_v<Node>) {
        return Node::short_name();
    } else {
        static_assert(traits::always_false_v<Node>,
                      "Node does not have a name");
    }
}

/** Returns the token_type of @a Node, the long form name is preferred.
 *
 * @note If @a Node does not have a long or short name, it is a compliation
 * failure
 * @tparam Node Node type
 * @return token_type
 */
template <typename Node>
token_type node_token_type()
{
    if constexpr (traits::has_long_name_method_v<Node>) {
        return {prefix_type::LONG, std::string{Node::long_name()}};
    } else if constexpr (traits::has_short_name_method_v<Node>) {
        return {prefix_type::SHORT, std::string{Node::short_name()}};
    } else {
        static_assert(traits::always_false_v<Node>,
                      "Node does not have a name");
    }
}

namespace detail
{
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
}  // namespace detail

/** Provides a tuple that can be used in the root for router arg processing.
 *
 * This is a tuple of either the @a Child's <TT>value_type</TT> or its
 * children's <TT>value_type</TT>s if it has any,
 * wrapped in a <TT>std::optional</TT>.  It is put in a optional so there is a
 * difference between a default constructed argument and one that has not been
 * set by the user.
 * 
 * The use of <TT>std::optional</TT> also allows the <TT>value_type</TT>s to be
 * non-default constructible.
 * @tparam Child Child type build for 
 */
template <typename Child>
using optional_router_args_t =
    boost::mp11::mp_transform<traits::add_optional_t,
                              typename detail::build_router_args<Child>::type>;

/** Visitation pattern to find a named child.
 *
 * Visitor needs to be equivalent to:
 * @code
 * template <typename Child>
 * operator()(std::size_t i, Child&&) {}
 * @endcode
 * <TT>i</TT> is the index into the children tuple.  If child has a
 * match(std::string_view) method then the visitor is called if a match is
 * found.  If it doesn't have a match method and the prefix is prefix_type::NONE
 * then every child encountered that fits that description will have @a visitor
 * called on it.
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
                 Fn visitor)
{
    auto found_child = false;
    utility::tuple_iterator(
        [&](auto i, auto&& child) {
            using child_type = std::decay_t<decltype(child)>;

            auto call_visitor = [&]() {
                visitor(i, child);
                found_child = true;
            };

            if constexpr (traits::has_match_method_v<child_type>) {
                const auto result = child.match(token);
                if (result) {
                    // Skip if already found
                    if (found_child) {
                        return;
                    }

                    call_visitor();
                }
            } else if (token.prefix == prefix_type::NONE &&
                       policy::has_value_tokens_v<child_type>) {
                // A positional arg type will always accept a token
                call_visitor();
            }
        },
        children);

    return found_child;
}

/** Positional arg-aware visitation pattern to find a named child.
 *
 * This is the same as other visit_child implementation except that it uses the
 * already parsed values to determine which positional arg-like child @a visitor
 * should operate on.  The visitor is not called more than once per invocation.
 * 
 * Positional args may accept multiple values, in which case the child that
 * represents it will be used to parse the value, until that positional arg's
 * maximum_count() or count() is reached - at which point the next positional
 * arg is used.
 * @tparam ChildrenTuple Tuple of child types
 * @tparam RouterArgs Tuple of parsed value types to be given to the router
 * policy
 * @tparam Fn Visitor Callable type
 * @param token Token
 * @param children Children tuple instance
 * @param router_args Tuple of parsed <TT>std::optional</TT> wrapped values to
 * be given to the router policy
 * @param visitor Visitor callable
 * @return True if a valid child was found
 */
template <typename ChildrenTuple, typename RouterArgs, typename Fn>
bool visit_child(const token_type& token,
                 const ChildrenTuple& children,
                 const RouterArgs& router_args,
                 Fn visitor)
{
    static_assert(
        std::tuple_size_v<ChildrenTuple> == std::tuple_size_v<RouterArgs>,
        "Number of children must be the same as number of router args");

    auto found_child = false;
    auto wrapped_visitor = [&](auto i, const auto& child) {
        using child_type = std::tuple_element_t<i, ChildrenTuple>;
        using value_type =
            typename std::tuple_element_t<i, RouterArgs>::value_type;

        // Skip if we have already found
        if (found_child) {
            return;
        }

        // Non-positional arg case just forwards onto the original visitor
        if constexpr (traits::has_match_method_v<child_type>) {
            visitor(i, child);
            found_child = true;
        } else {
            if constexpr (traits::has_push_back_method_v<value_type>) {
                // Unpleasant, there's no 'if constexpr' ternary operator though
                constexpr auto max_count = [&]() {
                    if constexpr (traits::has_maximum_count_method_v<
                                      child_type>) {
                        return child_type::maximum_count();
                    }

                    return std::numeric_limits<std::size_t>::max();
                }();

                // Compare the number of values the position arg has already, to
                // check if its maximum has been reached.  If the maximum has
                // been reached then just skip onto the next
                const auto& value = std::get<i>(router_args);
                const auto num_values = value ? std::size(*value) : 0u;
                if (num_values < max_count) {
                    visitor(i, child);
                    found_child = true;
                }
            } else {
                // Positional args support non-container value_types if their
                // min/max/count values are the same.  So use the hit_mask to
                // determine whether or not to skip
                if (!std::get<i>(router_args)) {
                    visitor(i, child);
                    found_child = true;
                }
            }
        }
    };

    visit_child(token, children, wrapped_visitor);
    return found_child;
}
}  // namespace parsing

/** Global parsing struct.
 *
 * If you want to provide custom parsing for an entire @em type, then you should
 * specialise this.  If you want to provide custom parsing for a particular type
 * just for a single argument, it is usually more convenient to use a
 * policy::custom_parser and define the conversion function inline.
 * @tparam T Type to parse @a token into
 * @param token Command line token to parse
 * @return The parsed instance
 * @exception parse_exception Thrown if parsing failed
 */
template <typename T, typename Enable = void>
struct parser {
    constexpr static T parse(std::string_view token)
    {
        boost::ignore_unused(token);
        static_assert(
            traits::always_false_v<T>,
            "No parse function for this type, use a custom_parser policy "
            "or define a parse(std::string_view) specialisation");
    }
};

template <typename T>
struct parser<T, typename std::enable_if_t<std::is_arithmetic_v<T>>> {
    static T parse(std::string_view token)
    {
        using namespace utility::string_view_ops;
        using namespace std::string_literals;

        if (token.front() == '+') {
            token.remove_prefix(1);
        }

        auto value = T{};
        const auto result = std::from_chars(token.begin(), token.end(), value);

        if (result.ec == std::errc{}) {
            return value;
        }

        if (result.ec == std::errc::result_out_of_range) {
            throw parse_exception{"Value out of range for argument: "s + token};
        }

        throw parse_exception{"Failed to parse: "s + token};
    }
};

template <>
struct parser<std::string_view> {
    constexpr static inline std::string_view parse(std::string_view token)
    {
        return token;
    }
};

template <>
struct parser<bool> {
    static bool parse(std::string_view token);
};

// The default vector-like container parser just forwards onto the value_type
// parser, this is because an argument that can be parsed as a complete
// container will need a custom parser.  In other words, this is only used for
// positional arg parsing
template <typename T>
struct parser<T, typename std::enable_if_t<traits::has_push_back_method_v<T>>> {
    static typename T::value_type parse(std::string_view token)
    {
        return parser<typename T::value_type>::parse(token);
    }
};
}  // namespace arg_router
