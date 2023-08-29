// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/error_name.hpp"
#include "arg_router/literals.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace arg_router::literals;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(error_name_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::error_name_t<str<"hello">>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_and_get_test)
{
    constexpr auto hello_str = policy::error_name_t{"hello"_S};
    static_assert(hello_str.error_name() == "hello");

    constexpr auto world_str = policy::error_name_t{"world"_S};
    static_assert(world_str.error_name() == "world");
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(empty_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/error_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"
#include "arg_router/literals.hpp"
using namespace arg_router::literals;
int main() {
    const auto des = arg_router::policy::error_name_t{""_S};
    return 0;
}
    )",
        "Error name must not be empty");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
