// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/help.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/policy/none_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/policy/value_separator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_literals;

std::size_t utility::terminal::test_columns_value = 80;

namespace
{
template <typename... Params>
class mock_root : public tree_node<Params...>
{
    using parent_type = tree_node<Params...>;

public:
    using typename parent_type::children_type;

    constexpr explicit mock_root(Params... params) : tree_node<Params...>{std::move(params)...} {}

    template <bool Flatten>
    class help_data_type
    {
    public:
        using label = AR_STRING("");
        using description = AR_STRING("");
        using children = typename tree_node<Params...>::template default_leaf_help_data_type<
            Flatten>::all_children_help;

        template <typename OwnerNode, typename FilterFn>
        [[nodiscard]] static std::vector<runtime_help_data> runtime_children(const OwnerNode& owner,
                                                                             FilterFn&& f)
        {
            return parent_type::template default_leaf_help_data_type<Flatten>::runtime_children(
                owner,
                std::forward<FilterFn>(f));
        }
    };
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(default_help_formatter_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::default_help_formatter_t<>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(generate_help_test)
{
    auto f = [](const auto& root, auto help_index, auto flatten, const auto& expected_result) {
        using root_type = std::decay_t<decltype(root)>;
        using root_hdt = typename root_type::template help_data_type<flatten>;
        using help_type = std::tuple_element_t<help_index, typename root_type::children_type>;

        // For some baffling reason MSVC will not accept this alias, but does accept the full
        // name...
        // using formatter_type = std::tuple_element_t<0, typename help_type::policies_type>;

        // Compile-time
        {
            auto stream = std::stringstream{};
            std::tuple_element_t<0, typename help_type::policies_type>::
                template generate_help<root_type, help_type, flatten>(stream);
            BOOST_CHECK_EQUAL(stream.str(), expected_result);
        }

        // Runtime
        {
            const auto rhd =
                runtime_help_data{root_hdt::label::get(),
                                  root_hdt::description::get(),
                                  root_hdt::runtime_children(root, [](auto&&) { return true; })};

            auto stream = std::stringstream{};
            std::tuple_element_t<0, typename help_type::policies_type>::
                template generate_help<root_type, help_type, flatten>(stream, rhd);
            BOOST_CHECK_EQUAL(stream.str(), expected_result);
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                mock_root{
                    flag(policy::long_name<AR_STRING("flag1")>,
                         policy::short_name<'a'>,
                         policy::description<AR_STRING("Flag1 description")>),
                    flag(policy::long_name<AR_STRING("flag2")>),
                    flag(policy::short_name<'b'>, policy::description<AR_STRING("b description")>),
                    arg<int>(policy::long_name<AR_STRING("arg1")>, policy::value_separator<'='>),
                    help(policy::long_name<AR_STRING("help")>,
                         policy::short_name<'h'>,
                         policy::description<AR_STRING("Help output")>,
                         policy::program_name<AR_STRING("foo")>,
                         policy::program_version<AR_STRING("v3.14")>,
                         policy::program_intro<AR_STRING("My foo is good for you")>)},
                traits::integral_constant<4>{},
                std::false_type{},
                R"(foo v3.14

My foo is good for you

    --flag1,-a        Flag1 description
    --flag2
    -b                b description
    --arg1=<Value>
    --help,-h         Help output
)"s},
            std::tuple{
                mock_root{
                    flag(policy::long_name<AR_STRING("flag1")>,
                         policy::short_name<'a'>,
                         policy::description<AR_STRING("Flag1 description")>),
                    flag(policy::long_name<AR_STRING("flag2")>),
                    flag(policy::short_name<'b'>, policy::description<AR_STRING("b description")>),
                    arg<int>(policy::long_name<AR_STRING("arg1")>, policy::value_separator<'='>),
                    help(policy::long_name<AR_STRING("help")>,
                         policy::short_name<'h'>,
                         policy::description<AR_STRING("Help output")>,
                         policy::program_name<AR_STRING("foo")>,
                         policy::program_version<AR_STRING("v3.14")>,
                         policy::program_intro<AR_STRING("My foo is good for you")>,
                         policy::program_addendum<AR_STRING("Some addendum information.")>)},
                traits::integral_constant<4>{},
                std::false_type{},
                R"(foo v3.14

My foo is good for you

    --flag1,-a        Flag1 description
    --flag2
    -b                b description
    --arg1=<Value>
    --help,-h         Help output

Some addendum information.
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 flag(policy::long_name<AR_STRING("flag2")>),
                                 flag(policy::short_name<'b'>,
                                      policy::description<AR_STRING("b description")>),
                                 arg<int>(policy::long_name<AR_STRING("arg1")>,
                                          policy::value_separator<'='>,
                                          policy::min_max_value<2, 8>()),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>)},
                       traits::integral_constant<4>{},
                       std::false_type{},
                       R"(foo v3.14

My foo is good for you

    --flag1,-a      Flag1 description
    --flag2
    -b              b description
    --arg1=<2-8>
    --help,-h       Help output
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 flag(policy::long_name<AR_STRING("flag2")>),
                                 flag(policy::short_name<'b'>,
                                      policy::description<AR_STRING("b description")>),
                                 arg<int>(policy::long_name<AR_STRING("arg1")>,
                                          policy::value_separator<'='>,
                                          policy::min_value<2>()),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>)},
                       traits::integral_constant<4>{},
                       std::false_type{},
                       R"(foo v3.14

My foo is good for you

