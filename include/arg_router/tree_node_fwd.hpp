#pragma once

#include "arg_router/policy/policy.hpp"

namespace arg_router
{
template <typename... Params>
class tree_node;

/** Evaulates to true if @a T is a tree_node specialisation.
 *
 * @tparam T Type to test
 */
template <typename T>
struct is_tree_node {
private:
    template <typename... Ts>
    static constexpr std::true_type test(const tree_node<Ts...>*);

    static constexpr std::false_type test(...);

public:
    constexpr static bool value = decltype(test(std::declval<T*>()))::value;
};

/** Helper variable for is_tree_node.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool is_tree_node_v = is_tree_node<T>::value;
}  // namespace arg_router
