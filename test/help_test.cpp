// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/help.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/runtime_enable.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_literals;

namespace
{
template <typename... Params>
class mock_root : public tree_node<Params...>
{
public:
    constexpr explicit mock_root(Params... params) : tree_node<Params...>{std::move(params)...} {}

    template <bool Flatten>
    class help_data_type
    {
    public:
        using label = AR_STRING("");
        using description = AR_STRING("");
        using children = typename tree_node<Params...>::template default_leaf_help_data_type<
            Flatten>::all_children_help;
    };
};
}  // namespace

BOOST_AUTO_TEST_SUITE(help_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<help_t<policy::long_name_t<AR_STRING("hello")>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(has_generate_help_method_test)
{
    static_assert(
        traits::has_generate_help_method_v<help_t<policy::long_name_t<AR_STRING("help")>>>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(has_generate_runtime_help_data_method_test)
{
    static_assert(traits::has_generate_runtime_help_data_method_v<
                      help_t<policy::long_name_t<AR_STRING("help")>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(has_runtime_generate_help_method_test)
{
    static_assert(
        traits::has_runtime_generate_help_method_v<help_t<policy::long_name_t<AR_STRING("help")>>>,
        "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(parse_test)
{
    auto output = ""s;
    auto f = [&](const auto& root,
                 auto help_index,
                 auto tokens,
                 const auto& ec,
                 const auto& expected_output) {
        output.clear();

        try {
            const auto& help_node = std::get<help_index>(root.children());
            auto target = help_node.pre_parse(parsing::pre_parse_data{tokens}, root);

            help_node.parse(std::move(*target), root);
            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(output, expected_output);
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 flag(policy::long_name<AR_STRING("flag2")>),
                                 flag(policy::short_name<'b'>,
                                      policy::description<AR_STRING("b description")>),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }})},
                       traits::integral_constant<3>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--help"}},
                       std::optional<multi_lang_exception>{},
                       R"(foo v3.14

My foo is good for you

    --flag1,-a    Flag1 description
    --flag2
    -b            b description
    --help,-h     Help output
)"s},
            std::tuple{
                mock_root{
                    flag(AR_STRING("flag1"){}, AR_STRING("a"){}, AR_STRING("Flag1 description"){}),
                    flag(AR_STRING("flag2"){}),
                    flag(AR_STRING("b"){}, policy::description<AR_STRING("b description")>),
                    help(AR_STRING("help"){},
                         AR_STRING("h"){},
                         AR_STRING("Help output"){},
                         policy::program_name<AR_STRING("foo")>,
                         policy::program_version<AR_STRING("v3.14")>,
                         policy::program_intro<AR_STRING("My foo is good for you")>,
                         policy::router{[&](auto stream) { output = stream.str(); }})},
                traits::integral_constant<3>{},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--help"}},
                std::optional<multi_lang_exception>{},
                R"(foo v3.14

My foo is good for you

    --flag1,-a    Flag1 description
    --flag2
    -b            b description
    --help,-h     Help output
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 flag(policy::long_name<AR_STRING("flag2")>),
                                 flag(policy::short_name<'b'>,
                                      policy::description<AR_STRING("b description")>),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }})},
                       traits::integral_constant<3>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"}},
                       std::optional<multi_lang_exception>{},
                       R"(foo v3.14

My foo is good for you

    --flag1,-a    Flag1 description
    --flag2
    -b            b description
    --help,-h     Help output
)"s},
            std::tuple{mock_root{help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }}),
                                 mode(policy::none_name<AR_STRING("mode1")>,
                                      policy::description<AR_STRING("Mode1 description")>,
                                      flag(policy::long_name<AR_STRING("flag1")>,
                                           policy::short_name<'a'>,
                                           policy::description<AR_STRING("Flag1 description")>),
                                      flag(policy::long_name<AR_STRING("flag2")>),
                                      flag(policy::short_name<'b'>,
                                           policy::description<AR_STRING("b description")>)),
                                 mode(policy::none_name<AR_STRING("mode2")>,
                                      flag(policy::long_name<AR_STRING("flag3")>,
                                           policy::short_name<'c'>,
                                           policy::description<AR_STRING("Flag3 description")>))},
                       traits::integral_constant<0>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                        {parsing::prefix_type::none, "mode1"}},
                       std::optional<multi_lang_exception>{},
                       R"(foo v3.14

