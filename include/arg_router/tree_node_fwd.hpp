#pragma once

#include <type_traits>

namespace arg_router
{
template <typename... Params>
class tree_node;

/** Evaulates to true if @a T is a tree_node specialisation.
 *
 * @tparam T Type to test
 */
template <typename T>
struct is_tree_node : std::false_type {
};

template <template <typename...> typename T, typename... Args>
struct is_tree_node<T<Args...>> :
    std::is_base_of<tree_node<Args...>, T<Args...>> {
};

/** Helper variable for is_tree_node.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool is_tree_node_v = is_tree_node<T>::value;
}  // namespace arg_router
