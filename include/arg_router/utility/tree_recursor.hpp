// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/tree_node.hpp"
#include "arg_router/utility/tuple_iterator.hpp"

namespace arg_router::utility
{
namespace detail
{
struct always_false {
    template <typename...>
    [[nodiscard]] constexpr static bool fn() noexcept
    {
        return false;
    }
};

template <typename Visitor, typename Current, typename... Parents>
constexpr void tree_recursor_impl(Visitor& visitor,
                                  const Current& current,
                                  const Parents&... parents)
{
    visitor(current, parents...);

    tuple_iterator(
        [&](auto /*i*/, const auto& child) {
            tree_recursor_impl(visitor, child, current, parents...);
        },
        current.children());
}

template <typename Visitor, typename SkipFn, typename Current, typename... Parents>
constexpr void tree_type_recursor_impl()
{
    if constexpr (!SkipFn::template fn<Current, Parents...>()) {
        Visitor::template fn<Current, Parents...>();

        if constexpr (is_tree_node_v<Current>) {
            using policies_and_children_tuple =
                boost::mp11::mp_append<typename Current::policies_type,
                                       typename Current::children_type>;

            tuple_type_iterator<policies_and_children_tuple>([](auto i) {
                using next_type = std::tuple_element_t<i, policies_and_children_tuple>;
                tree_type_recursor_impl<Visitor, SkipFn, next_type, Current, Parents...>();
            });
        }
    }
}

template <template <typename...> typename Visitor, typename Current, typename... Parents>
struct tree_type_recursor_collector_impl {
    template <typename ChildOrPolicy>
    using recurse =
        typename tree_type_recursor_collector_impl<Visitor, ChildOrPolicy, Current, Parents...>::
            type;

    template <typename T>
    using get_children_type = typename T::children_type;
    template <typename T>
    using get_policies_type = typename T::policies_type;

    using children_type = boost::mp11::
        mp_eval_if_c<!is_tree_node_v<Current>, std::tuple<>, get_children_type, Current>;
    using policies_type = boost::mp11::
        mp_eval_if_c<!is_tree_node_v<Current>, std::tuple<>, get_policies_type, Current>;

    using type =
        boost::mp11::mp_eval_if_c<!is_tree_node_v<Current>,
                                  std::tuple<typename Visitor<Current, Parents...>::type>,
                                  boost::mp11::mp_push_back,
                                  boost::mp11::mp_flatten<boost::mp11::mp_transform<
                                      recurse,
                                      boost::mp11::mp_append<children_type, policies_type>>>,
                                  typename Visitor<Current, Parents...>::type>;
};
}  // namespace detail

/** Depth-first search of the node tree, calling @a visitor on every tree_node.
 *
 * @a Visitor must be a Callable with the same method signatue as below:
 * @code
 * template <typename Current, typename... Parents>
 * void operator()(const Current& current, const Parents&... parents)
 * {
 *     ...
 * }
 * @endcode
 * Where <TT>current</TT> is the current node from the tree, and <TT>parents</TT> is a pack of
 * ancestors in increasing generation from <TT>current</TT>.  The last in the pack is always
 * @a root.
 * @tparam Visitor Visitor type
 * @tparam Root tree_node derived type to start from
 * @param visitor Callable called on each tree_node visited
 * @param root Tree node to start from (must be a tree_node derived type!)
 */
template <typename Visitor, typename Root>
constexpr void tree_recursor(Visitor visitor, const Root& root)
{
    detail::tree_recursor_impl(visitor, root);
}

/** Depth-first recurse through the parse tree, calling @a Visitor on every tree_node @em and its
 * policies.
 *
 * @a Visitor must be object with the same method signatue as below:
 * @code
 * struct my_fn {
 *     template <typename Current, typename... Parents>
 *     constexpr static void fn()
 *     {
 *         ...
 *     }
 * };
 * @endcode
 * Where <TT>Current</TT> is the current type from the tree, and <TT>Parents</TT> is a pack of
 * ancestors in increasing generation from <TT>Current</TT>.  The last in the pack is always the
 * root.
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
 * If the return value is true, then the <TT>Current</TT> node or policy and any subtree below it
 * are skipped i.e. <TT>fn</TT> is not called.
 *
 * @note Unlike tree_recursor(Visitor visitor, const Root& root), this will recurse into tree_node
 * policies.
 * @tparam Visitor Visitor type
 * @tparam SkipFn Skip function type
 * @tparam Root Start object type in the parse tree
 */
template <typename Visitor, typename SkipFn, typename Root>
constexpr void tree_type_recursor()
{
    detail::tree_type_recursor_impl<Visitor, SkipFn, Root>();
}

/** Overload without a skip function.
 *
 * @tparam Visitor Visitor type
 * @tparam Root Start object type in the parse tree
 */
template <typename Visitor, typename Root>
constexpr void tree_type_recursor()
{
    detail::tree_type_recursor_impl<Visitor, detail::always_false, Root>();
}

/** Similar in principle to tree_type_recursor(), but allows for a type to be built up from the
 * visitor as it recurses.
 *
 * @a Visitor must be a template template type like below:
 * @code
 * template <typename Current, typename... Parents>
 * struct my_fn_t {
 *     using type = // Some result type of the visitation
 * };
 * @endcode
 * <TT>my_fn_t<...>::type</TT> of all the nodes and policies is concatenated into a
 * <TT>std::tuple</TT> available in the type member.
 * @tparam Visitor Visitor object type
 * @tparam Root Start object type in the parse tree
 */
template <template <typename...> typename Visitor, typename Root>
struct tree_type_recursor_collector {
    /** Tuple of all the @a Visitor result types. */
    using type = typename detail::tree_type_recursor_collector_impl<Visitor, Root>::type;
};

/** Helper alias for tree_type_recursor_collector.
 *
 * @tparam Visitor Visitor object type
 * @tparam Root Start object type in the parse tree
 */
template <template <typename...> typename Visitor, typename Root>
using tree_type_recursor_collector_t = typename tree_type_recursor_collector<Visitor, Root>::type;
}  // namespace arg_router::utility