My foo is good for you

mode1             Mode1 description
    --flag1,-a    Flag1 description
    --flag2
    -b            b description
)"s},
            std::tuple{mock_root{help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }}),
                                 mode(policy::none_name<AR_STRING("mode1")>,
                                      policy::description<AR_STRING("Mode1 description")>,
                                      flag(policy::long_name<AR_STRING("flag1")>,
                                           policy::short_name<'a'>,
                                           policy::description<AR_STRING("Flag1 description")>),
                                      flag(policy::long_name<AR_STRING("flag2")>),
                                      flag(policy::short_name<'b'>,
                                           policy::description<AR_STRING("b description")>)),
                                 mode(policy::none_name<AR_STRING("mode2")>,
                                      flag(policy::long_name<AR_STRING("flag3")>,
                                           policy::short_name<'c'>,
                                           policy::description<AR_STRING("Flag3 description")>))},
                       traits::integral_constant<0>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                        {parsing::prefix_type::none, "mode2"}},
                       std::optional<multi_lang_exception>{},
                       R"(foo v3.14

My foo is good for you

mode2
    --flag3,-c    Flag3 description
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 flag(policy::long_name<AR_STRING("flag2")>),
                                 flag(policy::short_name<'b'>,
                                      policy::description<AR_STRING("b description")>),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }})},
                       traits::integral_constant<3>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--help"},
                                                        {parsing::prefix_type::none, "-b"}},
                       std::optional<multi_lang_exception>{},
                       R"(foo v3.14

My foo is good for you

-b    b description
)"s},
            std::tuple{mock_root{help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }}),
                                 mode(policy::none_name<AR_STRING("mode1")>,
                                      policy::description<AR_STRING("Mode1 description")>,
                                      flag(policy::long_name<AR_STRING("flag1")>,
                                           policy::short_name<'a'>,
                                           policy::description<AR_STRING("Flag1 description")>),
                                      flag(policy::long_name<AR_STRING("flag2")>),
                                      flag(policy::short_name<'b'>,
                                           policy::description<AR_STRING("b description")>)),
                                 mode(policy::none_name<AR_STRING("mode2")>,
                                      flag(policy::long_name<AR_STRING("flag3")>,
                                           policy::short_name<'c'>,
                                           policy::description<AR_STRING("Flag3 description")>))},
                       traits::integral_constant<0>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                        {parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "--flag2"}},
                       std::optional<multi_lang_exception>{},
                       R"(foo v3.14

My foo is good for you

--flag2
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 flag(policy::long_name<AR_STRING("flag2")>),
                                 flag(policy::short_name<'b'>,
                                      policy::description<AR_STRING("b description")>),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }})},
                       traits::integral_constant<3>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--help"},
                                                        {parsing::prefix_type::none, "--foo"}},
                       std::optional<multi_lang_exception>{multi_lang_exception{
                           error_code::unknown_argument,
                           std::vector{parsing::token_type{parsing::prefix_type::none, "--foo"}}}},
                       ""s},
            std::tuple{mock_root{help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }}),
                                 mode(policy::none_name<AR_STRING("mode1")>,
                                      policy::description<AR_STRING("Mode1 description")>,
                                      flag(policy::long_name<AR_STRING("flag1")>,
                                           policy::short_name<'a'>,
                                           policy::description<AR_STRING("Flag1 description")>),
                                      flag(policy::long_name<AR_STRING("flag2")>),
                                      flag(policy::short_name<'b'>,
                                           policy::description<AR_STRING("b description")>)),
                                 mode(policy::none_name<AR_STRING("mode2")>,
                                      flag(policy::long_name<AR_STRING("flag3")>,
                                           policy::short_name<'c'>,
                                           policy::description<AR_STRING("Flag3 description")>))},
                       traits::integral_constant<0>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                        {parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "--foo"}},
                       std::optional<multi_lang_exception>{multi_lang_exception{
                           error_code::unknown_argument,
                           std::vector{parsing::token_type{parsing::prefix_type::none, "--foo"}}}},
                       ""s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 flag(policy::long_name<AR_STRING("flag2")>),
                                 flag(policy::short_name<'b'>,
                                      policy::description<AR_STRING("b description")>,
                                      policy::runtime_enable{false}),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }})},
                       traits::integral_constant<3>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "--help"}},
                       std::optional<multi_lang_exception>{},
                       R"(foo v3.14

