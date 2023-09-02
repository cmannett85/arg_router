// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/arg.hpp"
#include "arg_router/help_data.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(arg_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<arg_t<int, policy::long_name_t<str<"hello">>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(policies_test)
{
    [[maybe_unused]] auto f = arg<int>(policy::long_name_t{"hello"_S},  //
                                       policy::short_name_t{"H"_S});
    static_assert(f.long_name() == "hello", "Long name test fail");
    static_assert(f.short_name() == "H", "Short name test fail");
}

BOOST_AUTO_TEST_CASE(parse_test)
{
    auto router_hit = false;
    auto f = [&](auto node,
                 auto tokens,
                 auto expected_result,
                 auto expected_router_hit,
                 const auto&... parents) {
        router_hit = false;

        auto target = parsing::parse_target{std::move(tokens), node, parents...};
        const auto result = node.parse(std::move(target), parents...);
        BOOST_CHECK_EQUAL(result, expected_result);
        BOOST_CHECK_EQUAL(router_hit, expected_router_hit);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{arg<int>(policy::long_name_t{"test"_S}),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}},
                       42,
                       false},
            std::tuple{arg<int>(policy::long_name_t{"test"_S}, policy::router{[&](int result) {
                                    BOOST_CHECK_EQUAL(result, 42);
                                    router_hit = true;
                                }}),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}},
                       42,
                       true},
            std::tuple{arg<int>("test"_S, policy::router{[&](int result) {
                                    BOOST_CHECK_EQUAL(result, 42);
                                    router_hit = true;
                                }}),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}},
                       42,
                       true}});
}

BOOST_AUTO_TEST_CASE(help_test)
{
    auto f = [](const auto& node, auto expected_label, auto expected_description) {
        const auto help_data = help_data::generate<false>(node);
        const auto flattened_help_data = help_data::generate<true>(node);

        BOOST_CHECK_EQUAL(help_data, flattened_help_data);
        BOOST_CHECK_EQUAL(help_data.label, expected_label);
        BOOST_CHECK_EQUAL(help_data.description, expected_description);
        BOOST_CHECK_EQUAL(help_data.children.size(), 0);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{arg<int>(policy::short_name_t{"h"_S},
                                policy::long_name_t{"hello"_S},
                                policy::description_t{"An arg!"_S}),
                       "--hello,-h <Value>",
                       "An arg!"},
            std::tuple{arg<int>(policy::long_name_t{"hello"_S}, policy::description_t{"An arg!"_S}),
                       "--hello <Value>",
                       "An arg!"},
            std::tuple{arg<int>(policy::short_name_t{"h"_S}, policy::description_t{"An arg!"_S}),
                       "-h <Value>",
                       "An arg!"},
            std::tuple{arg<int>(policy::short_name_t{"h"_S}), "-h <Value>", ""},
            std::tuple{arg<int>("h"_S, "hello"_S, "An arg!"_S), "--hello,-h <Value>", "An arg!"},
        });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{
                                  R"(
#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = arg<int>(
        policy::long_name_t{"hello"_S},
        flag(policy::short_name_t{"b"_S}),
        policy::short_name_t{"H"_S}
    );
    return 0;
}
    )",
                                  "Arg must only contain policies (not other nodes)",
                                  "only_policies_test"},
                              {
                                  R"(
#include "arg_router/arg.hpp"

using namespace arg_router;

int main() {
    auto a = arg<int>();
    return 0;
}
    )",
                                  "Arg must be named",
                                  "must_be_named_test"},
                              {
                                  R"(
#include "arg_router/arg.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = arg<int>(policy::long_name_t{"hello"_S},
                      policy::display_name_t{"hello2"_S});
    return 0;
}
    )",
                                  "Arg must not have a display name policy",
                                  "must_not_have_display_name_test"},
                              {
                                  R"(
#include "arg_router/arg.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = arg<int>(policy::long_name_t{"hello"_S},
                      policy::none_name_t{"hello2"_S});
    return 0;
}
    )",
                                  "Arg must not have a none name policy",
                                  "must_not_have_none_name_test"}});
}

BOOST_AUTO_TEST_SUITE_END()
