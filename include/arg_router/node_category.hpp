#pragma once

#include <arg_router/policy/has_contiguous_value_tokens.hpp>
#include <arg_router/traits.hpp>
#include <arg_router/tree_node_fwd.hpp>

namespace arg_router
{
/** Trait-like groupings of tree node types.
 *
 * arg_router uses duck-typing via traits to define node properties, so root_t
 * uses specific sets of duck-typing tests to detect certain categories of
 * nodes.  Those tests are defined here.
 */
namespace node_category
{
/** Determines if @a T has <TT>minimum_count()</TT> and <TT>maximum_count()</TT>
 * that are equal to @a N.
 *
 * @tparam T Type to query
 * @tparam N Value to compare against
 */
template <typename T, auto N>
struct has_fixed_count {
    constexpr static bool value = []() {
        if constexpr (traits::has_minimum_count_method_v<T> &&
                      traits::has_maximum_count_method_v<T>) {
            return (T::minimum_count() == N) && (T::maximum_count() == N);
        }

        return false;
    }();
};

/** Helper variable for has_fixed_count.
 *
 * @tparam T Type to query
 * @tparam N Value to compare against
 */
template <typename T, auto N>
constexpr bool has_fixed_count_v = has_fixed_count<T, N>::value;

/** Determines if @a T has neither <TT>minimum_count()</TT> or
 * <TT>maximum_count()</TT>
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_no_count {
    constexpr static bool value = !traits::has_minimum_count_method_v<T> &&  //
                                  !traits::has_maximum_count_method_v<T>;
};

/** Helper variable for has_no_count.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_no_count_v = has_no_count<T>::value;

/** Determines if a node is has a long name and/or a short name.
 *
 * @tparam T Type to query
 */
template <typename T>
struct is_named {
    constexpr static bool value = traits::has_long_name_method_v<T> ||  //
                                  traits::has_short_name_method_v<T>;
};

/** Helper variable for is_named.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool is_named_v = is_named<T>::value;

/** Returns true if @a T is a tree_node has one or more children.
 *
 * @tparam T Type to query
 */
template <typename T>
struct has_children {
    constexpr static bool value = []() {
        if constexpr (is_tree_node_v<T>) {
            return std::tuple_size_v<typename T::children_type> > 0;
        }

        return false;
    }();
};

/** Helper variable for has_children.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool has_children_v = has_children<T>::value;

/** True if @a T is a generic flag-like tree node.
 *
 * This will return true for both normal flag and counting flag type.
 * @tparam T Type to query
 */
template <typename T>
struct is_generic_flag_like {
    constexpr static bool value = is_tree_node_v<T> &&   //
                                  !has_children_v<T> &&  //
                                  is_named_v<T> &&
                                  traits::has_match_method_v<T> &&
                                  !policy::has_value_tokens_v<T>;
};

/** Helper variable for is_generic_flag_like.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool is_generic_flag_like_v = is_generic_flag_like<T>::value;

/** True if @a T is a flag-like tree node.
 *
 * @tparam T Type to query
 */
template <typename T>
struct is_flag_like {
    constexpr static bool value =
        is_generic_flag_like_v<T> && has_fixed_count_v<T, 0>;
};

/** Helper variable for is_flag_like.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool is_flag_like_v = is_flag_like<T>::value;

/** True if @a T is a counting flag-like tree node.
 *
 * @tparam T Type to query
 */
template <typename T>
struct is_counting_flag_like {
    constexpr static bool value =
        is_generic_flag_like_v<T> && !has_fixed_count_v<T, 0>;
};

/** Helper variable for is_counting_flag_like.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool is_counting_flag_like_v = is_counting_flag_like<T>::value;

/** True if @a T is an arg-like tree node.
 *
 * @tparam T Type to query
 */
template <typename T>
struct is_arg_like {
    constexpr static bool value = is_tree_node_v<T> &&  //
                                  is_named_v<T> &&      //
                                  traits::has_match_method_v<T> &&
                                  policy::has_value_tokens_v<T> &&
                                  has_fixed_count_v<T, 1>;
};

/** Helper variable for is_arg_like.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool is_arg_like_v = is_arg_like<T>::value;

/** True if @a T is a positional arg-like tree node.
 *
 * @tparam T Type to query
 */
template <typename T>
struct is_positional_arg_like {
    constexpr static bool value =
        is_tree_node_v<T> &&  //
        is_named_v<T> &&      //
        !traits::has_match_method_v<T> &&
        policy::has_contiguous_value_tokens_v<T> &&  //
        !has_fixed_count_v<T, 0>;
};

/** Helper variable for is_positional_arg_like.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool is_positional_arg_like_v = is_positional_arg_like<T>::value;

/** True if @a T is a generic mode-like tree node.
 *
 * @tparam T Type to query
 */
template <typename T>
struct is_generic_mode_like {
    constexpr static bool value = is_tree_node_v<T> &&  //
                                  traits::has_match_method_v<T> &&
                                  !policy::has_value_tokens_v<T> &&  //
                                  has_no_count_v<T>;
};

/** Helper variable for is_generic_mode_like.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool is_generic_mode_like_v = is_generic_mode_like<T>::value;

/** True if @a T is an anonymous mode-like tree node.
 *
 * @tparam T Type to query
 */
template <typename T>
struct is_anonymous_mode_like {
    constexpr static bool value = is_generic_mode_like_v<T> && !is_named_v<T>;
};

/** Helper variable for is_anonymous_mode_like.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool is_anonymous_mode_like_v = is_anonymous_mode_like<T>::value;

/** True if @a T is a named mode-like tree node.
 *
 * @tparam T Type to query
 */
template <typename T>
struct is_named_mode_like {
    constexpr static bool value = is_generic_mode_like_v<T> && is_named_v<T>;
};

/** Helper variable for is_named_mode_like.
 *
 * @tparam T Type to query
 */
template <typename T>
constexpr bool is_named_mode_like_v = is_named_mode_like<T>::value;
}  // namespace node_category
}  // namespace arg_router
