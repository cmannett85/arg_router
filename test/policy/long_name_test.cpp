/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(long_name_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::long_name_t<S_("hello")>>, "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_and_get_test)
{
    const auto hello_str = policy::long_name<S_("hello")>;
    BOOST_CHECK_EQUAL(hello_str.long_name(), "hello");

    const auto three_char_str = policy::long_name<S_("boo")>;
    BOOST_CHECK_EQUAL(three_char_str.long_name(), "boo");
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{
                                  R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"
int main() {
    const auto ln = arg_router::policy::long_name<S_("")>;
    return 0;
}
    )",
                                  "Long names must be longer than one character",
                                  "empty_test"},
                              {
                                  R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"
int main() {
    const auto ln = arg_router::policy::long_name<S_("a")>;
    return 0;
}
    )",
                                  "Long names must be longer than one character",
                                  "single_char_test"},
                              {
                                  R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"
int main() {
    const auto ln = arg_router::policy::long_name<S_("a b")>;
    return 0;
}
    )",
                                  "Long names cannot contain whitespace",
                                  "space_test"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
