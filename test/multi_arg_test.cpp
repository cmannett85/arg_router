// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/multi_arg.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(multi_arg_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(
        is_tree_node_v<multi_arg_t<std::vector<int>, policy::long_name_t<AR_STRING("hello")>>>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(policies_test)
{
    [[maybe_unused]] auto f = multi_arg<std::vector<int>>(policy::long_name<AR_STRING("hello")>,  //
                                                          policy::short_name<'H'>);
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
            std::tuple{multi_arg<std::vector<int>>(policy::long_name<AR_STRING("test")>),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}},
                       std::vector<int>{42}},
            std::tuple{multi_arg<std::vector<int>>(AR_STRING("test"){}),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}},
                       std::vector<int>{42}},
            std::tuple{multi_arg<std::vector<int>>(policy::long_name<AR_STRING("test")>),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "24"},
                                                        {parsing::prefix_type::none, "66"}},
                       std::vector<int>{42, 24, 66}},
        });
}

BOOST_AUTO_TEST_CASE(help_test)
{
    auto f = [](const auto& node, auto expected_label, auto expected_description) {
        using node_type = std::decay_t<decltype(node)>;

        using help_data = typename node_type::template help_data_type<false>;
        using flattened_help_data = typename node_type::template help_data_type<true>;

        static_assert(
            std::is_same_v<typename help_data::label, typename flattened_help_data::label>);
        static_assert(std::is_same_v<typename help_data::description,
                                     typename flattened_help_data::description>);
        static_assert(std::tuple_size_v<typename help_data::children> == 0);
        static_assert(std::tuple_size_v<typename flattened_help_data::children> == 0);

        BOOST_CHECK_EQUAL(help_data::label::get(), expected_label);
        BOOST_CHECK_EQUAL(help_data::description::get(), expected_description);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{multi_arg<std::vector<int>>(policy::short_name<'h'>,
                                                   policy::long_name<AR_STRING("hello")>,
                                                   policy::description<AR_STRING("An arg!")>),
                       "--hello,-h [1,N]",
                       "An arg!"},
            std::tuple{multi_arg<std::vector<int>>(policy::short_name<'h'>,
                                                   policy::long_name<AR_STRING("hello")>,
                                                   policy::min_count<4>,
                                                   policy::description<AR_STRING("An arg!")>),
                       "--hello,-h [4,N]",
                       "An arg!"},
            std::tuple{multi_arg<std::vector<int>>(policy::short_name<'h'>,
                                                   policy::long_name<AR_STRING("hello")>,
                                                   policy::min_max_count<1, 4>,
                                                   policy::description<AR_STRING("An arg!")>),
                       "--hello,-h [1,4]",
                       "An arg!"},
            std::tuple{multi_arg<std::vector<int>>(policy::long_name<AR_STRING("hello")>,
                                                   policy::description<AR_STRING("An arg!")>),
                       "--hello [1,N]",
                       "An arg!"},
            std::tuple{multi_arg<std::vector<int>>(policy::short_name<'h'>,
                                                   policy::description<AR_STRING("An arg!")>),
                       "-h [1,N]",
                       "An arg!"},
            std::tuple{multi_arg<std::vector<int>>(policy::short_name<'h'>), "-h [1,N]", ""},
            std::tuple{multi_arg<std::vector<int>>(AR_STRING("h"){},
                                                   AR_STRING("hello"){},
                                                   AR_STRING("An arg!"){}),
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
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = multi_arg<std::vector<int>>(
        policy::long_name<AR_STRING("hello")>,
        flag(policy::short_name<'b'>),
        policy::short_name<'H'>
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
#include "arg_router/multi_arg.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = multi_arg<std::vector<int>>(policy::long_name<AR_STRING("hello")>,
                      policy::display_name<AR_STRING("hello2")>);
    return 0;
}
    )",
            "Multi arg must not have a display name policy",
            "must_not_have_display_name_test"},
        {
            R"(
#include "arg_router/multi_arg.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = multi_arg<std::vector<int>>(policy::long_name<AR_STRING("hello")>,
                      policy::none_name<AR_STRING("hello2")>);
    return 0;
}
    )",
            "Multi arg must not have a none name policy",
            "must_not_have_none_name_test"},
        {
            R"(
#include "arg_router/multi_arg.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = multi_arg<std::vector<int>>(policy::long_name<AR_STRING("hello")>,
                                         policy::min_count<0>);
    return 0;
}
    )",
            "Multi arg requires a minimum of one value token, use min_max_count_t to define the "
            "range",
            "minimum_of_one_value_token"},
        {
            R"(
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/multi_arg.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = multi_arg<std::vector<int>>(policy::long_name<AR_STRING("--")>,
                                         policy::router{[](int) {}});
    return 0;
}
    )",
            "Multi arg does not support policies with routing phases (e.g. router)",
            "routing_phase_test"},
    });
}

BOOST_AUTO_TEST_SUITE_END()
