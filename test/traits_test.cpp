/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/traits.hpp"

#include "test_helpers.hpp"

#include <array>
#include <deque>
#include <variant>

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(traits_suite)

BOOST_AUTO_TEST_CASE(is_tuple_like_test)
{
    static_assert(traits::is_tuple_like_v<std::tuple<>>, "Fail");
    static_assert(traits::is_tuple_like_v<std::tuple<int, double>>, "Fail");
    static_assert(!traits::is_tuple_like_v<int>, "Fail");
    static_assert(!traits::is_tuple_like_v<double>, "Fail");
}

BOOST_AUTO_TEST_CASE(is_specialisation_test)
{
    struct test {
    };

    static_assert(traits::is_specialisation_v<std::vector<int>>, "Fail");
    static_assert(traits::is_specialisation_v<std::deque<int>>, "Fail");
    static_assert(traits::is_specialisation_v<std::string_view>, "Fail");
    static_assert(traits::is_specialisation_v<std::tuple<char, int, double>>,
                  "Fail");

    static_assert(!traits::is_specialisation_v<float>, "Fail");
    static_assert(!traits::is_specialisation_v<test>, "Fail");
}

BOOST_AUTO_TEST_CASE(is_specialisation_of_test)
{
    static_assert(traits::is_specialisation_of_v<std::vector<int>, std::vector>,
                  "Fail");

    static_assert(!traits::is_specialisation_of_v<std::vector<int>,
                                                  std::basic_string_view>,
                  "Fail");
    static_assert(!traits::is_specialisation_of_v<std::vector<int>, std::deque>,
                  "Fail");
    static_assert(!traits::is_specialisation_of_v<double, std::deque>, "Fail");
}

BOOST_AUTO_TEST_CASE(is_same_when_despecialised_test)
{
    static_assert(traits::is_same_when_despecialised_v<std::vector<int>,
                                                       std::vector<int>>,
                  "Fail");
    static_assert(traits::is_same_when_despecialised_v<std::vector<int>,
                                                       std::vector<double>>,
                  "Fail");

    static_assert(!traits::is_same_when_despecialised_v<std::vector<int>,
                                                        std::deque<int>>,
                  "Fail");
    static_assert(!traits::is_same_when_despecialised_v<std::vector<int>, int>,
                  "Fail");
    static_assert(!traits::is_same_when_despecialised_v<int, std::vector<int>>,
                  "Fail");
    static_assert(!traits::is_same_when_despecialised_v<int, int>, "Fail");
}

BOOST_AUTO_TEST_CASE(integral_constant_test)
{
    static_assert(
        std::is_same_v<traits::integral_constant<-42>::value_type, int>,
        "Fail");
    static_assert(traits::integral_constant<-42>::value == -42, "Fail");

    static_assert(
        std::is_same_v<traits::integral_constant<std::size_t{42}>::value_type,
                       std::size_t>,
        "Fail");
    static_assert(traits::integral_constant<std::size_t{42}>::value == 42,
                  "Fail");
}

BOOST_AUTO_TEST_SUITE_END()
