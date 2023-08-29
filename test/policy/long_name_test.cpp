// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/long_name.hpp"
#include "arg_router/literals.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace arg_router::literals;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(long_name_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::long_name_t<str<"hello">>>, "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_and_get_test)
{
    constexpr auto hello_str = policy::long_name_t{"hello"_S};
    static_assert(hello_str.long_name() == "hello");

    constexpr auto three_char_str = policy::long_name_t{"boo"_S};
    static_assert(three_char_str.long_name() == "boo");

    constexpr auto world_str = policy::long_name_t{"world"_S};
    static_assert(world_str.long_name() == "world");
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{
                                  R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"
#include "arg_router/literals.hpp"
using namespace arg_router::literals;
int main() {
    const auto ln = arg_router::policy::long_name_t{""_S};
    return 0;
}
    )",
                                  "Long names must be longer than one character",
                                  "empty_test"},
                              {
                                  R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"
#include "arg_router/literals.hpp"
using namespace arg_router::literals;
int main() {
    const auto ln = arg_router::policy::long_name_t{"a"_S};
    return 0;
}
    )",
                                  "Long names must be longer than one character",
                                  "single_char_test"},
                              {
                                  R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"
#include "arg_router/literals.hpp"
using namespace arg_router::literals;
int main() {
    const auto ln = arg_router::policy::long_name_t{"a b"_S};
    return 0;
}
    )",
                                  "Long names cannot contain whitespace",
                                  "space_test"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
