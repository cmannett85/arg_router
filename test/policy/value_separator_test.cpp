/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/policy/value_separator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(value_separator_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(
        policy::is_policy_v<
            policy::value_separator_t<traits::integral_constant<'='>>>,
        "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_and_get_test)
{
    const auto c_a = policy::value_separator<'='>;
    BOOST_CHECK_EQUAL(c_a.value_separator(), "=");

    const auto c_4 = policy::value_separator<'/'>;
    BOOST_CHECK_EQUAL(c_4.value_separator(), "/");
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(whitespace_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/value_separator.hpp"
int main() {
    const auto ln = arg_router::policy::value_separator<' '>;
    return 0;
}
    )",
        "Value separator character must not be whitespace");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
