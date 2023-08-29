// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/list.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/policy.hpp"
#include "arg_router/policy/short_name.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace arg_router::literals;

BOOST_AUTO_TEST_SUITE(list_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(!is_tree_node_v<list<>>, "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(!policy::is_policy_v<list<>>, "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_test)
{
    const auto l = list{flag(policy::short_name_t{"a"_S}), flag(policy::short_name_t{"b"_S})};
    static_assert(
        std::is_same_v<
            std::decay_t<decltype(l.children())>,
            std::tuple<flag_t<policy::short_form_expander_t<>, policy::short_name_t<str<"a">>>,
                       flag_t<policy::short_form_expander_t<>, policy::short_name_t<str<"b">>>>>,
        "Constructor test failed");

    BOOST_CHECK_EQUAL(std::get<0>(l.children()).short_name(), "a");
}

BOOST_AUTO_TEST_CASE(list_expander_test)
{
    [[maybe_unused]] const auto result =
        list_expander(flag(policy::short_name_t{"a"_S}),
                      list{flag(policy::short_name_t{"b"_S}), flag(policy::short_name_t{"c"_S})},
                      flag(policy::short_name_t{"d"_S}));
    static_assert(
        std::is_same_v<
            std::decay_t<decltype(result)>,
            std::tuple<flag_t<policy::short_form_expander_t<>, policy::short_name_t<str<"a">>>,
                       flag_t<policy::short_form_expander_t<>, policy::short_name_t<str<"b">>>,
                       flag_t<policy::short_form_expander_t<>, policy::short_name_t<str<"c">>>,
                       flag_t<policy::short_form_expander_t<>, policy::short_name_t<str<"d">>>>>,
        "list_expander test failed");
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{R"(
#include "arg_router/list.hpp"
#include "arg_router/policy/short_name.hpp"

using namespace arg_router;

int main() {
    list<policy::short_name_t<traits::integral_constant<'a'>>>{};
        
    return 0;
}
    )",
                               "All list children must be tree_nodes (i.e. not policies)",
                               "single_policy_test"},
                              {
                                  R"(
#include "arg_router/list.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    list<policy::short_name_t<traits::integral_constant<'a'>>,
         policy::long_name_t<str<"hello">>,
         policy::short_name_t<traits::integral_constant<'b'>>>{};
        
    return 0;
}
    )",
                                  "All list children must be tree_nodes (i.e. not policies)",
                                  "triple_policy_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/list.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    list<flag_t<policy::short_name_t<traits::integral_constant<'a'>>,
                policy::long_name_t<str<"hello">>>,
         policy::short_name_t<traits::integral_constant<'b'>>>{};
        
    return 0;
}
    )",
                                  "All list children must be tree_nodes (i.e. not policies)",
                                  "tree_node_policy_mix_test"}});
}

BOOST_AUTO_TEST_SUITE_END()
