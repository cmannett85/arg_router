// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/algorithm.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

#include <string>
#include <vector>

using namespace arg_router;
using namespace std::string_literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(algorithm_suite)

BOOST_AUTO_TEST_CASE(find_specialisation_test)
{
    BOOST_CHECK_EQUAL((algorithm::find_specialisation<
                          std::vector,
                          std::tuple<int, std::string, double, std::vector<int>, float>>::value),
                      3);
    BOOST_CHECK_EQUAL(
        (algorithm::find_specialisation<
            std::vector,
            std::tuple<int, std::vector<double>, double, std::vector<int>, float>>::value),
        1);
    BOOST_CHECK_EQUAL(
        (algorithm::find_specialisation<std::vector,
                                        std::tuple<int, std::string, double, float>>::value),
        4);
    BOOST_CHECK_EQUAL((algorithm::find_specialisation<std::vector, std::tuple<>>::value), 0);
}

BOOST_AUTO_TEST_CASE(find_specialisation_v_test)
{
    BOOST_CHECK_EQUAL((algorithm::find_specialisation_v<
                          std::vector,
                          std::tuple<int, std::string, double, std::vector<int>, float>>),
                      3);
    BOOST_CHECK_EQUAL((algorithm::find_specialisation_v<
                          std::vector,
                          std::tuple<int, std::vector<double>, double, std::vector<int>, float>>),
                      1);
    BOOST_CHECK_EQUAL(
        (algorithm::find_specialisation_v<std::vector,
                                          std::tuple<int, std::string, double, float>>),
        4);
    BOOST_CHECK_EQUAL((algorithm::find_specialisation_v<std::vector, std::tuple<>>), 0);
}

BOOST_AUTO_TEST_CASE(count_specialisation_test)
{
    BOOST_CHECK_EQUAL((algorithm::count_specialisation<
                          std::vector,
                          std::tuple<int, std::string, double, std::vector<int>, float>>::value),
                      1);
    BOOST_CHECK_EQUAL(
        (algorithm::count_specialisation<
            std::vector,
            std::tuple<int, std::vector<double>, double, std::vector<int>, float>>::value),
        2);
    BOOST_CHECK_EQUAL(
        (algorithm::count_specialisation<std::vector,
                                         std::tuple<int, std::string, double, float>>::value),
        0);
    BOOST_CHECK_EQUAL((algorithm::count_specialisation<std::vector, std::tuple<>>::value), 0);
}

BOOST_AUTO_TEST_CASE(count_specialisation_v_test)
{
    BOOST_CHECK_EQUAL((algorithm::count_specialisation_v<
                          std::vector,
                          std::tuple<int, std::string, double, std::vector<int>, float>>),
                      1);
    BOOST_CHECK_EQUAL((algorithm::count_specialisation_v<
                          std::vector,
                          std::tuple<int, std::vector<double>, double, std::vector<int>, float>>),
                      2);
    BOOST_CHECK_EQUAL(
        (algorithm::count_specialisation_v<std::vector,
                                           std::tuple<int, std::string, double, float>>),
        0);
    BOOST_CHECK_EQUAL((algorithm::count_specialisation_v<std::vector, std::tuple<>>), 0);
}

BOOST_AUTO_TEST_CASE(count_despecialised_test)
{
    BOOST_CHECK_EQUAL((algorithm::count_despecialised<
                          std::vector<double>,
                          std::tuple<int, std::string, double, std::vector<int>, float>>::value),
                      1);
    BOOST_CHECK_EQUAL(
        (algorithm::count_despecialised<
            std::vector<double>,
            std::tuple<int, std::vector<double>, double, std::vector<int>, float>>::value),
        2);
    BOOST_CHECK_EQUAL(
        (algorithm::count_despecialised<std::vector<double>,
                                        std::tuple<int, std::string, double, float>>::value),
        0);
    BOOST_CHECK_EQUAL((algorithm::count_despecialised<std::vector<double>, std::tuple<>>::value),
                      0);
}

BOOST_AUTO_TEST_CASE(count_despecialised_v_test)
{
    BOOST_CHECK_EQUAL((algorithm::count_despecialised_v<
                          std::vector<double>,
                          std::tuple<int, std::string, double, std::vector<int>, float>>),
                      1);
    BOOST_CHECK_EQUAL((algorithm::count_despecialised_v<
                          std::vector<double>,
                          std::tuple<int, std::vector<double>, double, std::vector<int>, float>>),
                      2);
    BOOST_CHECK_EQUAL(
        (algorithm::count_despecialised_v<std::vector<double>,
                                          std::tuple<int, std::string, double, float>>),
        0);
    BOOST_CHECK_EQUAL((algorithm::count_despecialised_v<std::vector<double>, std::tuple<>>), 0);
}

BOOST_AUTO_TEST_CASE(has_specialisation_test)
{
    BOOST_CHECK_EQUAL((algorithm::has_specialisation<
                          std::vector,
                          std::tuple<int, std::string, double, std::vector<int>, float>>::value),
                      true);
    BOOST_CHECK_EQUAL(
        (algorithm::has_specialisation<
            std::vector,
            std::tuple<int, std::vector<double>, double, std::vector<int>, float>>::value),
        true);
    BOOST_CHECK_EQUAL(
        (algorithm::has_specialisation<std::vector,
                                       std::tuple<int, std::string, double, float>>::value),
        false);
    BOOST_CHECK_EQUAL((algorithm::has_specialisation<std::vector, std::tuple<>>::value), false);
}