    --flag1,-a      Flag1 description
    --flag2
    -b              b description
    --arg1=<2-N>
    --help,-h       Help output
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 flag(policy::long_name<AR_STRING("flag2")>),
                                 flag(policy::short_name<'b'>,
                                      policy::description<AR_STRING("b description")>),
                                 arg<int>(policy::long_name<AR_STRING("arg1")>,
                                          policy::value_separator<'='>,
                                          policy::max_value<8>()),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>)},
                       traits::integral_constant<4>{},
                       std::false_type{},
                       R"(foo v3.14

My foo is good for you

    --flag1,-a       Flag1 description
    --flag2
    -b               b description
    --arg1=<-N-8>
    --help,-h        Help output
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 flag(policy::long_name<AR_STRING("flag2")>),
                                 flag(policy::short_name<'b'>,
                                      policy::description<AR_STRING("b description")>),
                                 arg<std::size_t>(policy::long_name<AR_STRING("arg1")>,
                                                  policy::value_separator<'='>,
                                                  policy::max_value<8u>()),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>)},
                       traits::integral_constant<4>{},
                       std::false_type{},
                       R"(foo v3.14

My foo is good for you

    --flag1,-a      Flag1 description
    --flag2
    -b              b description
    --arg1=<0-8>
    --help,-h       Help output
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 flag(policy::long_name<AR_STRING("flag2")>),
                                 flag(policy::short_name<'b'>,
                                      policy::description<AR_STRING("b description")>),
                                 arg<int>(policy::long_name<AR_STRING("arg1")>),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>)},
                       traits::integral_constant<4>{},
                       std::false_type{},
                       R"(foo v3.14

My foo is good for you

    --flag1,-a        Flag1 description
    --flag2
    -b                b description
    --arg1 <Value>
    --help,-h         Help output
)"s},
            std::tuple{mock_root{help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>),
                                 mode(flag(policy::long_name<AR_STRING("flag1")>,
                                           policy::short_name<'a'>,
                                           policy::description<AR_STRING("Flag1 description")>),
                                      flag(policy::long_name<AR_STRING("flag2")>),
                                      arg<int>(policy::long_name<AR_STRING("arg1")>,
                                               policy::value_separator<'='>,
                                               policy::description<AR_STRING("Arg1 description")>),
                                      flag(policy::short_name<'b'>,
                                           policy::description<AR_STRING("b description")>))},
                       traits::integral_constant<0>{},
                       std::false_type{},
                       R"(foo v3.14

My foo is good for you

    --help,-h             Help output
     
        --flag1,-a        Flag1 description
        --flag2
        --arg1=<Value>    Arg1 description
        -b                b description
)"s},
            std::tuple{mock_root{help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::flatten_help),
                                 mode(flag(policy::long_name<AR_STRING("flag1")>,
                                           policy::short_name<'a'>,
                                           policy::description<AR_STRING("Flag1 description")>),
                                      flag(policy::long_name<AR_STRING("flag2")>),
                                      flag(policy::short_name<'b'>,
                                           policy::description<AR_STRING("b description")>))},
                       traits::integral_constant<0>{},
                       std::true_type{},
                       R"(foo v3.14