My foo is good for you

    --flag1,-a    Flag1 description
    --flag2
    --help,-h     Help output
)"s},
            std::tuple{mock_root{help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }}),
                                 mode(policy::none_name<AR_STRING("mode1")>,
                                      policy::description<AR_STRING("Mode1 description")>,
                                      flag(policy::long_name<AR_STRING("flag1")>,
                                           policy::short_name<'a'>,
                                           policy::description<AR_STRING("Flag1 description")>,
                                           policy::runtime_enable{false}),
                                      flag(policy::long_name<AR_STRING("flag2")>),
                                      flag(policy::short_name<'b'>,
                                           policy::description<AR_STRING("b description")>)),
                                 mode(policy::none_name<AR_STRING("mode2")>,
                                      flag(policy::long_name<AR_STRING("flag3")>,
                                           policy::short_name<'c'>,
                                           policy::description<AR_STRING("Flag3 description")>))},
                       traits::integral_constant<0>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                        {parsing::prefix_type::none, "mode1"}},
                       std::optional<multi_lang_exception>{},
                       R"(foo v3.14

My foo is good for you

mode1             Mode1 description
    --flag2
    -b            b description
)"s}});
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{R"(
#include "arg_router/help.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = help(policy::long_name<AR_STRING("hello")>,
                        policy::display_name<AR_STRING("help")>);
    return 0;
}
    )",
                               "Help must not have a display name policy",
                               "must_not_have_display_name_test"},
                              {
                                  R"(
#include "arg_router/help.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = help(policy::long_name<AR_STRING("hello")>,
                        policy::none_name<AR_STRING("help")>);
    return 0;
}
    )",
                                  "Help must not have a none name policy",
                                  "must_not_have_none_name_test"},
                              {
                                  R"(
#include "arg_router/help.hpp"
#include "arg_router/policy/custom_parser.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = help(policy::long_name<AR_STRING("help")>,
                        policy::custom_parser<int>{[](auto) { return 42; }});
    return 0;
}
    )",
                                  "Help only supports policies with pre-parse and routing phases",
                                  "must_not_have_parse_phase_test"},
                              {
                                  R"(
#include "arg_router/help.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = help(policy::long_name<AR_STRING("help")>,
                        policy::min_max_value<0, 1>());
    return 0;
}
    )",
                                  "Help only supports policies with pre-parse and routing phases",
                                  "must_not_have_validation_phase_test"},
                              {
                                  R"(
#include "arg_router/help.hpp"
#include "arg_router/policy/default_value.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

int main() {
    const auto m = help(policy::long_name<AR_STRING("help")>,
                        policy::default_value{42});
    return 0;
}
    )",
                                  "Help only supports policies with pre-parse and routing phases",
                                  "must_not_have_missing_phase_test"},
                              {
                                  R"(
#include "arg_router/help.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

struct mock_root {};

int main() {
    const auto m = help(policy::long_name<AR_STRING("help")>);

    auto tokens = vector<parsing::token_type>{};
    const auto result = m.pre_parse(parsing::pre_parse_data{tokens});
    return 0;
}
    )",
                                  "At least one parent needed for help",
                                  "parse_must_have_parents_test"}});
}

BOOST_AUTO_TEST_SUITE_END()
