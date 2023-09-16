// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <utility>
#include <vector>

namespace arg_router
{
template <typename... Params>
class tree_node;

/** Evaulates to true if @a T derives from a tree_node specialisation.
 *
 * @tparam T Type to test
 */
template <typename T>
struct is_tree_node {
private:
    template <typename... Ts>
    consteval static std::true_type test(const tree_node<Ts...>*);

    consteval static std::false_type test(...);

public:
    // NOLINTNEXTLINE(hicpp-vararg)
    constexpr static bool value = decltype(test(std::declval<T*>()))::value;
};

/** Helper variable for is_tree_node.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool is_tree_node_v = is_tree_node<T>::value;
}  // namespace arg_router
