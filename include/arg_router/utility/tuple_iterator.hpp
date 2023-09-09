// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/traits.hpp"

#include <boost/mp11/algorithm.hpp>

/** Namespace for utility types and functions.
 */
namespace arg_router::utility
{
/** Iterates over @a tuple and calls @a f on each element.
 *
 * @code
 * auto t = std::tuple{"hello"s, 42.5, 9};
 * tuple_iterator([](auto i, auto&& v) {
 *     std::cout << i << ": " << v << std::endl;
 * }, t);
 * // 0: hello
 * // 1: 42.5
 * // 2: 9
 * @endcode
 * @tparam F Callable type
 * @tparam Tuple Tuple type
 * @param f Callable instance
 * @param tuple Tuple instance
 */
template <typename F, typename Tuple>
    requires concepts::is_tuple_like<std::decay_t<Tuple>>
constexpr void tuple_iterator(F&& f, Tuple&& tuple)
{
    constexpr auto N = std::tuple_size_v<std::decay_t<Tuple>>;
    boost::mp11::mp_for_each<boost::mp11::mp_iota_c<N>>([&](auto i) { f(i, std::get<i>(tuple)); });
}

/** Iterates over @a pack and calls @a f on each instance within it.
 *
 * Forwards @a pack into a tuple and uses it to call tuple_iterator(F&&, Tuple&&).
 * @tparam F Callable type
 * @tparam T Pack types
 * @param f Callable instance
 * @param pack Instances to iterate over
 */
template <typename F, typename... T>
constexpr void tuple_iterator(F&& f, T&&... pack)
{
    tuple_iterator(std::forward<F>(f), std::tuple{std::forward<T>(pack)...});
}

/** Iterates over @a tuple and passes a nullptr pointer of the tuple element to
 * @a f.
 *
 * @code
 * using Tuple = std::tuple<std::string, double, int>;
 * tuple_type_iterator<Tuple>([](auto i) {
 *     using Arg = std::tuple_element_t<i, Tuple>;
 *     std::cout << i << ": " << boost::core::demangle(typeid(Arg).name())
 *               << std::endl;
 * });
 * // 0: std::string
 * // 1: double
 * // 2: int
 * @endcode
 * @tparam Tuple Tuple type
 * @tparam F Callable type
 * @param f Callable instance
 */
template <typename Tuple, typename F>
constexpr void tuple_type_iterator(F&& f)
    requires concepts::is_tuple_like<std::decay_t<Tuple>>
{
    constexpr auto N = std::tuple_size_v<std::decay_t<Tuple>>;
    boost::mp11::mp_for_each<boost::mp11::mp_iota_c<N>>([&](auto i) { f(i); });
}
}  // namespace arg_router::utility
