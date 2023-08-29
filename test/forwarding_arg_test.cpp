// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/forwarding_arg.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(forwarding_arg_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<forwarding_arg_t<policy::none_name_t<str<"--">>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(policies_test)
{
    [[maybe_unused]] auto f = forwarding_arg(policy::none_name_t{"--"_S});
    static_assert(f.none_name() == "--", "Long name test fail");
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
            std::tuple{forwarding_arg(policy::none_name_t{"--"_S}),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}},
                       std::vector<std::string_view>{"hello"}},
            std::tuple{forwarding_arg("--"_S),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"}},
                       std::vector<std::string_view>{"hello"}},
            std::tuple{forwarding_arg(policy::none_name_t{"--"_S}),
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "hello"},
                                                        {parsing::prefix_type::none, "world"},
                                                        {parsing::prefix_type::none, "goodbye"}},
                       std::vector<std::string_view>{"hello", "world", "goodbye"}},
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

    test::data_set(f,
                   std::tuple{
                       std::tuple{forwarding_arg(policy::none_name_t{"--"_S},
                                                 policy::description_t{"An arg!"_S}),
                                  "-- [0,N]",
                                  "An arg!"},
                       std::tuple{forwarding_arg(policy::none_name_t{"--"_S},
                                                 policy::min_count<4>,
                                                 policy::description_t{"An arg!"_S}),
                                  "-- [4,N]",
                                  "An arg!"},
                       std::tuple{forwarding_arg(policy::none_name_t{"--"_S},
                                                 policy::min_max_count<1, 4>,
                                                 policy::description_t{"An arg!"_S}),
                                  "-- [1,4]",
                                  "An arg!"},
                       std::tuple{forwarding_arg(policy::none_name_t{"--"_S},
                                                 policy::description_t{"An arg!"_S}),
                                  "-- [0,N]",
                                  "An arg!"},
                       std::tuple{forwarding_arg("--"_S, "An arg!"_S), "-- [0,N]", "An arg!"},
                   });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({
        {
            R"(
#include "arg_router/forwarding_arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = forwarding_arg(
        policy::none_name_t{"--"_S},
        flag(policy::short_name_t{"b"_S})
    );
    return 0;
}
    )",
            "Arg must only contain policies (not other nodes)",
            "only_policies_test"},
        {
            R"(
#include "arg_router/forwarding_arg.hpp"

using namespace arg_router;

int main() {
    auto a = forwarding_arg();
    return 0;
}
    )",
            "Arg must be named",
            "must_be_named_test"},
        {
            R"(
#include "arg_router/forwarding_arg.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = forwarding_arg(policy::none_name_t{"--"_S},
                            policy::display_name_t{"hello"_S});
    return 0;
}
    )",
            "Forwarding arg can only have a none name policy",
            "must_not_have_display_name_test"},
        {
            R"(
#include "arg_router/forwarding_arg.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = forwarding_arg(policy::none_name_t{"--"_S},
                            policy::long_name_t{"hello"_S});
    return 0;
}
    )",
            "Forwarding arg can only have a none name policy",
            "must_not_have_long_name_test"},
        {
            R"(
#include "arg_router/forwarding_arg.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = forwarding_arg(policy::none_name_t{"--"_S},
                            policy::short_name_t{"A"_S});
    return 0;
}
    )",
            "Forwarding arg can only have a none name policy",
            "must_not_have_short_name_test"},
        {
            R"(
#include "arg_router/policy/none_name.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/forwarding_arg.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = forwarding_arg(policy::none_name_t{"--"_S},
                            policy::router{[](int) {}});
    return 0;
}
    )",
            "Forwarding arg does not support policies with routing phases "
            "(e.g. router)",
            "routing_phase_test"},
    });
}

BOOST_AUTO_TEST_SUITE_END()
