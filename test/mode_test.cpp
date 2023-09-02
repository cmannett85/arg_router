// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/mode.hpp"
#include "arg_router/arg.hpp"
#include "arg_router/counting_flag.hpp"
#include "arg_router/dependency/alias_group.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/list.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/positional_arg.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
namespace ard = arg_router::dependency;
using namespace std::string_literals;

namespace
{
void check_tree(const help_data::type& child_a, const help_data::type& child_b)
{
    BOOST_CHECK_EQUAL(child_a.label, child_b.label);
    BOOST_CHECK_EQUAL(child_a.description, child_b.description);

    BOOST_CHECK_EQUAL(child_a.children.size(), child_b.children.size());
    for (auto i = 0u; i < std::min(child_a.children.size(), child_b.children.size()); ++i) {
        check_tree(child_a.children[i], child_b.children[i]);
    }
}

template <typename Label, typename Description, typename Children>
struct test_help_data {
    using label = Label;
    using description = Description;
    using children = Children;
};

struct pre_parse_data {
    std::size_t hash_code;
    std::vector<parsing::token_type> tokens;
};
}  // namespace

BOOST_AUTO_TEST_SUITE(mode_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<arg_router::mode_t<flag_t<policy::long_name_t<str<"hello">>>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(anonymous_test)
{
    static_assert(!arg_router::mode_t<policy::none_name_t<str<"mode">>,
                                      flag_t<policy::long_name_t<str<"hello">>>>::is_anonymous,
                  "Fail");
    static_assert(arg_router::mode_t<flag_t<policy::long_name_t<str<"hello">>>>::is_anonymous,
                  "Fail");
}

BOOST_AUTO_TEST_CASE(anonymous_single_flag_pre_parse_test)
{
    const auto m = mode(flag(policy::long_name_t{"hello"_S},
                             policy::short_name_t{"l"_S},
                             policy::description_t{"Hello arg"_S}),
                        policy::router{[](auto) {}});

    auto f = [&](auto args, auto expected_args, auto expected_result, auto ec) {
        try {
            auto result = m.pre_parse(parsing::pre_parse_data{args});

            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(args, expected_args);
            BOOST_REQUIRE_EQUAL(expected_result, !!result);

            BOOST_CHECK(result->tokens().empty());
            BOOST_CHECK_EQUAL(result->node_type(), utility::type_hash<std::decay_t<decltype(m)>>());
            if (expected_result) {
                BOOST_REQUIRE_EQUAL(result->sub_targets().size(), 1);
                auto& sub_target = result->sub_targets().front();
                BOOST_CHECK_EQUAL(
                    sub_target.node_type(),
                    utility::type_hash<std::decay_t<decltype(test::get_node<0>(m))>>());
                BOOST_CHECK(sub_target.tokens().empty());
            } else {
                BOOST_CHECK(result->sub_targets().empty());
            }
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       true,
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"},
                                                        {parsing::prefix_type::none, "--goodbye"}},
                       std::vector<parsing::token_type>{},
                       false,
                       std::optional<multi_lang_exception>{
                           multi_lang_exception{error_code::unhandled_arguments,
                                                {parsing::prefix_type::none, "--goodbye"}}}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "-l"}},
                       std::vector<parsing::token_type>{},
                       true,
                       std::optional<multi_lang_exception>{}},
            std::tuple{
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--goodbye"}},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--goodbye"}},
                false,
                std::optional<multi_lang_exception>{multi_lang_exception{
                    error_code::unknown_argument_with_suggestion,
                    std::vector<parsing::token_type>{{parsing::prefix_type::none, "--goodbye"},
                                                     {parsing::prefix_type::long_, "hello"}}}}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"}},
                       false,
                       std::optional<multi_lang_exception>{multi_lang_exception{
                           error_code::unknown_argument_with_suggestion,
                           std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                            {parsing::prefix_type::short_, "l"}}}}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"},
                                                        {parsing::prefix_type::none, "--goodbye"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--goodbye"}},
                       false,
                       std::optional<multi_lang_exception>{
                           multi_lang_exception{error_code::unhandled_arguments,
                                                {parsing::prefix_type::none, "--goodbye"}}}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       false,
                       std::optional<multi_lang_exception>{
                           multi_lang_exception{error_code::argument_has_already_been_set,
                                                {parsing::prefix_type::none, "--hello"}}}},
        });
}

