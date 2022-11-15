// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/display_name.hpp"
#include "arg_router/literals.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace arg_router::literals;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(display_name_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::display_name_t<S_("hello")>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_and_get_test)
{
    constexpr auto hello_str = policy::display_name<S_("hello")>;
    static_assert(hello_str.display_name() == "hello");

    constexpr auto world_str = policy::display_name_t{S_("world"){}};
    static_assert(world_str.display_name() == "world");
}

#ifdef ENABLE_CPP20_STRINGS
BOOST_AUTO_TEST_CASE(string_literal_test)
{
    const auto world_str = policy::display_name_t{"world"_S};
    static_assert(world_str.display_name() == "world");
}
#endif

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(empty_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/display_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"
int main() {
    const auto des = arg_router::policy::display_name<S_("")>;
    return 0;
}
    )",
        "Display name must not be empty");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
