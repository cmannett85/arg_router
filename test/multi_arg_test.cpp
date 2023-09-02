// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/multi_arg.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(multi_arg_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<multi_arg_t<std::vector<int>, policy::long_name_t<str<"hello">>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(policies_test)
{
    [[maybe_unused]] auto f = multi_arg<std::vector<int>>(policy::long_name_t{"hello"_S},  //
                                                          policy::short_name_t{"H"_S});
    static_assert(f.long_name() == "hello", "Long name test fail");
    static_assert(f.short_name() == "H", "Short name test fail");
}

BOOST_AUTO_TEST_CASE(parse_test)
{
    auto f = [&](auto node, auto tokens, auto expected_result) {
        auto target = parsing::parse_target{std::move(tokens), node};
        const auto result = node.parse(std::move(target));
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{multi_arg<std::vector<int>>(policy::long_name_t{"test"_S}),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}},
                       std::vector<int>{42}},
            std::tuple{multi_arg<std::vector<int>>("test"_S),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}},
                       std::vector<int>{42}},
            std::tuple{multi_arg<std::vector<int>>(policy::long_name_t{"test"_S}),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "24"},
                                                        {parsing::prefix_type::none, "66"}},
                       std::vector<int>{42, 24, 66}},
        });
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
            std::tuple{multi_arg<std::vector<int>>(policy::short_name_t{"h"_S},
                                                   policy::long_name_t{"hello"_S},
                                                   policy::description_t{"An arg!"_S}),
                       "--hello,-h [1,N]",
                       "An arg!"},
            std::tuple{multi_arg<std::vector<int>>(policy::short_name_t{"h"_S},
                                                   policy::long_name_t{"hello"_S},
                                                   policy::min_count<4>,
                                                   policy::description_t{"An arg!"_S}),
                       "--hello,-h [4,N]",
                       "An arg!"},
            std::tuple{multi_arg<std::vector<int>>(policy::short_name_t{"h"_S},
                                                   policy::long_name_t{"hello"_S},
                                                   policy::min_max_count<1, 4>,
                                                   policy::description_t{"An arg!"_S}),
                       "--hello,-h [1,4]",
                       "An arg!"},
            std::tuple{multi_arg<std::vector<int>>(policy::long_name_t{"hello"_S},
                                                   policy::description_t{"An arg!"_S}),
                       "--hello [1,N]",
                       "An arg!"},
            std::tuple{multi_arg<std::vector<int>>(policy::short_name_t{"h"_S},
                                                   policy::description_t{"An arg!"_S}),
                       "-h [1,N]",
                       "An arg!"},
            std::tuple{multi_arg<std::vector<int>>(policy::short_name_t{"h"_S}), "-h [1,N]", ""},
            std::tuple{multi_arg<std::vector<int>>("h"_S, "hello"_S, "An arg!"_S),
                       "--hello,-h [1,N]",
                       "An arg!"},
        });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({
        {
            R"(
#include "arg_router/multi_arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = multi_arg<std::vector<int>>(
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
#include "arg_router/multi_arg.hpp"

using namespace arg_router;

int main() {
    auto a = multi_arg<std::vector<int>>();
    return 0;
}
    )",
            "Arg must be named",
            "must_be_named_test"},
        {
            R"(
#include "arg_router/literals.hpp"
#include "arg_router/multi_arg.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = multi_arg<std::vector<int>>(policy::long_name_t{"hello"_S},
                      policy::display_name_t{"hello2"_S});
    return 0;
}
    )",
            "Multi arg must not have a display name policy",
            "must_not_have_display_name_test"},
        {
            R"(
#include "arg_router/literals.hpp"
#include "arg_router/multi_arg.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = multi_arg<std::vector<int>>(policy::long_name_t{"hello"_S},
                      policy::none_name_t{"hello2"_S});
    return 0;
}
    )",
            "Multi arg must not have a none name policy",
            "must_not_have_none_name_test"},
        {
            R"(
#include "arg_router/literals.hpp"
#include "arg_router/multi_arg.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = multi_arg<std::vector<int>>(policy::long_name_t{"hello"_S},
                                         policy::min_count<0>);
    return 0;
}
    )",
            "Multi arg requires a minimum of one value token, use min_max_count_t to define the "
            "range",
            "minimum_of_one_value_token"},
        {
            R"(
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/multi_arg.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = multi_arg<std::vector<int>>(policy::long_name_t{"--"_S},
                                         policy::router{[](int) {}});
    return 0;
}
    )",
            "Multi arg does not support policies with routing phases (e.g. router)",
            "routing_phase_test"},
    });
}

BOOST_AUTO_TEST_SUITE_END()