BOOST_AUTO_TEST_CASE(anonymous_single_flag_parse_test)
{
    auto result = false;
    const auto m = mode(flag(policy::long_name_t{"hello"_S},
                             policy::short_name_t{"l"_S},
                             policy::description_t{"Hello arg"_S}),
                        policy::router([&](bool f1) { result = f1; }));

    auto target = parsing::parse_target{m};
    target.add_sub_target({{{parsing::prefix_type::long_, "hello"}}, test::get_node<0>(m)});
    target();
    BOOST_CHECK(result);

    result = false;
    target = parsing::parse_target{m};
    target.add_sub_target({{{parsing::prefix_type::short_, "l"}}, test::get_node<0>(m)});
    target();
    BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE(anonymous_single_positional_single_count_arg_parse_test)
{
    auto result = 0;
    const auto m = mode(positional_arg<int>(policy::display_name_t{"hello"_S},
                                            policy::description_t{"Hello arg"_S},
                                            policy::fixed_count<1>),
                        policy::router([&](int f1) { result = f1; }));

    auto target = parsing::parse_target{m};
    target.add_sub_target({{{parsing::prefix_type::none, "42"}}, test::get_node<0>(m)});
    target();
    BOOST_CHECK_EQUAL(result, 42);

    result = 0;
    target = parsing::parse_target{m};
    target.add_sub_target({{{parsing::prefix_type::none, "hello"}}, test::get_node<0>(m)});
    BOOST_CHECK_EXCEPTION(  //
        target(),
        multi_lang_exception,
        [](const auto& e) {
            return (e.ec() == error_code::failed_to_parse) && (e.tokens().size() == 1) &&
                   (e.tokens().front() == parsing::token_type{parsing::prefix_type::none, "hello"});
        });
    BOOST_CHECK_EQUAL(result, 0);
}

BOOST_AUTO_TEST_CASE(anonymous_triple_child_pre_parse_test)
{
    const auto m = mode(
        flag(policy::long_name_t{"hello"_S},
             policy::short_name_t{"l"_S},
             policy::description_t{"Hello arg"_S}),
        arg<int>(policy::long_name_t{"フー"_S},
                 policy::description_t{"フー arg"_S},
                 policy::default_value{42}),
        counting_flag<std::size_t>(policy::short_name_t{"b"_S}, policy::description_t{"b arg"_S}),
        policy::router{[](bool, int, std::size_t) {}});

    auto f = [&](auto args, auto expected_args, const auto& expected_results, auto ec) {
        try {
            auto result = m.pre_parse(parsing::pre_parse_data{args});

            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(args, expected_args);
            BOOST_REQUIRE_EQUAL(expected_results.empty(), !result);

            BOOST_CHECK(result->tokens().empty());
            BOOST_CHECK_EQUAL(result->node_type(), utility::type_hash<std::decay_t<decltype(m)>>());

            BOOST_REQUIRE_EQUAL(result->sub_targets().size(), expected_results.size());
            for (auto i = 0u; i < expected_results.size(); ++i) {
                auto& sub_target = result->sub_targets()[i];
                BOOST_CHECK_EQUAL(sub_target.node_type(), expected_results[i].hash_code);
                BOOST_CHECK_EQUAL(sub_target.tokens(), expected_results[i].tokens);
            }
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<0>(m), std::vector<parsing::token_type>{}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "-l"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<0>(m), std::vector<parsing::token_type>{}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--フー"},
                                                        {parsing::prefix_type::none, "42"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<1>(m),
                            std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "-b"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<2>(m), std::vector<parsing::token_type>{}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "-b"},
                                                        {parsing::prefix_type::none, "-b"},
                                                        {parsing::prefix_type::none, "-b"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<2>(m), std::vector<parsing::token_type>{}},
                           {test::get_type_index<2>(m), std::vector<parsing::token_type>{}},
                           {test::get_type_index<2>(m), std::vector<parsing::token_type>{}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "-f"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-f"}},
                       std::vector<pre_parse_data>{},
                       std::optional<multi_lang_exception>{
                           multi_lang_exception{error_code::unknown_argument_with_suggestion,
                                                std::vector<parsing::token_type>{
                                                    {parsing::prefix_type::none, "-f"},
                                                    {parsing::prefix_type::short_, "l"},
                                                }}}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "-l"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-l"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       std::vector<pre_parse_data>{},
                       std::optional<multi_lang_exception>{
                           multi_lang_exception{error_code::argument_has_already_been_set,
                                                {parsing::prefix_type::none, "--hello"}}}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "-b"},
                                                        {parsing::prefix_type::none, "--フー"},
                                                        {parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "-b"},
                                                        {parsing::prefix_type::none, "-l"},
                                                        {parsing::prefix_type::none, "-b"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<2>(m), std::vector<parsing::token_type>{}},
                           {test::get_type_index<1>(m),
                            std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}}},
                           {test::get_type_index<2>(m), std::vector<parsing::token_type>{}},
                           {test::get_type_index<0>(m), std::vector<parsing::token_type>{}},
                           {test::get_type_index<2>(m), std::vector<parsing::token_type>{}},
                       },
                       std::optional<multi_lang_exception>{}},
        });
}

