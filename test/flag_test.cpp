// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/flag.hpp"
#include "arg_router/help_data.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(flag_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<flag_t<policy::long_name_t<str<"hello">>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(policies_test)
{
    [[maybe_unused]] auto f = flag(policy::long_name_t{"hello"_S}, policy::short_name_t{"H"_S});
    static_assert(f.long_name() == "hello"sv, "Long name test fail");
    static_assert(f.short_name() == "H", "Short name test fail");
    static_assert(f.minimum_count() == 0, "Minimum count test fail");
    static_assert(f.maximum_count() == 0, "Maximum count test fail");

    static_assert(
        boost::mp11::mp_any_of_q<typename std::decay_t<decltype(f)>::policies_type,
                                 boost::mp11::mp_bind<traits::is_same_when_despecialised,
                                                      boost::mp11::_1,
                                                      policy::short_form_expander_t<>>>::value,
        "Expected short_form_expander policy");
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
                   std::tuple{std::tuple{flag("a"_S), false},
                              std::tuple{flag("a"_S, policy::router{[&](bool result) {
                                                  BOOST_CHECK(result);
                                                  router_hit = true;
                                              }}),
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
            std::tuple{flag(policy::short_name_t{"h"_S},
                            policy::long_name_t{"hello"_S},
                            policy::description_t{"A flag!"_S}),
                       "--hello,-h",
                       "A flag!"},
            std::tuple{flag(policy::long_name_t{"hello"_S}, policy::description_t{"A flag!"_S}),
                       "--hello",
                       "A flag!"},
            std::tuple{flag(policy::short_name_t{"h"_S}, policy::description_t{"A flag!"_S}),
                       "-h",
                       "A flag!"},
            std::tuple{flag(policy::short_name_t{"h"_S}), "-h", ""},
            std::tuple{flag("h"_S, "hello"_S, "A flag!"_S), "--hello,-h", "A flag!"},
        });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = flag(
        policy::long_name_t{"hello"_S},
        flag(policy::short_name_t{"b"_S}),
        policy::short_name_t{"H"_S}
    );
    return 0;
}
    )",
                               "Flags must only contain policies (not other nodes)",
                               "policies_only_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"

using namespace arg_router;

int main() {
    auto f = flag();
    return 0;
}
    )",
                                  "Flag must have a long and/or short name policy",
                                  "must_be_named_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = flag(policy::long_name_t{"hello"_S},
                  policy::display_name_t{"hello2"_S});
    return 0;
}
    )",
                                  "Flag must not have a display name policy",
                                  "must_not_have_display_name_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = flag(policy::long_name_t{"hello"_S},
                  policy::none_name_t{"hello2"_S});
    return 0;
}
    )",
                                  "Flag must not have a none name policy",
                                  "must_not_have_none_name_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = flag(policy::long_name_t{"hello"_S},
                  policy::custom_parser<bool>{[](std::string_view) { return true; }});
    return 0;
}
    )",
                                  "Flag does not support policies with parse or validation phases "
                                  "(e.g. custom_parser or min_max_value)",
                                  "parse_policy_test"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    auto f = flag(policy::long_name_t{"hello"_S},
                  policy::min_max_value<true, true>());
    return 0;
}
    )",
                                  "Flag does not support policies with parse or validation phases "
                                  "(e.g. custom_parser or min_max_value)",
                                  "validation_policy_test"}});
}

BOOST_AUTO_TEST_SUITE_END()