My foo is good for you

    --help,-h         Help output
     
        --flag1,-a    Flag1 description
        --flag2
        -b            b description
)"s},
            std::tuple{mock_root{help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>),
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
                       std::false_type{},
                       R"(foo v3.14

My foo is good for you

    --help,-h    Help output
    mode1        Mode1 description
    mode2
)"s},
            std::tuple{mock_root{help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>,
                                      policy::flatten_help),
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
                       std::true_type{},
                       R"(foo v3.14

My foo is good for you

    --help,-h         Help output
    mode1             Mode1 description
        --flag1,-a    Flag1 description
        --flag2
        -b            b description
    mode2
        --flag3,-c    Flag3 description
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>)},
                       traits::integral_constant<1>{},
                       std::false_type{},
                       R"(foo

My foo is good for you

    --flag1,-a    Flag1 description
    --help,-h     Help output
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_version<AR_STRING("v3.14")>,
                                      policy::program_intro<AR_STRING("My foo is good for you")>)},
                       traits::integral_constant<1>{},
                       std::false_type{},
                       R"(My foo is good for you

    --flag1,-a    Flag1 description
    --help,-h     Help output
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>,
                                      policy::program_name<AR_STRING("foo")>,
                                      policy::program_version<AR_STRING("v3.14")>)},
                       traits::integral_constant<1>{},
                       std::false_type{},
                       R"(foo v3.14

    --flag1,-a    Flag1 description
    --help,-h     Help output
)"s},
            std::tuple{mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                      policy::short_name<'a'>,
                                      policy::description<AR_STRING("Flag1 description")>),
                                 help(policy::long_name<AR_STRING("help")>,
                                      policy::short_name<'h'>,
                                      policy::description<AR_STRING("Help output")>)},
                       traits::integral_constant<1>{},
                       std::false_type{},
                       R"(    --flag1,-a    Flag1 description
    --help,-h     Help output
)"s},
        });
}