BOOST_AUTO_TEST_CASE(anonymous_triple_child_parse_test)
{
    auto result = std::optional<std::tuple<bool, int, bool>>{};
    const auto m = mode(flag(policy::long_name_t{"hello"_S},
                             policy::short_name_t{"l"_S},
                             policy::description_t{"Hello arg"_S}),
                        arg<int>(policy::long_name_t{"foo"_S},
                                 policy::description_t{"Foo arg"_S},
                                 policy::default_value{42}),
                        flag(policy::short_name_t{"b"_S}, policy::description_t{"b arg"_S}),
                        policy::router([&](bool f1, int f2, bool f3) {
                            result = std::tuple{f1, f2, f3};
                        }));

    auto f = [&](auto tokens, auto expected_result, auto ec) {
        result.reset();
        try {
            auto target = m.pre_parse(parsing::pre_parse_data{tokens});
            BOOST_REQUIRE(target);

            (*target)();
            BOOST_CHECK(!ec);
            BOOST_REQUIRE(result);
            BOOST_CHECK_EQUAL(std::get<0>(*result), std::get<0>(expected_result));
            BOOST_CHECK_EQUAL(std::get<1>(*result), std::get<1>(expected_result));
            BOOST_CHECK_EQUAL(std::get<2>(*result), std::get<2>(expected_result));
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector<parsing::token_type>{},
                       std::tuple{false, 42, false},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "hello"}},
                       std::tuple{true, 42, false},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::short_, "l"}},
                       std::tuple{true, 42, false},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "foo"},
                                                        {parsing::prefix_type::none, "13"}},
                       std::tuple{false, 13, false},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::short_, "b"}},
                       std::tuple{false, 42, true},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "hello"},
                                                        {parsing::prefix_type::short_, "b"}},
                       std::tuple{true, 42, true},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::short_, "l"},
                                                        {parsing::prefix_type::short_, "b"},
                                                        {parsing::prefix_type::long_, "foo"},
                                                        {parsing::prefix_type::none, "48"}},
                       std::tuple{true, 48, true},
                       std::optional<multi_lang_exception>{}},
        });
}

BOOST_AUTO_TEST_CASE(named_single_flag_parse_test)
{
    auto result = std::optional<bool>{};
    const auto m = mode("my-mode"_S,
                        flag(policy::long_name_t{"hello"_S},
                             policy::short_name_t{"l"_S},
                             policy::description_t{"Hello arg"_S}),
                        policy::router([&](bool f1) { result = f1; }));

    auto f = [&](auto tokens, auto expected_result, auto ec) {
        result.reset();
        try {
            auto target = m.pre_parse(parsing::pre_parse_data{tokens});
            BOOST_REQUIRE(target);

            (*target)();
            BOOST_CHECK(!ec);
            BOOST_REQUIRE(result);
            BOOST_CHECK_EQUAL(*result, expected_result);
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"}},
                       std::optional<bool>{false},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::long_, "hello"}},
                       std::optional<bool>{true},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::short_, "l"}},
                       std::optional<bool>{true},
                       std::optional<multi_lang_exception>{}},
        });
}

BOOST_AUTO_TEST_CASE(named_triple_arg_pre_parse_test)
{
    const auto m = mode("my-mode"_S,
                        flag(policy::long_name_t{"hello"_S},
                             policy::short_name_t{"l"_S},
                             policy::description_t{"Hello arg"_S}),
                        arg<int>(policy::long_name_t{"foo"_S},
                                 policy::description_t{"Foo arg"_S},
                                 policy::default_value{42}),
                        flag(policy::short_name_t{"b"_S}, policy::description_t{"b arg"_S}),
                        policy::router{[](bool, int, bool) {}});

    auto f = [&](auto args, auto expected_args, const auto& expected_results, auto ec) {
        try {
            auto result = m.pre_parse(parsing::pre_parse_data{args});

            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(args, expected_args);
            BOOST_CHECK_EQUAL(expected_results.empty(), !result);

            if (result) {
                BOOST_CHECK(result->tokens().empty());
                BOOST_CHECK_EQUAL(result->node_type(),
                                  utility::type_hash<std::decay_t<decltype(m)>>());

                BOOST_REQUIRE_EQUAL(result->sub_targets().size(), expected_results.size());
                for (auto i = 0u; i < expected_results.size(); ++i) {
                    auto& sub_target = result->sub_targets()[i];
                    BOOST_CHECK_EQUAL(sub_target.node_type(), expected_results[i].hash_code);
                    BOOST_CHECK_EQUAL(sub_target.tokens(), expected_results[i].tokens);
                }
            }
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<0>(m), std::vector<parsing::token_type>{}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "-l"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<0>(m), std::vector<parsing::token_type>{}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "--foo"},
                                                        {parsing::prefix_type::none, "42"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<1>(m),
                            std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "-b"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<2>(m), std::vector<parsing::token_type>{}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "-f"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "-f"}},
                       std::vector<pre_parse_data>{},
                       std::optional<multi_lang_exception>{
                           multi_lang_exception{error_code::unknown_argument_with_suggestion,
                                                std::vector<parsing::token_type>{
                                                    {parsing::prefix_type::none, "-f"},
                                                    {parsing::prefix_type::short_, "l"},
                                                }}}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "-l"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "-l"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       std::vector<pre_parse_data>{},
                       std::optional<multi_lang_exception>{
                           multi_lang_exception{error_code::argument_has_already_been_set,
                                                {parsing::prefix_type::none, "--hello"}}}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "--foo"},
                                                        {parsing::prefix_type::none, "42"},
                                                        {parsing::prefix_type::none, "-l"},
                                                        {parsing::prefix_type::none, "-b"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<1>(m),
                            std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}}},
                           {test::get_type_index<0>(m), std::vector<parsing::token_type>{}},
                           {test::get_type_index<2>(m), std::vector<parsing::token_type>{}},
                       },
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "-b"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-b"}},
                       std::vector<pre_parse_data>{},
                       std::optional<multi_lang_exception>{}},
        });
}

