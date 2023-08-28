// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/colour_help_formatter.hpp"
#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/help.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/policy/value_separator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

namespace
{
template <typename... Params>
class mock_root : public tree_node<Params...>
{
    using parent_type = tree_node<Params...>;

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

        template <typename OwnerNode, typename FilterFn>
        [[nodiscard]] static std::vector<runtime_help_data> runtime_children(const OwnerNode& owner,
                                                                             FilterFn&& f)
        {
            return parent_type::template default_leaf_help_data_type<true>::runtime_children(
                owner,
                std::forward<FilterFn>(f));
        }
    };
};
}  // namespace

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(colour_help_formatter_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::colour_help_formatter_t<>>, "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(generate_help_test)
{
    const auto root =
        mock_root{flag(policy::long_name<AR_STRING("flag1")>,
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
                       policy::colour_help_formatter)};

    constexpr auto expected_result =
        "foo v3.14\n\n"
        "My foo is good for you\n\n"
        "\033[31m    --flag1,-a\033[32m        Flag1 description\n\033[0m"
        "\033[31m    --flag2\n\033[0m"
        "\033[31m    -b\033[32m                b description\n\033[0m"
        "\033[31m    --arg1=<Value>\n\033[0m"
        "\033[31m    --help,-h\033[32m         Help output\n\033[0m"sv;

    using root_type = std::decay_t<decltype(root)>;
    using help_type = std::tuple_element_t<4, typename root_type::children_type>;

    auto stream = std::stringstream{};
    std::tuple_element_t<8, typename help_type::policies_type>::
        template generate_help<root_type, help_type, false>(stream);

    BOOST_CHECK_EQUAL(stream.str(), expected_result);
}

BOOST_AUTO_TEST_CASE(generate_runtime_help_test)
{
    const auto root =
        mock_root{flag(policy::long_name<AR_STRING("flag1")>,
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
                       policy::colour_help_formatter)};

    constexpr auto expected_result =
        "foo v3.14\n\n"
        "My foo is good for you\n\n"
        "\033[31m    --flag1,-a\033[32m        Flag1 description\n\033[0m"
        "\033[31m    --flag2\n\033[0m"
        "\033[31m    -b\033[32m                b description\n\033[0m"
        "\033[31m    --arg1=<Value>\n\033[0m"
        "\033[31m    --help,-h\033[32m         Help output\n\033[0m"sv;

    using root_type = std::decay_t<decltype(root)>;
    using root_hdt = typename root_type::template help_data_type<false>;
    using help_type = std::tuple_element_t<4, typename root_type::children_type>;
    using formatter_type = std::tuple_element_t<8, typename help_type::policies_type>;

    const auto rhd =
        runtime_help_data{root_hdt::label::get(),
                          root_hdt::description::get(),
                          root_hdt::runtime_children(root, [](auto&&) { return true; })};

    auto stream = std::stringstream{};
    formatter_type::template generate_help<root_type, help_type, false>(stream, rhd);

    BOOST_CHECK_EQUAL(stream.str(), expected_result);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
