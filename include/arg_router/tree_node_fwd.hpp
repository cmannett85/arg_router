// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/utility/dynamic_string_view.hpp"

#include <utility>
#include <vector>

namespace arg_router
{
/** Help data for runtime help collect.
 *
 * arg_router's help system supports compile-time (see @ref help_hdt) and runtime collation via this
 * structure.  The latter is intended to replace the former as it allows for dynamic filtering and
 * other adjustments.
 */
struct runtime_help_data {
    utility::dynamic_string_view label;        //!< Node name
    utility::dynamic_string_view description;  //!< Node description
    std::vector<runtime_help_data> children;   //!< Child node help data
};

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
    constexpr static std::true_type test(const tree_node<Ts...>*);

    constexpr static std::false_type test(...);

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