BOOST_AUTO_TEST_CASE(named_triple_arg_parse_test)
{
    auto result = std::optional<std::tuple<bool, int, bool>>{};
    const auto m = mode("my-mode"_S,
                        flag(policy::long_name_t{"hello"_S},
                             policy::short_name_t{"l"_S},
                             policy::description_t{"Hello arg"_S}),
                        arg<int>(policy::long_name_t{"フー"_S},
                                 policy::description_t{"Foo arg"_S},
                                 policy::default_value{42}),
                        flag(policy::short_name_t{"b"_S}, policy::description_t{"b arg"_S}),
                        policy::router([&](bool f1, int f2, bool f3) {
                            result = std::tuple{f1, f2, f3};
                        }));

    auto f = [&](auto tokens, auto expected_result, auto ec) {
        result.reset();
        try {
            auto target = m.pre_parse(parsing::pre_parse_data{tokens});
            BOOST_REQUIRE(target);

            (*target)();
            BOOST_CHECK(!ec);
            BOOST_REQUIRE(result);
            BOOST_CHECK_EQUAL(std::get<0>(*result), std::get<0>(expected_result));
            BOOST_CHECK_EQUAL(std::get<1>(*result), std::get<1>(expected_result));
            BOOST_CHECK_EQUAL(std::get<2>(*result), std::get<2>(expected_result));
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"}},
                       std::tuple{false, 42, false},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::long_, "hello"}},
                       std::tuple{true, 42, false},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::short_, "l"}},
                       std::tuple{true, 42, false},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::long_, "フー"},
                                                        {parsing::prefix_type::none, "13"}},
                       std::tuple{false, 13, false},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::short_, "b"}},
                       std::tuple{false, 42, true},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::long_, "hello"},
                                                        {parsing::prefix_type::short_, "b"}},
                       std::tuple{true, 42, true},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::short_, "l"},
                                                        {parsing::prefix_type::short_, "b"},
                                                        {parsing::prefix_type::long_, "フー"},
                                                        {parsing::prefix_type::none, "48"}},
                       std::tuple{true, 48, true},
                       std::optional<multi_lang_exception>{}},
        });
}

BOOST_AUTO_TEST_CASE(anonymous_triple_flag_single_list_pre_parse_test)
{
    const auto flags = list{
        flag(policy::long_name_t{"hello"_S},
             policy::short_name_t{"l"_S},
             policy::description_t{"Hello arg"_S}),
        flag(policy::long_name_t{"foo"_S}, policy::description_t{"Foo arg"_S}),
        flag(policy::short_name_t{"b"_S}, policy::description_t{"b arg"_S}),
    };
    const auto m = mode(flags, policy::router{[](bool, bool, bool) {}});

    auto f = [&](auto args, auto expected_args, const auto& expected_results, auto ec) {
        try {
            auto result = m.pre_parse(parsing::pre_parse_data{args});

            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(args, expected_args);
            BOOST_REQUIRE_EQUAL(expected_results.empty(), !result);

            BOOST_CHECK(result->tokens().empty());
            BOOST_CHECK_EQUAL(result->node_type(), utility::type_hash<std::decay_t<decltype(m)>>());

            BOOST_REQUIRE_EQUAL(result->sub_targets().size(), expected_results.size());
            for (auto i = 0u; i < expected_results.size(); ++i) {
                auto& sub_target = result->sub_targets()[i];
                BOOST_CHECK_EQUAL(sub_target.node_type(), expected_results[i].hash_code);
                BOOST_CHECK_EQUAL(sub_target.tokens(), expected_results[i].tokens);
            }
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<0>(m), std::vector<parsing::token_type>{}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "-l"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<0>(m), std::vector<parsing::token_type>{}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--goodbye"}},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--goodbye"}},
                std::vector<pre_parse_data>{},
                std::optional<multi_lang_exception>{multi_lang_exception{
                    error_code::unknown_argument_with_suggestion,
                    std::vector<parsing::token_type>{{parsing::prefix_type::none, "--goodbye"},
                                                     {parsing::prefix_type::long_, "foo"}}}}},
        });
}

BOOST_AUTO_TEST_CASE(named_triple_flag_double_list_pre_parse_test)
{
    const auto list1 = list{flag(policy::long_name_t{"hello"_S},
                                 policy::short_name_t{"l"_S},
                                 policy::description_t{"Hello arg"_S}),
                            flag(policy::long_name_t{"foo"_S}, policy::description_t{"Foo arg"_S})};
    const auto list2 = list{flag(policy::short_name_t{"b"_S}, policy::description_t{"b arg"_S})};
    const auto m =
        mode(policy::none_name_t{"my-mode"_S}, list1, list2, policy::router{[](bool, bool, bool) {
             }});

    auto f = [&](auto args, auto expected_args, const auto& expected_results, auto ec) {
        try {
            auto result = m.pre_parse(parsing::pre_parse_data{args});

            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(args, expected_args);
            BOOST_CHECK_EQUAL(expected_results.empty(), !result);

            if (result) {
                BOOST_CHECK(result->tokens().empty());
                BOOST_CHECK_EQUAL(result->node_type(),
                                  utility::type_hash<std::decay_t<decltype(m)>>());

                BOOST_REQUIRE_EQUAL(result->sub_targets().size(), expected_results.size());
                for (auto i = 0u; i < expected_results.size(); ++i) {
                    auto& sub_target = result->sub_targets()[i];
                    BOOST_CHECK_EQUAL(sub_target.node_type(), expected_results[i].hash_code);
                    BOOST_CHECK_EQUAL(sub_target.tokens(), expected_results[i].tokens);
                }
            }
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<0>(m), std::vector<parsing::token_type>{}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "-l"}},
                       std::vector<parsing::token_type>{},
                       std::vector<pre_parse_data>{
                           {test::get_type_index<0>(m), std::vector<parsing::token_type>{}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "--goodbye"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "my-mode"},
                                                        {parsing::prefix_type::none, "--goodbye"}},
                       std::vector<pre_parse_data>{},
                       std::optional<multi_lang_exception>{
                           multi_lang_exception{error_code::unknown_argument_with_suggestion,
                                                std::vector<parsing::token_type>{
                                                    {parsing::prefix_type::none, "--goodbye"},
                                                    {parsing::prefix_type::long_, "foo"},
                                                }}}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "wrong-mode"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "wrong-mode"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       std::vector<pre_parse_data>{},
                       std::optional<multi_lang_exception>{}},
        });
}

