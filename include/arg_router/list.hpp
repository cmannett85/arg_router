// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/tree_node_fwd.hpp"

#include <tuple>

namespace arg_router
{
/** This is arg and flag container, that when used a child in another tree_node is 'flattened' i.e.
 * the children of the list become the direct children of the parent.
 *
 * This is used to easily copy groups of args and flags into multiple modes.
 * @tparam Params Child types
 */
template <typename... Children>
class list
{
public:
    /** A tuple of all the child tree node types in parameters_type. */
    using children_type = std::tuple<std::decay_t<Children>...>;

    static_assert(boost::mp11::mp_all_of<children_type, is_tree_node>::value,
                  "All list children must be tree_nodes (i.e. not policies)");

    /** Constructor.
     *
     * @param children Child instances
     */
    constexpr explicit list(Children... children) noexcept : children_{std::move(children)...} {}

    /** Returns a reference to the children.
     *
     * @return Children
     */
    [[nodiscard]] children_type& children() noexcept { return children_; }

    /** Const overload.
     *
     * @return Children
     */
    [[nodiscard]] const children_type& children() const noexcept { return children_; }

private:
    children_type children_;
};

namespace detail
{
// Forward declared
template <typename Result, typename Next, typename... Others>
constexpr auto list_expander_impl(Result r, Next n, Others... others) noexcept;

template <typename Result, typename ListChildren, std::size_t... I>
[[nodiscard]] constexpr auto list_expander_unpacker(
    Result result,
    ListChildren list_children,
    [[maybe_unused]] std::integer_sequence<std::size_t, I...> Is) noexcept
{
    return list_expander_impl(std::move(result), std::move(std::get<I>(list_children))...);
}

// Recursion end condition
template <typename Result>
[[nodiscard]] constexpr auto list_expander_impl(Result result) noexcept
{
    return result;
}

template <typename Result, typename Next, typename... Others>
[[nodiscard]] constexpr auto list_expander_impl(Result result, Next next, Others... others) noexcept
{
    if constexpr (traits::is_specialisation_of_v<std::decay_t<Next>, list>) {
        return list_expander_impl(
            list_expander_unpacker(
                std::move(result),
                next.children(),
                std::make_index_sequence<std::tuple_size_v<typename Next::children_type>>{}),
            std::move(others)...);
    } else {
        return list_expander_impl(algorithm::tuple_push_back(std::move(result), std::move(next)),
                                  std::move(others)...);
    }
}
}  // namespace detail

/** Takes a pack of node parameters and flattens any list types within it.
 *
 * @tparam Params Parameter types
 * @param params Parameter instances
 * @return Flattened tuple of nodes
 */
template <typename... Params>
[[nodiscard]] constexpr auto list_expander(Params... params) noexcept
{
    return detail::list_expander_impl(std::tuple{}, std::move(params)...);
}

/** Takes a tuple of node parameters and flattens any list types within it.
 *
 * @tparam Params Parameter types
 * @param params Parameter instances
 * @return Flattened tuple of nodes
 */
template <typename... Params>
[[nodiscard]] constexpr auto list_expander(std::tuple<Params...> params) noexcept
{
    return std::apply(
        [](auto&... args) {
            return detail::list_expander_impl(std::tuple{}, std::forward<decltype(args)>(args)...);
        },
        params);
}
}  // namespace arg_router
