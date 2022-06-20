/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(flag_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<flag_t<policy::long_name_t<S_("hello")>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(policies_test)
{
    [[maybe_unused]] auto f =
        flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
    static_assert(f.long_name() == "hello"sv, "Long name test fail");
    static_assert(f.short_name() == "H", "Short name test fail");
    static_assert(f.minimum_count() == 0, "Minimum count test fail");
    static_assert(f.maximum_count() == 0, "Maximum count test fail");
}

BOOST_AUTO_TEST_CASE(parse_test)
{
    auto router_hit = false;
    auto f = [&](auto node, auto expected_router_hit) {
        router_hit = false;
        auto target = parsing::parse_target{{}, node};
        const auto result = node.parse(std::move(target));
        BOOST_CHECK_EQUAL(result, true);
        BOOST_CHECK_EQUAL(router_hit, expected_router_hit);
    };

    test::data_set(f,
                   std::tuple{std::tuple{flag(policy::short_name<'h'>), false},
                              std::tuple{flag(policy::short_name<'a'>,  //
                                              policy::router{[&](bool result) {
                                                  BOOST_CHECK(result);
                                                  router_hit = true;
                                              }}),
                                         true}});
}

BOOST_AUTO_TEST_CASE(help_test)
{
    auto f = [](const auto& node,
                auto expected_label,
                auto expected_description) {
        using node_type = std::decay_t<decltype(node)>;

        using help_data = typename node_type::template help_data_type<false>;
        using flattened_help_data =
            typename node_type::template help_data_type<true>;

        static_assert(std::is_same_v<typename help_data::label,
                                     typename flattened_help_data::label>);
        static_assert(
            std::is_same_v<typename help_data::description,
                           typename flattened_help_data::description>);
        static_assert(std::tuple_size_v<typename help_data::children> == 0);
        static_assert(
            std::tuple_size_v<typename flattened_help_data::children> == 0);

        BOOST_CHECK_EQUAL(help_data::label::get(), expected_label);
        BOOST_CHECK_EQUAL(help_data::description::get(), expected_description);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{flag(policy::short_name<'h'>,
                                       policy::long_name<S_("hello")>,
                                       policy::description<S_("A flag!")>),
                                  "--hello,-h",
                                  "A flag!"},
                       std::tuple{flag(policy::long_name<S_("hello")>,
                                       policy::description<S_("A flag!")>),
                                  "--hello",
                                  "A flag!"},
                       std::tuple{flag(policy::short_name<'h'>,
                                       policy::description<S_("A flag!")>),
                                  "-h",
                                  "A flag!"},
                       std::tuple{flag(policy::short_name<'h'>), "-h", ""},
                   });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(policies_only_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = flag(
        policy::long_name<S_("hello")>,
        flag(policy::short_name<'b'>),
        policy::short_name<'H'>
    );
    return 0;
}
    )",
        "Flags must only contain policies (not other nodes)");
}

BOOST_AUTO_TEST_CASE(must_be_named_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"

using namespace arg_router;

int main() {
    auto f = flag();
    return 0;
}
    )",
        "Flag must have a long and/or short name policy");
}

BOOST_AUTO_TEST_CASE(must_not_have_display_name_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = flag(policy::long_name<S_("hello")>,
                  policy::display_name<S_("hello2")>);
    return 0;
}
    )",
        "Flag must not have a display name policy");
}

BOOST_AUTO_TEST_CASE(must_not_have_none_name_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = flag(policy::long_name<S_("hello")>,
                  policy::none_name<S_("hello2")>);
    return 0;
}
    )",
        "Flag must not have a none name policy");
}

BOOST_AUTO_TEST_CASE(parse_policy_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = flag(policy::long_name<S_("hello")>,
                  policy::custom_parser<bool>{[](std::string_view) { return true; }});
    return 0;
}
    )",
        "Flag does not support policies with parse or validation phases "
        "(e.g. custom_parser or min_max_value)");
}

BOOST_AUTO_TEST_CASE(validation_policy_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    auto f = flag(policy::long_name<S_("hello")>,
                  policy::min_max_value{true, true});
    return 0;
}
    )",
        "Flag does not support policies with parse or validation phases "
        "(e.g. custom_parser or min_max_value)");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