BOOST_AUTO_TEST_CASE(nested_modes_pre_parse_test)
{
    const auto m = mode(policy::none_name_t{"mode1"_S},
                        mode(policy::none_name_t{"mode2"_S},
                             mode(policy::none_name_t{"mode3"_S},
                                  flag(policy::long_name_t{"hello"_S},
                                       policy::short_name_t{"l"_S},
                                       policy::description_t{"Hello arg"_S}),
                                  arg<int>(policy::long_name_t{"フー"_S},
                                           policy::description_t{"Foo arg"_S},
                                           policy::default_value{42}),
                                  policy::router{[](bool, int) {}})));

    auto f = [&](auto args,
                 auto expected_args,
                 auto expected_hash,
                 const auto& expected_results,
                 auto ec) {
        try {
            auto result = m.pre_parse(parsing::pre_parse_data{args});

            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(args, expected_args);
            BOOST_CHECK_EQUAL(!expected_results, !result);

            if (result) {
                BOOST_CHECK(result->tokens().empty());
                BOOST_CHECK_EQUAL(result->node_type(), expected_hash);

                if (expected_results) {
                    BOOST_REQUIRE_EQUAL(result->sub_targets().size(), expected_results->size());
                    for (auto i = 0u; i < expected_results->size(); ++i) {
                        auto& sub_target = result->sub_targets()[i];
                        BOOST_CHECK_EQUAL(sub_target.node_type(), (*expected_results)[i].hash_code);
                        BOOST_CHECK_EQUAL(sub_target.tokens(), (*expected_results)[i].tokens);
                    }
                }
            }
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"}},
                       std::vector<parsing::token_type>{},
                       utility::type_hash<std::decay_t<decltype(m)>>(),
                       std::optional{std::vector<pre_parse_data>{}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode2"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode2"}},
                       std::size_t{0},
                       std::optional<std::vector<pre_parse_data>>{},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "mode2"}},
                       std::vector<parsing::token_type>{},
                       test::get_type_index<0>(m),
                       std::optional{std::vector<pre_parse_data>{}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "mode9"}},
                       std::vector<parsing::token_type>{},
                       std::size_t{0},
                       std::optional{std::vector<pre_parse_data>{}},
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::unknown_argument_with_suggestion,
                                                  {"mode9", "mode2"})}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "mode2"},
                                                        {parsing::prefix_type::none, "mode3"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{},
                       test::get_type_index<0, 0>(m),
                       std::optional{std::vector<pre_parse_data>{
                           {test::get_type_index<0, 0, 0>(m), std::vector<parsing::token_type>{}}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "mode2"},
                                                        {parsing::prefix_type::none, "mode3"},
                                                        {parsing::prefix_type::none, "-l"}},
                       std::vector<parsing::token_type>{},
                       test::get_type_index<0, 0>(m),
                       std::optional{std::vector<pre_parse_data>{
                           {test::get_type_index<0, 0, 0>(m), std::vector<parsing::token_type>{}}}},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "mode2"},
                                                        {parsing::prefix_type::none, "mode3"},
                                                        {parsing::prefix_type::none, "-l"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "mode2"},
                                                        {parsing::prefix_type::none, "mode3"},
                                                        {parsing::prefix_type::none, "-l"},
                                                        {parsing::prefix_type::none, "--hello"}},
                       test::get_type_index<0, 0>(m),
                       std::optional<std::vector<pre_parse_data>>{},
                       std::optional<multi_lang_exception>{
                           multi_lang_exception{error_code::argument_has_already_been_set,
                                                {parsing::prefix_type::none, "--hello"}}}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "mode2"},
                                                        {parsing::prefix_type::none, "mode3"},
                                                        {parsing::prefix_type::none, "--フー"},
                                                        {parsing::prefix_type::none, "42"}},
                       std::vector<parsing::token_type>{},
                       test::get_type_index<0, 0>(m),
                       std::optional{std::vector<pre_parse_data>{
                           {test::get_type_index<0, 0, 1>(m),
                            std::vector<parsing::token_type>{{parsing::prefix_type::none, "42"}}}}},
                       std::optional<multi_lang_exception>{}},
        });
}

