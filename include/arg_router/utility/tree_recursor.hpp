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

template <typename Visitor,
          typename SkipFn,
          typename Current,
          typename... Parents>
constexpr void tree_recursor_impl()
{
    if constexpr (!SkipFn::template fn<Current, Parents...>()) {
        if constexpr (is_tree_node_v<Current>) {
            using children_and_policies = boost::mp11::mp_append<  //
                typename Current::children_type,
                typename Current::policies_type>;

            // Recurse into the types
            tuple_type_iterator<children_and_policies>([](auto i) {
                using type = std::tuple_element_t<i, children_and_policies>;
                tree_recursor_impl<Visitor,
                                   SkipFn,
                                   type,
                                   Current,
                                   Parents...>();
            });
        }

        Visitor::template fn<Current, Parents...>();
    }
}

template <template <typename...> typename Visitor,
          typename Current,
          typename... Parents>
struct tree_type_recursor_impl {
    template <typename ChildOrPolicy>
    using recurse = typename tree_type_recursor_impl<Visitor,
                                                     ChildOrPolicy,
                                                     Current,
                                                     Parents...>::type;

    template <typename T>
    using get_children_type = typename T::children_type;
    template <typename T>
    using get_policies_type = typename T::policies_type;

    using children_type = boost::mp11::mp_eval_if_c<!is_tree_node_v<Current>,
                                                    std::tuple<>,
                                                    get_children_type,
                                                    Current>;
    using policies_type = boost::mp11::mp_eval_if_c<!is_tree_node_v<Current>,
                                                    std::tuple<>,
                                                    get_policies_type,
                                                    Current>;

    using type = boost::mp11::mp_eval_if_c<
        !is_tree_node_v<Current>,
        std::tuple<typename Visitor<Current, Parents...>::type>,
        boost::mp11::mp_push_back,
        boost::mp11::mp_flatten<boost::mp11::mp_transform<
            recurse,
            boost::mp11::mp_append<children_type, policies_type>>>,
        typename Visitor<Current, Parents...>::type>;
};
}  // namespace detail

/** Depth-first recurse through the parse tree, calling @a Visitor on every
 * tree_node and policy.
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
 * @tparam Visitor Visitor object type
 * @tparam Root Start object type in the parse tree
 * @return void
 */
template <typename Visitor, typename Root>
constexpr void tree_recursor()
{
    detail::tree_recursor_impl<Visitor, detail::always_false, Root>();
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
 * @tparam Visitor Visitor object type
 * @tparam SkipFn Functional object type to skip recursion on current node
 * @tparam Root Start object type in the parse tree
 * @return void
 */
template <typename Visitor, typename SkipFn, typename Root>
constexpr void tree_recursor()
{
    detail::tree_recursor_impl<Visitor, SkipFn, Root>();
}

/** Similar in principle to tree_recursor(), but allows for a type to built up
 * from the visitor as it recurses.
 *
 * @a Fn must be a template template type like below:
 * @code
 * template <typename Current, typename... Parents>
 * struct my_fn_t {
 *     using type = // Some result type of the visitation
 * };
 * @endcode
 * <TT>my_fn_t<...>::type</TT> of all the nodes and policies is concatenated
 * into a <TT>std::tuple</TT> available in the type member.
 * @tparam Visitor Visitor object type
 * @tparam Root Start object type in the parse tree
 */
template <template <typename...> typename Visitor, typename Root>
struct tree_type_recursor {
    /** Tuple of all the @a Visitor result types. */
    using type = typename detail::tree_type_recursor_impl<Visitor, Root>::type;
};

/** Helper alias for tree_type_recursor.
 *
 * @tparam Visitor Visitor object type
 * @tparam Root Start object type in the parse tree
 */
template <template <typename...> typename Visitor, typename Root>
using tree_type_recursor_t = typename tree_type_recursor<Visitor, Root>::type;
}  // namespace utility
}  // namespace arg_router
