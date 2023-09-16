// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/utility/from_chars.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(from_chars_suite)

BOOST_AUTO_TEST_CASE(integer_test)
{
    auto f = [](auto expected_result, auto str) {
        using T = typename std::decay_t<decltype(expected_result)>::value_type;

        const auto result = utility::from_chars<T>(str);
        BOOST_CHECK_EQUAL(expected_result, result);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::optional<int>{}, ""},
                       std::tuple{std::optional<int>{}, "  "},
                       std::tuple{std::optional<int>{}, "+"},
                       std::tuple{std::optional<int>{}, "0x"},
                       std::tuple{std::optional<int>{}, "0X"},
                       std::tuple{std::optional<int>{}, "hello"},
                       std::tuple{std::optional{42}, "42"},
                       std::tuple{std::optional{42}, " 42 "},
                       std::tuple{std::optional{42}, "+42"},
                       std::tuple{std::optional{0x42}, "0x42"},
                       std::tuple{std::optional{0x42}, "0X42"},
                   });
}

BOOST_AUTO_TEST_CASE(fp_test)
{
    auto f = [](auto expected_result, auto str) {
        using T = typename std::decay_t<decltype(expected_result)>::value_type;

        const auto result = utility::from_chars<T>(str);
        BOOST_CHECK_EQUAL(expected_result, result);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::optional<double>{}, ""},
                       std::tuple{std::optional<double>{}, "  "},
                       std::tuple{std::optional<double>{}, "+"},
                       std::tuple{std::optional<double>{}, "0x"},
                       std::tuple{std::optional<double>{}, "0X"},
                       std::tuple{std::optional<double>{}, "hello"},
                       std::tuple{std::optional{3.14}, "3.14"},
                       std::tuple{std::optional{3.14}, " 3.14 "},
                       std::tuple{std::optional{3.14}, "+3.14"},
                       std::tuple{std::optional{0x3.14p0}, "0x3.14"},
                       std::tuple{std::optional{0x3.14p0}, "0X3.14"},
                   });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