BOOST_AUTO_TEST_CASE(has_specialisation_v_test)
{
    BOOST_CHECK_EQUAL((algorithm::has_specialisation_v<
                          std::vector,
                          std::tuple<int, std::string, double, std::vector<int>, float>>),
                      true);
    BOOST_CHECK_EQUAL((algorithm::has_specialisation_v<
                          std::vector,
                          std::tuple<int, std::vector<double>, double, std::vector<int>, float>>),
                      true);
    BOOST_CHECK_EQUAL(
        (algorithm::has_specialisation_v<std::vector, std::tuple<int, std::string, double, float>>),
        false);
    BOOST_CHECK_EQUAL((algorithm::has_specialisation_v<std::vector, std::tuple<>>), false);
}

BOOST_AUTO_TEST_CASE(zip_test)
{
    auto f = [](auto first, auto second, auto expected) {
        using first_type = std::decay_t<decltype(first)>;
        using second_type = std::decay_t<decltype(second)>;
        using expected_type = std::decay_t<decltype(expected)>;
        using result_type = algorithm::zip_t<first_type, second_type>;

        BOOST_CHECK((std::is_same_v<result_type, expected_type>));
    };

    using data_set = std::tuple<
        std::tuple<std::tuple<std::integral_constant<std::size_t, 0>,
                              std::integral_constant<std::size_t, 1>,
                              std::integral_constant<std::size_t, 2>>,
                   std::tuple<float, int, std::string_view>,
                   std::tuple<std::pair<std::integral_constant<std::size_t, 0>, float>,
                              std::pair<std::integral_constant<std::size_t, 1>, int>,
                              std::pair<std::integral_constant<std::size_t, 2>, std::string_view>>>,
        std::tuple<
            std::tuple<float, int, std::string_view>,
            std::tuple<std::integral_constant<std::size_t, 0>,
                       std::integral_constant<std::size_t, 1>,
                       std::integral_constant<std::size_t, 2>>,
            std::tuple<std::pair<float, std::integral_constant<std::size_t, 0>>,
                       std::pair<int, std::integral_constant<std::size_t, 1>>,
                       std::pair<std::string_view, std::integral_constant<std::size_t, 2>>>>>;

    test::data_set(f, data_set{});
}

BOOST_AUTO_TEST_CASE(unzip_test)
{
    auto f = [](auto input, auto expected_first, auto expected_second) {
        using input_type = std::decay_t<decltype(input)>;
        using expected_first_type = std::decay_t<decltype(expected_first)>;
        using expected_second_type = std::decay_t<decltype(expected_second)>;

        using result_type = algorithm::unzip<input_type>;
        using first_type = typename result_type::first_type;
        using second_type = typename result_type::second_type;

        BOOST_CHECK((std::is_same_v<first_type, expected_first_type>));
        BOOST_CHECK((std::is_same_v<second_type, expected_second_type>));
    };

    using data_set = std::tuple<
        std::tuple<std::tuple<std::pair<std::integral_constant<std::size_t, 0>, float>,
                              std::pair<std::integral_constant<std::size_t, 1>, int>,
                              std::pair<std::integral_constant<std::size_t, 2>, std::string_view>>,
                   std::tuple<std::integral_constant<std::size_t, 0>,
                              std::integral_constant<std::size_t, 1>,
                              std::integral_constant<std::size_t, 2>>,
                   std::tuple<float, int, std::string_view>>,
        std::tuple<std::tuple<std::pair<float, std::integral_constant<std::size_t, 0>>,
                              std::pair<int, std::integral_constant<std::size_t, 1>>,
                              std::pair<std::string_view, std::integral_constant<std::size_t, 2>>>,
                   std::tuple<float, int, std::string_view>,
                   std::tuple<std::integral_constant<std::size_t, 0>,
                              std::integral_constant<std::size_t, 1>,
                              std::integral_constant<std::size_t, 2>>>>;

    test::data_set(f, data_set{});
}

BOOST_AUTO_TEST_CASE(tuple_drop_test)
{
    auto f = [](auto count, auto input, auto expected) {
        const auto result = algorithm::tuple_drop<count>(std::move(input));
        utility::tuple_iterator(
            [&](auto i, auto&& v) {
                const auto e = std::get<i>(expected);
                BOOST_CHECK_EQUAL(e, v);
            },
            result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{traits::integral_constant<0>{},
                       std::tuple{42, 3.14, "hello"},
                       std::tuple{42, 3.14, "hello"}},
            std::tuple{traits::integral_constant<1>{},
                       std::tuple{42, 3.14, "hello"},
                       std::tuple{3.14, "hello"}},
            std::tuple{traits::integral_constant<2>{},
                       std::tuple{42, 3.14, "hello"},
                       std::tuple{"hello"}},
            std::tuple{traits::integral_constant<3>{}, std::tuple{42, 3.14, "hello"}, std::tuple{}},
            std::tuple{traits::integral_constant<0>{}, std::tuple{}, std::tuple{}},
        });
}

BOOST_AUTO_TEST_CASE(pack_element_test)
{
    auto f = [](auto I, auto expected, auto... pack) {
        const auto& result = algorithm::pack_element<I>(pack...);
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{traits::integral_constant<0>{}, 0, 0, 1, 2, 3, 4},
                       std::tuple{traits::integral_constant<1>{}, 3, 4, 3, 2, 1, 0},
                       std::tuple{traits::integral_constant<2>{}, "c"s, "a"s, "b"s, "c"s},
                   });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(zip_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/algorithm.hpp"
int main() {
    using tuple_a = std::tuple<int, float, double>;
    using tuple_b = std::tuple<double, int>;
    using my_zip = arg_router::algorithm::zip_t<tuple_a, tuple_b>;
    return 0;
}
    )",
        "First and Second tuples must contain the same number of elements");
}

BOOST_AUTO_TEST_CASE(pack_element_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/algorithm.hpp"
int main() {
    const auto& result = arg_router::algorithm::pack_element<0>();
    return 0;
}
    )",
        "Index out of bounds for pack");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
