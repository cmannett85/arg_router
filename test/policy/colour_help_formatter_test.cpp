// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/policy/colour_help_formatter.hpp"
#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/help.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/policy/value_separator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
using namespace std::string_view_literals;

namespace
{
template <typename... Params>
class mock_root : public tree_node<Params...>
{
    using parent_type = tree_node<Params...>;

public:
    constexpr explicit mock_root(Params... params) : tree_node<Params...>{std::move(params)...} {}
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
        mock_root{flag(policy::long_name_t{"flag1"_S},
                       policy::short_name_t{"a"_S},
                       policy::description_t{"Flag1 description"_S}),
                  flag(policy::long_name_t{"flag2"_S}),
                  flag(policy::short_name_t{"b"_S}, policy::description_t{"b description"_S}),
                  arg<int>(policy::long_name_t{"arg1"_S}, policy::value_separator_t{"="_S}),
                  help(policy::long_name_t{"help"_S},
                       policy::short_name_t{"h"_S},
                       policy::description_t{"Help output"_S},
                       policy::program_name_t{"foo"_S},
                       policy::program_version_t{"v3.14"_S},
                       policy::program_intro_t{"My foo is good for you"_S},
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
    using formatter_type = std::tuple_element_t<8, typename help_type::policies_type>;

    const auto rhd = help_data::generate<false>(root);

    auto stream = std::stringstream{};
    formatter_type::template generate_help<help_type>(stream, rhd);

    BOOST_CHECK_EQUAL(stream.str(), expected_result);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