BOOST_AUTO_TEST_CASE(nested_modes_parse_test)
{
    auto result = std::optional<std::tuple<bool, int>>{};
    const auto m = mode(policy::none_name_t{"mode1"_S},
                        mode(policy::none_name_t{"mode2"_S},
                             mode(policy::none_name_t{"mode3"_S},
                                  flag(policy::long_name_t{"hello"_S},
                                       policy::short_name_t{"l"_S},
                                       policy::description_t{"Hello arg"_S}),
                                  arg<int>(policy::long_name_t{"フー"_S},
                                           policy::description_t{"Foo arg"_S},
                                           policy::default_value{42}),
                                  policy::router([&](bool f1, int f2) {
                                      result = std::tuple{f1, f2};
                                  }))));

    auto f = [&](auto tokens, auto expected_result, auto ec) {
        result.reset();
        try {
            auto target = m.pre_parse(parsing::pre_parse_data{tokens});
            BOOST_REQUIRE(target);

            (*target)();
            BOOST_CHECK(!ec);
            BOOST_REQUIRE(result);
            BOOST_CHECK_EQUAL(std::get<0>(*result), std::get<0>(expected_result));
            BOOST_CHECK_EQUAL(std::get<1>(*result), std::get<1>(expected_result));
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"}},
                       std::tuple{false, 42},
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::mode_requires_arguments, {"mode1"})}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "mode2"}},
                       std::tuple{false, 42},
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::mode_requires_arguments, {"mode2"})}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "mode2"},
                                                        {parsing::prefix_type::none, "mode3"}},
                       std::tuple{false, 42},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "mode2"},
                                                        {parsing::prefix_type::none, "mode3"},
                                                        {parsing::prefix_type::long_, "hello"}},
                       std::tuple{true, 42},
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "mode2"},
                                                        {parsing::prefix_type::none, "mode3"},
                                                        {parsing::prefix_type::long_, "hello"},
                                                        {parsing::prefix_type::long_, "フー"},
                                                        {parsing::prefix_type::none, "13"}},
                       std::tuple{true, 13},
                       std::optional<multi_lang_exception>{}},
        });
}

BOOST_AUTO_TEST_CASE(no_missing_phase_test)
{
    {
        auto result = 42;
        const auto m = mode(arg<int>(policy::long_name_t{"hello"_S}),
                            policy::router([&](int arg1) { result = arg1; }));

        m.parse({{}, m});
        BOOST_CHECK_EQUAL(result, 0);
    }

    {
        auto result = 3.14;
        const auto m = mode(arg<int>(policy::long_name_t{"hello"_S}),
                            policy::router([&](double arg1) { result = arg1; }));

        m.parse({{}, m});
        BOOST_CHECK_EQUAL(result, 0.0);
    }

    {
        auto result = std::vector<int>{3, 4, 5};
        const auto m = mode(positional_arg<std::vector<int>>(policy::display_name_t{"hello"_S}),
                            policy::router([&](std::vector<int> arg1) { result = arg1; }));

        m.parse({{}, m});
        BOOST_CHECK_EQUAL(result, std::vector<int>{});
    }
}

