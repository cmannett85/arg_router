#pragma once

#include "arg_router/policy/policy.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

namespace arg_router
{
namespace utility
{
namespace detail
{
template <typename Fn, typename Current, typename... Parents>
constexpr void tree_recursor_impl()
{
    if constexpr (is_tree_node_v<Current>) {
        // Iterate over the children (if any) and recurse into them
        tuple_type_iterator<typename Current::children_type>(
            [](auto /*i*/, auto ptr) {
                using child_type = std::remove_pointer_t<decltype(ptr)>;
                tree_recursor_impl<Fn, child_type, Current, Parents...>();
            });

        // And the same for policies
        tuple_type_iterator<typename Current::policies_type>(
            [](auto /*i*/, auto ptr) {
                using policy_type = std::remove_pointer_t<decltype(ptr)>;
                tree_recursor_impl<Fn, policy_type, Current, Parents...>();
            });
    }

    Fn::template fn<Current, Parents...>();
}
}  // namespace detail

/** Depth-first recurse through the parse tree, calling @a Fn on every tree_node
 * and policy.
 *
 * @a Fn must be object with the same method signatue as below:
 * @code
 * struct my_fn {
 *     template <typename Current, typename... Parents>
 *     constexpr static void fn()
 *     {
 *         ...
 *     }
 * };
 * @endcode
 * Where <TT>Current</TT> is the current type from the tree, from the root down
 * to policy level, and <TT>Parents</TT> is a pack of ancestors in increasing
 * generation from <TT>Current</TT>.  The last in the pack is always the root
 * unless <TT>Current</TT> is itself the root.
 * @tparam Fn Functional object type
 * @tparam Root Start object type in the parse tree
 * @return void
 */
template <typename Fn, typename Root>
constexpr void tree_recursor()
{
    detail::tree_recursor_impl<Fn, Root>();
}
}  // namespace utility
}  // namespace arg_router
