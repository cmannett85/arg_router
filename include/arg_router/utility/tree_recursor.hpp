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
struct always_false {
    template <typename...>
    constexpr static bool fn()
    {
        return false;
    }
};

template <typename Fn, typename SkipFn, typename Current, typename... Parents>
constexpr void tree_recursor_impl()
{
    if constexpr (!SkipFn::template fn<Current, Parents...>()) {
        if constexpr (is_tree_node_v<Current>) {
            using children_and_policies = boost::mp11::mp_append<  //
                typename Current::children_type,
                typename Current::policies_type>;

            // Recurse into the types
            tuple_type_iterator<children_and_policies>(
                [](auto /*i*/, auto ptr) {
                    using type = std::remove_pointer_t<decltype(ptr)>;
                    tree_recursor_impl<Fn, SkipFn, type, Current, Parents...>();
                });
        }

        Fn::template fn<Current, Parents...>();
    }
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
    detail::tree_recursor_impl<Fn, detail::always_false, Root>();
}

/** Specialisation of tree_recursor() where a skip function can be used to
 * ignore subtrees.
 *
 * @a SkipFn must be object with the same method signatue as below:
 * @code
 * struct my_skip_fn {
 *     template <typename Current, typename... Parents>
 *     constexpr static bool fn()
 *     {
 *         ...
 *         return condition_met;
 *     }
 * };
 * @endcode
 * If the return value is true, then the <TT>Current</TT> node and any subtree
 * below it are skipped i.e. @a Fn is not called.
 * 
 * @tparam Fn Functional object type
 * @tparam SkipFn Functional object type to skip recursion on current node
 * @tparam Root Start object type in the parse tree
 * @return void
 */
template <typename Fn, typename SkipFn, typename Root>
constexpr void tree_recursor()
{
    detail::tree_recursor_impl<Fn, SkipFn, Root>();
}
}  // namespace utility
}  // namespace arg_router
