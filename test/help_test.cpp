/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/help.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

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
        using label = S_("");
        using description = S_("");
        using children = typename tree_node<Params...>::template  //
            default_leaf_help_data_type<Flatten>::all_children_help;
    };
};
}  // namespace

BOOST_AUTO_TEST_SUITE(help_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<help_t<policy::long_name_t<S_("hello")>>>,
                  "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(parse_test)
{
    auto output = ""s;
    auto f = [&](const auto& root,
                 auto help_index,
                 auto tokens,
                 const auto& fail_message,
                 const auto& expected_output) {
        output.clear();

        try {
            const auto& help_node = std::get<help_index>(root.children());
            auto target = help_node.pre_parse(parsing::pre_parse_data{tokens}, root);

            help_node.parse(std::move(*target), root);
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(output, expected_output);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                mock_root{flag(policy::long_name<S_("flag1")>,
                               policy::short_name<'a'>,
                               policy::description<S_("Flag1 description")>),
                          flag(policy::long_name<S_("flag2")>),
                          flag(policy::short_name<'b'>, policy::description<S_("b description")>),
                          help(policy::long_name<S_("help")>,
                               policy::short_name<'h'>,
                               policy::description<S_("Help output")>,
                               policy::program_name<S_("foo")>,
                               policy::program_version<S_("v3.14")>,
                               policy::program_intro<S_("My foo is good for you")>,
                               policy::router{[&](auto stream) { output = stream.str(); }})},
                traits::integral_constant<3>{},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--help"}},
                ""s,
                R"(foo v3.14

My foo is good for you

    --flag1,-a    Flag1 description
    --flag2
    -b            b description
    --help,-h     Help output
)"s},
            std::tuple{
                mock_root{flag(policy::long_name<S_("flag1")>,
                               policy::short_name<'a'>,
                               policy::description<S_("Flag1 description")>),
                          flag(policy::long_name<S_("flag2")>),
                          flag(policy::short_name<'b'>, policy::description<S_("b description")>),
                          help(policy::long_name<S_("help")>,
                               policy::short_name<'h'>,
                               policy::description<S_("Help output")>,
                               policy::program_name<S_("foo")>,
                               policy::program_version<S_("v3.14")>,
                               policy::program_intro<S_("My foo is good for you")>,
                               policy::router{[&](auto stream) { output = stream.str(); }})},
                traits::integral_constant<3>{},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"}},
                ""s,
                R"(foo v3.14

My foo is good for you

    --flag1,-a    Flag1 description
    --flag2
    -b            b description
    --help,-h     Help output
)"s},
            std::tuple{mock_root{help(policy::long_name<S_("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<S_("Help output")>,
                                      policy::program_name<S_("foo")>,
                                      policy::program_version<S_("v3.14")>,
                                      policy::program_intro<S_("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }}),
                                 mode(policy::none_name<S_("mode1")>,
                                      policy::description<S_("Mode1 description")>,
                                      flag(policy::long_name<S_("flag1")>,
                                           policy::short_name<'a'>,
                                           policy::description<S_("Flag1 description")>),
                                      flag(policy::long_name<S_("flag2")>),
                                      flag(policy::short_name<'b'>,
                                           policy::description<S_("b description")>)),
                                 mode(policy::none_name<S_("mode2")>,
                                      flag(policy::long_name<S_("flag3")>,
                                           policy::short_name<'c'>,
                                           policy::description<S_("Flag3 description")>))},
                       traits::integral_constant<0>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                        {parsing::prefix_type::none, "mode1"}},
                       ""s,
                       R"(foo v3.14

My foo is good for you

mode1             Mode1 description
    --flag1,-a    Flag1 description
    --flag2
    -b            b description
)"s},
            std::tuple{mock_root{help(policy::long_name<S_("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<S_("Help output")>,
                                      policy::program_name<S_("foo")>,
                                      policy::program_version<S_("v3.14")>,
                                      policy::program_intro<S_("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }}),
                                 mode(policy::none_name<S_("mode1")>,
                                      policy::description<S_("Mode1 description")>,
                                      flag(policy::long_name<S_("flag1")>,
                                           policy::short_name<'a'>,
                                           policy::description<S_("Flag1 description")>),
                                      flag(policy::long_name<S_("flag2")>),
                                      flag(policy::short_name<'b'>,
                                           policy::description<S_("b description")>)),
                                 mode(policy::none_name<S_("mode2")>,
                                      flag(policy::long_name<S_("flag3")>,
                                           policy::short_name<'c'>,
                                           policy::description<S_("Flag3 description")>))},
                       traits::integral_constant<0>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                        {parsing::prefix_type::none, "mode2"}},
                       ""s,
                       R"(foo v3.14

My foo is good for you

mode2
    --flag3,-c    Flag3 description
)"s},
            std::tuple{
                mock_root{flag(policy::long_name<S_("flag1")>,
                               policy::short_name<'a'>,
                               policy::description<S_("Flag1 description")>),
                          flag(policy::long_name<S_("flag2")>),
                          flag(policy::short_name<'b'>, policy::description<S_("b description")>),
                          help(policy::long_name<S_("help")>,
                               policy::short_name<'h'>,
                               policy::description<S_("Help output")>,
                               policy::program_name<S_("foo")>,
                               policy::program_version<S_("v3.14")>,
                               policy::program_intro<S_("My foo is good for you")>,
                               policy::router{[&](auto stream) { output = stream.str(); }})},
                traits::integral_constant<3>{},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--help"},
                                                 {parsing::prefix_type::none, "-b"}},
                ""s,
                R"(foo v3.14

My foo is good for you

-b    b description
)"s},
            std::tuple{mock_root{help(policy::long_name<S_("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<S_("Help output")>,
                                      policy::program_name<S_("foo")>,
                                      policy::program_version<S_("v3.14")>,
                                      policy::program_intro<S_("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }}),
                                 mode(policy::none_name<S_("mode1")>,
                                      policy::description<S_("Mode1 description")>,
                                      flag(policy::long_name<S_("flag1")>,
                                           policy::short_name<'a'>,
                                           policy::description<S_("Flag1 description")>),
                                      flag(policy::long_name<S_("flag2")>),
                                      flag(policy::short_name<'b'>,
                                           policy::description<S_("b description")>)),
                                 mode(policy::none_name<S_("mode2")>,
                                      flag(policy::long_name<S_("flag3")>,
                                           policy::short_name<'c'>,
                                           policy::description<S_("Flag3 description")>))},
                       traits::integral_constant<0>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                        {parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "--flag2"}},
                       ""s,
                       R"(foo v3.14

My foo is good for you

--flag2
)"s},
            std::tuple{
                mock_root{flag(policy::long_name<S_("flag1")>,
                               policy::short_name<'a'>,
                               policy::description<S_("Flag1 description")>),
                          flag(policy::long_name<S_("flag2")>),
                          flag(policy::short_name<'b'>, policy::description<S_("b description")>),
                          help(policy::long_name<S_("help")>,
                               policy::short_name<'h'>,
                               policy::description<S_("Help output")>,
                               policy::program_name<S_("foo")>,
                               policy::program_version<S_("v3.14")>,
                               policy::program_intro<S_("My foo is good for you")>,
                               policy::router{[&](auto stream) { output = stream.str(); }})},
                traits::integral_constant<3>{},
                std::vector<parsing::token_type>{{parsing::prefix_type::none, "--help"},
                                                 {parsing::prefix_type::none, "--foo"}},
                "Unknown argument: --foo"s,
                ""s},
            std::tuple{mock_root{help(policy::long_name<S_("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<S_("Help output")>,
                                      policy::program_name<S_("foo")>,
                                      policy::program_version<S_("v3.14")>,
                                      policy::program_intro<S_("My foo is good for you")>,
                                      policy::router{[&](auto stream) { output = stream.str(); }}),
                                 mode(policy::none_name<S_("mode1")>,
                                      policy::description<S_("Mode1 description")>,
                                      flag(policy::long_name<S_("flag1")>,
                                           policy::short_name<'a'>,
                                           policy::description<S_("Flag1 description")>),
                                      flag(policy::long_name<S_("flag2")>),
                                      flag(policy::short_name<'b'>,
                                           policy::description<S_("b description")>)),
                                 mode(policy::none_name<S_("mode2")>,
                                      flag(policy::long_name<S_("flag3")>,
                                           policy::short_name<'c'>,
                                           policy::description<S_("Flag3 description")>))},
                       traits::integral_constant<0>{},
                       std::vector<parsing::token_type>{{parsing::prefix_type::none, "-h"},
                                                        {parsing::prefix_type::none, "mode1"},
                                                        {parsing::prefix_type::none, "--foo"}},
                       "Unknown argument: --foo"s,
                       ""s},
        });
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
    const auto m = help(policy::long_name<S_("hello")>,
                        policy::display_name<S_("help")>);
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
    const auto m = help(policy::long_name<S_("hello")>,
                        policy::none_name<S_("help")>);
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
    const auto m = help(policy::long_name<S_("help")>,
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
    const auto m = help(policy::long_name<S_("help")>,
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
    const auto m = help(policy::long_name<S_("help")>,
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
    const auto m = help(policy::long_name<S_("help")>);

    auto tokens = vector<parsing::token_type>{};
    const auto result = m.pre_parse(parsing::pre_parse_data{tokens});
    return 0;
}
    )",
                                  "At least one parent needed for help",
                                  "parse_must_have_parents_test"}});
}

BOOST_AUTO_TEST_SUITE_END()