BOOST_AUTO_TEST_CASE(help_test)
{
    auto f = [](const auto& node, auto flatten, const auto& expected) {
        check_tree(help_data::generate<flatten>(node), expected);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                mode(flag(policy::long_name_t{"hello"_S},
                          policy::short_name_t{"h"_S},
                          policy::description_t{"Hello desc"_S}),
                     policy::router([](bool) {})),
                std::true_type{},
                help_data::type{" ",
                                "",
                                std::vector<help_data::type>{
                                    {"--hello,-h", "Hello desc", std::vector<help_data::type>{}}}}},
            std::tuple{
                mode(flag(policy::long_name_t{"hello"_S},
                          policy::short_name_t{"h"_S},
                          policy::description_t{"Hello desc"_S}),
                     policy::router([](bool) {})),
                std::false_type{},
                help_data::type{" ",
                                "",
                                std::vector<help_data::type>{
                                    {"--hello,-h", "Hello desc", std::vector<help_data::type>{}}}}},
            std::tuple{
                mode(flag(policy::long_name_t{"hello"_S},
                          policy::short_name_t{"h"_S},
                          policy::description_t{"Hello desc"_S}),
                     flag(policy::long_name_t{"flag1"_S},
                          policy::short_name_t{"a"_S},
                          policy::description_t{"Flag1 desc"_S}),
                     policy::router([](bool, bool) {})),
                std::true_type{},
                help_data::type{" ",
                                "",
                                std::vector<help_data::type>{
                                    {"--hello,-h", "Hello desc", std::vector<help_data::type>{}},
                                    {"--flag1,-a", "Flag1 desc", std::vector<help_data::type>{}}}}},
            std::tuple{mode("mode1"_S,
                            "Mode desc"_S,
                            flag(policy::long_name_t{"hello"_S},
                                 policy::short_name_t{"h"_S},
                                 policy::description_t{"Hello desc"_S}),
                            flag(policy::long_name_t{"flag1"_S},
                                 policy::short_name_t{"a"_S},
                                 policy::description_t{"Flag1 desc"_S}),
                            policy::router([](bool, bool) {})),
                       std::false_type{},
                       help_data::type{"mode1", "Mode desc", std::vector<help_data::type>{}}},
            std::tuple{
                mode(policy::none_name_t{"mode1"_S},
                     policy::description_t{"Mode desc"_S},
                     flag(policy::long_name_t{"hello"_S},
                          policy::short_name_t{"h"_S},
                          policy::description_t{"Hello desc"_S}),
                     flag(policy::long_name_t{"flag1"_S},
                          policy::short_name_t{"a"_S},
                          policy::description_t{"Flag1 desc"_S}),
                     policy::router([](bool, bool) {})),
                std::true_type{},
                help_data::type{"mode1",
                                "Mode desc",
                                std::vector<help_data::type>{
                                    {"--hello,-h", "Hello desc", std::vector<help_data::type>{}},
                                    {"--flag1,-a", "Flag1 desc", std::vector<help_data::type>{}}}}},
            std::tuple{
                mode(policy::none_name_t{"mode1"_S},
                     policy::description_t{"Mode1 desc"_S},
                     flag(policy::long_name_t{"hello"_S},
                          policy::short_name_t{"h"_S},
                          policy::description_t{"Hello desc"_S}),
                     mode(policy::none_name_t{"mode2"_S},
                          policy::description_t{"Mode2 desc"_S},
                          flag(policy::long_name_t{"goodbye"_S},
                               policy::short_name_t{"g"_S},
                               policy::description_t{"Goodbye desc"_S}),
                          flag(policy::long_name_t{"flag2"_S},
                               policy::short_name_t{"b"_S},
                               policy::description_t{"Flag2 desc"_S})),
                     policy::router([](bool) {})),
                std::true_type{},
                help_data::type{
                    "mode1",
                    "Mode1 desc",
                    std::vector<help_data::type>{
                        {"--hello,-h", "Hello desc", std::vector<help_data::type>{}},
                        {"mode2",
                         "Mode2 desc",
                         std::vector<help_data::type>{
                             {"--goodbye,-g", "Goodbye desc", std::vector<help_data::type>{}},
                             {"--flag2,-b", "Flag2 desc", std::vector<help_data::type>{}}}}}}},
            std::tuple{mode(policy::none_name_t{"mode1"_S},
                            policy::description_t{"Mode1 desc"_S},
                            flag(policy::long_name_t{"hello"_S},
                                 policy::short_name_t{"h"_S},
                                 policy::description_t{"Hello desc"_S}),
                            mode(policy::none_name_t{"mode2"_S},
                                 policy::description_t{"Mode2 desc"_S},
                                 flag(policy::long_name_t{"goodbye"_S},
                                      policy::short_name_t{"g"_S},
                                      policy::description_t{"Goodbye desc"_S}),
                                 flag(policy::long_name_t{"flag2"_S},
                                      policy::short_name_t{"b"_S},
                                      policy::description_t{"Flag2 desc"_S})),
                            policy::router([](bool) {})),
                       std::false_type{},
                       help_data::type{"mode1", "Mode1 desc", std::vector<help_data::type>{}}},
        });
}

BOOST_AUTO_TEST_CASE(multi_stage_alias_group_test)
{
    auto result = 0;
    const auto m = mode(ard::alias_group(arg<int>(policy::long_name_t{"arg"_S}),
                                         counting_flag<int>(policy::short_name_t{"a"_S}),
                                         policy::required),
                        policy::router([&](int value) { result = value; }));

    auto tokens = std::vector<parsing::token_type>{{parsing::prefix_type::long_, "arg"},
                                                   {parsing::prefix_type::none, "5"},
                                                   {parsing::prefix_type::short_, "a"}};

    auto target = m.pre_parse(parsing::pre_parse_data{tokens});
    BOOST_REQUIRE(target);

    (*target)();
    BOOST_CHECK_EQUAL(result, 6);
}

BOOST_AUTO_TEST_CASE(multi_stage_validated_alias_group_test)
{
    auto result = 0;
    const auto m = mode(ard::alias_group(arg<int>(policy::long_name_t{"arg"_S}),
                                         counting_flag<int>(policy::short_name_t{"a"_S}),
                                         policy::min_max_value<1, 3>(),
                                         policy::required),
                        policy::router([&](int value) { result = value; }));

    auto f = [&](auto tokens, auto expected_result, auto ec) {
        result = 0;
        try {
            auto target = m.pre_parse(parsing::pre_parse_data{tokens});
            BOOST_REQUIRE(target);

            (*target)();
            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(result, expected_result);
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "arg"},
                                                        {parsing::prefix_type::none, "1"}},
                       1,
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "arg"},
                                                        {parsing::prefix_type::none, "3"}},
                       3,
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "arg"},
                                                        {parsing::prefix_type::none, "0"}},
                       0,
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::minimum_value_not_reached,
                                                  {"Alias Group: --arg,-a"})}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "arg"},
                                                        {parsing::prefix_type::none, "5"}},
                       0,
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::maximum_value_exceeded,
                                                  {"Alias Group: --arg,-a"})}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::short_, "a"}},
                       1,
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::short_, "a"},
                                                        {parsing::prefix_type::short_, "a"}},
                       2,
                       std::optional<multi_lang_exception>{}},
            std::tuple{std::vector<parsing::token_type>{},
                       0,
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::missing_required_argument,
                                                  {"Alias Group: --arg,-a"})}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::short_, "a"},
                                                        {parsing::prefix_type::short_, "a"},
                                                        {parsing::prefix_type::short_, "a"},
                                                        {parsing::prefix_type::short_, "a"}},
                       0,
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::maximum_value_exceeded,
                                                  {"Alias Group: --arg,-a"})}},
            std::tuple{std::vector<parsing::token_type>{{parsing::prefix_type::long_, "arg"},
                                                        {parsing::prefix_type::none, "2"},
                                                        {parsing::prefix_type::short_, "a"},
                                                        {parsing::prefix_type::short_, "a"}},
                       0,
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::maximum_value_exceeded,
                                                  {"Alias Group: --arg,-a"})}},
        });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile(
        {{R"(
#include "arg_router/mode.hpp"

using namespace arg_router;

int main() {
    const auto m = mode();
    return 0;
}
    )",
          "Mode must have at least one child node",
          "no_children_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto tokens = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    const auto m = mode(flag(policy::long_name_t{"hello"_S}));
    auto target = m.pre_parse(parsing::pre_parse_data{tokens});
    (*target)();
    return 0;
}
    )",
             "Anonymous modes must have routing",
             "anonymous_modes_must_have_routing_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = mode(policy::long_name_t{"my-mode"_S},
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
             "Mode must not have a long name policy",
             "must_not_have_a_long_name_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = mode(policy::short_name_t{"l"_S},
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
             "Mode must not have a short name policy",
             "must_not_have_a_short_name_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = mode(policy::display_name_t{"mode"_S},
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
             "Mode must not have a display name policy",
             "must_not_have_a_display_name_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

