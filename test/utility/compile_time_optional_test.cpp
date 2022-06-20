/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/utility/compile_time_optional.hpp"

#include "test_helpers.hpp"

using namespace std::string_literals;
using namespace arg_router;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(compile_time_optional_suite)

BOOST_AUTO_TEST_CASE(constructor_test)
{
    constexpr auto cto1 = utility::compile_time_optional{};
    static_assert(cto1.empty, "Failed");
    static_assert(!cto1, "Failed");

    constexpr auto cto2 = utility::compile_time_optional{42};
    static_assert(!cto2.empty, "Failed");
    static_assert(cto2, "Failed");

    const auto cto3 = utility::compile_time_optional{"hello"s};
    static_assert(!cto3.empty, "Failed");
    static_assert(cto3, "Failed");

    const auto val = 42;
    auto cto4 = utility::compile_time_optional{std::cref(val)};
    static_assert(!cto4.empty, "Failed");
    static_assert(cto4, "Failed");
}

BOOST_AUTO_TEST_CASE(derefence_operator_test)
{
    constexpr auto cto1 = utility::compile_time_optional{42};
    static_assert(*cto1 == 42, "Failed");

    const auto cto2 = utility::compile_time_optional{"hello"s};
    BOOST_CHECK_EQUAL(*cto2, "hello");

    auto cto3 = utility::compile_time_optional{42};
    *cto3 = 84;
    BOOST_CHECK_EQUAL(*cto3, 84);

    auto val = 42;
    auto cto4 = utility::compile_time_optional{std::ref(val)};
    *cto4 = 84;
    BOOST_CHECK_EQUAL(*cto4, 84);
}

BOOST_AUTO_TEST_CASE(derefence_to_member_operator_test)
{
    const auto cto1 = utility::compile_time_optional{"hello"s};
    BOOST_CHECK_EQUAL(cto1->size(), 5);

    auto val = "hello"s;
    auto cto2 = utility::compile_time_optional{std::ref(val)};
    BOOST_CHECK_EQUAL(cto2->size(), 5);
    *cto2 += " world";
    BOOST_CHECK_EQUAL(cto2->size(), 11);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
