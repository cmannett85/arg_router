#pragma once

#include "arg_router/traits.hpp"

#include <boost/mp11/algorithm.hpp>

namespace arg_router
{
/** Namespace for utility types and functions.
 */
namespace utility
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
 * @return Void
 */
template <typename F, typename Tuple>
constexpr std::enable_if_t<traits::is_tuple_like_v<std::decay_t<Tuple>>>
tuple_iterator(F&& f, Tuple&& tuple)
{
    constexpr auto N = std::tuple_size_v<std::decay_t<Tuple>>;
    boost::mp11::mp_for_each<boost::mp11::mp_iota_c<N>>(
        [&](auto i) { f(i, std::get<i>(tuple)); });
}

/** Iterates over @a pack and calls @a f on each instance within it.
 *
 * Forwards @a pack into a tuple and uses it to call
 * tuple_iterator(F&&, Tuple&&).
 * @tparam F Callable type
 * @tparam T Pack types
 * @param f Callable instance
 * @param pack Instances to iterate over
 * @return Void
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
 * @return Void
 */
template <typename Tuple, typename F>
constexpr std::enable_if_t<traits::is_tuple_like_v<std::decay_t<Tuple>>>
tuple_type_iterator(F&& f)
{
    constexpr auto N = std::tuple_size_v<std::decay_t<Tuple>>;
    boost::mp11::mp_for_each<boost::mp11::mp_iota_c<N>>([&](auto i) { f(i); });
}
}  // namespace utility
}  // namespace arg_router