BOOST_AUTO_TEST_CASE(generate_help_terminal_width_test)
{
    auto f = [](const auto& root, auto help_index, auto term_width, const auto& expected_result) {
        [[maybe_unused]] const auto columns1 = utility::terminal::columns();
        utility::terminal::test_columns_value = term_width;
        [[maybe_unused]] const auto columns2 = utility::terminal::columns();

        using root_type = std::decay_t<decltype(root)>;
        using help_type = std::tuple_element_t<help_index,  //
                                               typename root_type::children_type>;

        auto stream = std::stringstream{};
        std::tuple_element_t<0, typename help_type::policies_type>::
            template generate_help<root_type, help_type, false>(stream);
        BOOST_CHECK_EQUAL(stream.str(), expected_result);
    };

    // Save the default value...
    const auto default_test_columns_value = utility::terminal::test_columns_value;

    test::data_set(
        f,
        std::tuple{
            std::tuple{
                mock_root{
                    flag(policy::long_name<AR_STRING("flag1")>,
                         policy::short_name<'a'>,
                         policy::description<AR_STRING("Flag1 description")>),
                    flag(policy::long_name<AR_STRING("flag2")>),
                    flag(policy::short_name<'b'>, policy::description<AR_STRING("b description")>),
                    arg<int>(policy::long_name<AR_STRING("arg1")>, policy::value_separator<'='>),
                    help(policy::long_name<AR_STRING("help")>,
                         policy::short_name<'h'>,
                         policy::description<AR_STRING("Help output")>,
                         policy::program_name<AR_STRING("foo")>,
                         policy::program_version<AR_STRING("v3.14")>,
                         policy::program_intro<AR_STRING("My foo is good for you")>)},
                traits::integral_constant<4>{},
                32,
                R"(foo v3.14

My foo is good for you

    --flag1,-a        Flag1 
                      descriptio
                      n
    --flag2
    -b                b 
                      descriptio
                      n
    --arg1=<Value>
    --help,-h         Help 
                      output
)"s},
            std::tuple{
                mock_root{
                    flag(policy::long_name<AR_STRING("flag1")>,
                         policy::short_name<'a'>,
                         policy::description<AR_STRING("aaa aaaaaaa aaa aaaaaaa aaaaaaaaaaa")>),
                    flag(policy::long_name<AR_STRING("flag2")>),
                    help(policy::long_name<AR_STRING("help")>,
                         policy::short_name<'h'>,
                         policy::description<AR_STRING("bbbbbbbbbbbbbbbbbbbbbbbb")>,
                         policy::program_name<AR_STRING("foo")>,
                         policy::program_version<AR_STRING("v3.14")>,
                         policy::program_intro<AR_STRING("My foo is good for you")>)},
                traits::integral_constant<2>{},
                40,
                R"(foo v3.14

My foo is good for you

    --flag1,-a    aaa aaaaaaa aaa 
                  aaaaaaa aaaaaaaaaaa
    --flag2
    --help,-h     bbbbbbbbbbbbbbbbbbbbbb
                  bb
)"s},
        });

    // ... And then reinstate so we don't break later tests
    utility::terminal::test_columns_value = default_test_columns_value;
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({{
                                  R"(
#include "arg_router/help.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

struct mock_root {};

int main() {
    const auto m = help(policy::long_name<AR_STRING("help")>);
    auto stream = std::stringstream{};
    m.generate_help<mock_root, std::decay_t<decltype(m)>, false>(stream);
    return 0;
}
    )",
                                  "Node must have a help_data_type to generate help from",
                                  "generate_help_node_must_have_help_data_test"},
                              {
                                  R"(
#include "arg_router/help.hpp"

using namespace arg_router;

int main() {
    const auto m = policy::default_help_formatter_t<
        int,
        traits::integral_constant<std::size_t{8}>,
        policy::help_formatter_component::default_line_formatter<4>,
        policy::help_formatter_component::default_preamble_formatter>{};
    return 0;
}
    )",
                                  "Indent must have a value_type",
                                  "indent_must_have_value_type"},
                              {
                                  R"(
#include "arg_router/help.hpp"

using namespace arg_router;

int main() {
    const auto m = policy::default_help_formatter_t<
        traits::integral_constant<std::size_t{4}>,
        int,
        policy::help_formatter_component::default_line_formatter<4>,
        policy::help_formatter_component::default_preamble_formatter>{};
    return 0;
}
    )",
                                  "DescColumnOffset must have a value_type",
                                  "desc_column_offset_must_have_value_type"},
                              {
                                  R"(
#include "arg_router/help.hpp"

using namespace arg_router;

int main() {
    const auto m = policy::default_help_formatter_t<
        traits::integral_constant<std::size_t{0}>,
        traits::integral_constant<std::size_t{8}>,
        policy::help_formatter_component::default_line_formatter<4>,
        policy::help_formatter_component::default_preamble_formatter>{};
    return 0;
}
    )",
                                  "Indent value_type must be greater than zero",
                                  "indent_value_type_must_be_greater_than_zero"},
                              {
                                  R"(
#include "arg_router/help.hpp"

using namespace arg_router;

int main() {
    const auto m = policy::default_help_formatter_t<
        traits::integral_constant<std::size_t{4}>,
        traits::integral_constant<std::size_t{0}>,
        policy::help_formatter_component::default_line_formatter<4>,
        policy::help_formatter_component::default_preamble_formatter>{};
    return 0;
}
    )",
                                  "DescColumnOffset value_type must be greater than zero",
                                  "desc_column_offset_value_type_must_be_greater_than_zero"},
                              {
                                  R"(
#include "arg_router/help.hpp"

using namespace arg_router;

struct mock_root {
    template <bool Flatten>
    class help_data_type
    {
    public:
        using label = AR_STRING("");
        using description = AR_STRING("");
        using children = std::tuple<>;
    };
};

int main() {
    const auto m = policy::default_help_formatter_t<
        traits::integral_constant<std::size_t{4}>,
        traits::integral_constant<std::size_t{8}>,
        policy::help_formatter_component::default_line_formatter<0>,
        policy::help_formatter_component::default_preamble_formatter>{};

    auto stream = std::stringstream{};
    m.generate_help<mock_root, std::decay_t<decltype(m)>, false>(stream);
    return 0;
}
    )",
                                  "Indent must be greater than zero",
                                  "indent_must_be_greater_than_zero"},
                              {
                                  R"(
#include "arg_router/flag.hpp"
#include "arg_router/help.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

using namespace arg_router;

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
        using children = typename tree_node<std::decay_t<Params>...>::template  //
            default_leaf_help_data_type<Flatten>::all_children_help;
    };
};

int main() {
    const auto root = mock_root{flag(policy::long_name<AR_STRING("flag1")>,
                                     policy::description<AR_STRING("Flag1\tdescription")>),
                                help(policy::long_name<AR_STRING("help")>)};
    const auto& h = std::get<1>(root.children());

    auto stream = std::ostringstream{};
    h.generate_help<std::decay_t<decltype(root)>, std::decay_t<decltype(h)>, false>(stream);
    return 0;
}
    )",
                                  "Help descriptions cannot contain tabs",
                                  "no_tabs_in_description_test"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