namespace
{
template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    constexpr explicit stub_node(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }

    void parse(parsing::parse_target target) const
    {
        std::get<0>(this->children()).parse(target, *this);
    }

    template <typename Validator, bool HasTarget, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const
    {
        return std::get<0>(this->children()).pre_parse(
                pre_parse_data,
                *this,
                parents...);
    }
};
} // namespace

int main() {
    const auto m = stub_node(
                        mode(policy::none_name_t{"mode"_S},
                             mode(flag(policy::long_name_t{"hello"_S}),
                                  policy::router([&](bool) {}))));

    auto tokens = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    auto target = m.pre_parse(parsing::pre_parse_data{tokens});
    return 0;
}
    )",
             "Anonymous modes can only exist under the root",
             "anonymous_child_mode_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    [[maybe_unused]] const auto m = mode(
                    flag(policy::long_name_t{"flag"_S}),
                    mode(policy::none_name_t{"mode"_S},
                         flag(policy::long_name_t{"hello"_S})));
    return 0;
}
    )",
             "Anonymous mode cannot have a child mode",
             "anonymous_mode_cannot_have_a_child_mode_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/tree_node.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

namespace
{
template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    constexpr explicit stub_node(Params... params) :
        tree_node<Params...>{std::move(params)...}
    {
    }

    void parse(parsing::parse_target target) const
    {
        std::get<0>(this->children()).parse(target, *this);
    }

    template <typename Validator, bool HasTarget, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const
    {
        return std::get<0>(this->children()).pre_parse(
                pre_parse_data,
                *this,
                parents...);
    }
};
} // namespace

int main() {
    const auto m = stub_node(
                        mode(policy::none_name_t{"mode"_S},
                             flag(policy::long_name_t{"f1"_S}),
                             mode(flag(policy::long_name_t{"f2"_S}),
                                  policy::router([&](bool) {}))));
                             

    auto tokens = std::vector<parsing::token_type>{
                    {parsing::prefix_type::long_, "hello"}};
    auto target = m.pre_parse(parsing::pre_parse_data{tokens});
    (*target)();
    return 0;
}
    )",
             "Mode must have a router or all its children are modes",
             "mode_has_router_or_all_children_are_modes_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = mode(flag(policy::long_name_t{"hello"_S},
                             policy::router([&](bool) {})),
                        policy::router([&](bool) {}));
    return 0;
}
    )",
             "Non-mode children cannot have routing",
             "non_mode_children_cannot_have_children_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = mode(policy::custom_parser<int>{[](auto) { return false; }},
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
             "Mode does not support policies with parse, validation, or missing phases; as it "
             "delegates those to its children",
             "parse_phase_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = mode(policy::min_max_value<1, 3>(),
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
             "Mode does not support policies with parse, validation, or missing phases; as it "
             "delegates those to its children",
             "validation_phase_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/required.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = mode(policy::required,
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
             "Mode does not support policies with parse, validation, or missing phases; as it "
             "delegates those to its children",
             "missing_phase_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto fake_parent = flag(policy::long_name_t{"fake"_S});
    const auto m = mode(flag(policy::long_name_t{"hello"_S}));

    auto tokens = std::vector<parsing::token_type>{
                        {parsing::prefix_type::none, "--hello"}};
    auto result = m.pre_parse(parsing::pre_parse_data{
                    tokens,
                    parsing::parse_target{fake_parent}});

    return 0;
}
    )",
             "Modes cannot receive pre_parse_data containing parent parse_targets",
             "no_parent_parse_target_test"},
         {
             R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/error_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto m = mode(policy::none_name_t{"my-mode"_S},
                        policy::error_name_t{"error!"_S},
                        flag(policy::long_name_t{"hello"_S}));
    return 0;
}
    )",
             "Named modes must not have an error name policy",
             "named_modes_no_error_name_test"}});
}

BOOST_AUTO_TEST_SUITE_END()
