/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/utility/tuple_iterator.hpp"

#include <boost/test/unit_test.hpp>

#include <forward_list>
#include <typeindex>

namespace arg_router
{
namespace test
{
/** Returns the node using the descending child indices, starting from @a root.
 *
 * For example:
 * @code
 * test::get_node<3, 2, 1>(root)
 * @endcode
 * Will access the child at index 3 of @a root, then it's child at index 2, then return it's child
 * at index 1.
 * @tparam I First index
 * @tparam Is Optional nested indices
 * @tparam Root Starting node type
 * @param root Starting node
 * @return Reference to node at the specified child indices
 */
template <std::size_t I, std::size_t... Is, typename Root>
constexpr auto& get_node(Root& root) noexcept
{
    auto& child = std::get<I>(root.children());

    if constexpr (sizeof...(Is) > 0) {
        return get_node<Is...>(child);
    } else {
        return child;
    }
}

/** Returns the <TT>std::type_index</TT> using the descending child indices, starting from @a root.
 *
 * @tparam I First index
 * @tparam Is Optional nested indices
 * @tparam Root Starting node type
 * @param root Starting node
 * @return Type index of node at the specified child indices
 */
template <std::size_t I, std::size_t... Is, typename Root>
std::type_index get_type_index(const Root& root) noexcept
{
    const auto& child = get_node<I, Is...>(root);
    return typeid(std::decay_t<decltype(child)>);
}

/** Generates a tuple of <TT>std::reference_wrapper<TT>s containing a node and all of its parents in
 * ascending ancestry.
 *
 * For example:
 * @code
 * test::get_parents<3, 2, 1>(root)
 * @endcode
 * The template values are the indices of each successive child tuple starting from @a root.  So
 * here the child at index 3 of @a root, then it's child at index 2, and then it's child at index 1;
 * the tree instances are returned in reverse order.
 * @tparam I First index
 * @tparam Is Optional nested indices
 * @tparam Root Starting node type
 * @param root Starting node
 * @return Tuple of parent references
 */
template <std::size_t I, std::size_t... Is, typename Root>
constexpr auto get_parents(const Root& root) noexcept
{
    auto result = std::tuple{std::cref(get_node<I, Is...>(root))};

    if constexpr (sizeof...(Is) > 0) {
        // All this because you can't resize a tuple in std...
        using index_tuple = boost::mp11::mp_pop_back<
            std::tuple<traits::integral_constant<I>, traits::integral_constant<Is>...>>;
        return std::apply(
            [&](auto... NewIs) { return std::tuple_cat(result, get_parents<NewIs...>(root)); },
            index_tuple{});
    } else {
        return std::tuple_cat(result, std::tuple{std::cref(root)});
    }
}

/** Loops through the list of argument sets in @a args and executes the test function object @a f
 * with them.
 *
 * @a f should contain standard Boost Test checking macros, so the test can fail inside it.  No
 * return value of @a f is read.
 *
 * @code
 * const auto args = { std::make_tuple(1, 2, 3),
 *                     std::make_tuple(3, 4, 7) };
 * auto f = [](auto a, auto b, auto expected) {
 *     const auto actual = a + b;
 *     BOOST_CHECK_EQUAL(actual, expected);
 * };
 *
 * data_set(f, args);
 * @endcode
 *
 * @tparam F The test function object type
 * @tparam Args The argument types
 * @param f The test function object
 * @param args A list of tuples carrying @a Args
 */
template <typename F, typename... Args>
void data_set(F&& f, std::vector<std::tuple<Args...>>&& args)
{
    auto count = 0u;
    for (auto&& a : args) {
        BOOST_TEST_MESSAGE("Performing test " << ++count);
        std::apply(f, std::move(a));
    }
}

/** Overload for std::initializer_list.
 *
 * @tparam F The test function object type
 * @tparam Args The argument types
 * @param f The test function object
 * @param args A list of tuples carrying @a Args
 */
template <typename F, typename... Args>
void data_set(F&& f, std::initializer_list<std::tuple<Args...>> args)
{
    data_set(std::forward<F>(f), std::vector<std::tuple<Args...>>(args));
}

/** Overload for for passing a tuple of tuples.
 *
 * This allows the passing of types as test arguments, useful for testing template parameters.
 *
 * @code
 * using data_set = std::tuple<
 *    std::pair<traits::integral_constant<2u>,  traits::integral_constant<4u>>,
 *    std::pair<traits::integral_constant<3u>,  traits::integral_constant<8u>>
 * >;
 *
 * auto f = [](auto expo, auto result) {
 *     constexpr auto r = math::ipow<std::uint32_t, 2u, decltype(expo)::value>();
 *     BOOST_CHECK_EQUAL(r, decltype(result)::value);
 * };
 *
 * test::data_set(f, data_set{});
 * @endcode
 * Or for using test data sets with different types in each test, for example:
 * @code
 * auto f = [](auto input, auto expected) {
 *     using T = std::decay_t<decltype(expected)>;
 *     const auto result = utility::from_chars<T>(input);
 *     BOOST_CHECK_EQUAL(result, expected);
 * };
 *
 * test::data_set(
 *     f,
 *     std::tuple{
 *         std::tuple{"0"sv, std::uint8_t{0}},
 *         std::tuple{"+42"sv, std::uint32_t{42}},
 *         std::tuple{"-42"sv, std::int32_t{-42}},
 *     }
 * );
 * @endcode
 * @tparam F The test function object type
 * @tparam Args The argument types
 * @param f The test function object
 * @param args A tuple of tuples carrying @a Args
 */
template <typename F, typename... Args>
constexpr void data_set(F&& f, std::tuple<Args...>&& tuple)
{
    utility::tuple_iterator(
        [&](auto i, auto&& t) {
            BOOST_TEST_MESSAGE("Performing test " << (i + 1));
            std::apply(f, std::forward<std::decay_t<decltype(t)>>(t));
        },
        std::forward<std::decay_t<decltype(tuple)>>(tuple));
}

/** Pair-like structure containing the input data for death_test_compile. */
struct death_test_info {
    std::string code;                 /// Code to compile, must contain a <TT>main()</TT>
    std::string_view expected_error;  /// Error string to search for in output
    std::string_view test_name;       /// Test name, ignored if empty
};

/** Runs multiple death tests in parallel, up to a maximum of AR_DEATH_TEST_PARALLEL.
 *
 * @param tests death_test_info instances
 */
void death_test_compile(std::forward_list<death_test_info> tests);

/** Compiles @a code and returns the result.
 *
 * There is a dedicated CMake target for death tests, this function replaces the content of the
 * target's only source file with @a code (which is why it must contain <TT>main()</TT> function)
 * and attempts to build it.
 *
 * This function contains all the necessary <TT>BOOST_CHECK_...</TT> calls for testing.
 * @param code Code to compile, must contain a <TT>main()</TT>
 * @param expected_error Error string to search for in output
 */
void death_test_compile(std::string_view code, std::string_view expected_error);

}  // namespace test
}  // namespace arg_router
