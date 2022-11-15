// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/short_name.hpp"
#include "arg_router/literals.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace arg_router::literals;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(short_name_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::short_name_t<traits::integral_constant<'a'>>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_and_get_test)
{
    constexpr auto c_a = policy::short_name<'a'>;
    static_assert(c_a.short_name() == "a");

    constexpr auto c_4 = policy::short_name<'4'>;
    static_assert(c_4.short_name() == "4");

    constexpr auto s_a = policy::short_name_t{S_("a"){}};
    static_assert(s_a.short_name() == "a");
}

#ifdef ENABLE_CPP20_STRINGS
BOOST_AUTO_TEST_CASE(string_literal_test)
{
    const auto s_a = policy::short_name_t{"a"_S};
    static_assert(s_a.short_name() == "a");
}
#endif

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{R"(
#include "arg_router/policy/short_name.hpp"
int main() {
    const auto ln = arg_router::policy::short_name_utf8<S_("")>;
    return 0;
}
    )",
                               "Short name must only be one character",
                               "short_name_must_be_one_character_test"},
                              {
                                  R"(
#include "arg_router/policy/short_name.hpp"
int main() {
    const auto ln = arg_router::policy::short_name<'-'>;
    return 0;
}
    )",
                                  "Short name with short prefix cannot match the long prefix",
                                  "short_name_cannot_start_with_argument_prefix_test"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
