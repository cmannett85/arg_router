/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/utility/tuple_iterator.hpp"

#include <boost/test/unit_test.hpp>

namespace arg_router
{
namespace test
{
/** Loops through the list of argument sets in @a args and executes the test
 * function object @a f with them.
 *
 * @a f should contain standard Boost Test checking macros, so the test can fail
 * inside it.  No return value of @a f is read.
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
 * This allows the passing of types as test arguments, useful for testing
 * template parameters.
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

/** Compiles @a code and returns the result.
 *
 * There is a dedicated CMake target for death tests, this function replaces
 * the content of the target's only source file with @a code (which is why it
 * must contain <TT>main()</TT> function) and attempts to build it.
 * 
 * This function contains all the necessary <TT>BOOST_CHECK_...</TT> calls for
 * testing.
 * @param code Code to compile, must contain a <TT>main()</TT>
 * @param expected_error Error string to search for in output
 */
void death_test_compile(std::string_view code, std::string_view expected_error);
}  // namespace test
}  // namespace arg_router
