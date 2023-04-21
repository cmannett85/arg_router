// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/arg.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/policy/value_separator.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

namespace
{
template <typename... Policies>
class custom_flag_t : public flag_t<std::decay_t<Policies>...>
{
    using parent_type = flag_t<std::decay_t<Policies>...>;

public:
    using typename parent_type::policies_type;

    /** Flag value type. */
    using value_type = typename parent_type::value_type;

    /** Help data type. */
    template <bool Flatten>
    using help_data_type = typename parent_type::template default_leaf_help_data_type<Flatten>;

    constexpr explicit custom_flag_t(Policies... policies) noexcept :
        parent_type{std::move(policies)...}
    {
    }

    template <typename Validator, bool HasTarget, typename... Parents>
    [[nodiscard]] std::optional<parsing::parse_target> pre_parse(
        parsing::pre_parse_data<Validator, HasTarget> pre_parse_data,
        const Parents&... parents) const

    {
        return parent_type::pre_parse(pre_parse_data, *this, parents...);
    }

    template <typename... Parents>
    value_type parse([[maybe_unused]] parsing::parse_target&& target,
                     [[maybe_unused]] const Parents&... parents) const
    {
        return parent_type::parse(target, *this, parents...);
    }
};

template <typename... Params>
class stub_node : public tree_node<Params...>
{
public:
    using value_type = double;

    constexpr explicit stub_node(Params... params) : tree_node<Params...>{std::move(params)...} {}
};
}  // namespace

BOOST_AUTO_TEST_SUITE(parsing_suite)

BOOST_AUTO_TEST_CASE(match_test)
{
    {
        [[maybe_unused]] const auto f =
            flag(policy::long_name<AR_STRING("hello")>, policy::short_name<'H'>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::long_, "hello"});
        BOOST_CHECK(result);
    }

    {
        [[maybe_unused]] const auto f =
            flag(policy::long_name<AR_STRING("hello")>, policy::short_name<'H'>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::short_, "H"});
        BOOST_CHECK(result);
    }

    {
        [[maybe_unused]] const auto f =
            flag(policy::long_name<AR_STRING("hello")>, policy::short_name<'H'>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::long_, "foo"});
        BOOST_CHECK(!result);
    }

    {
        [[maybe_unused]] const auto f = flag(policy::long_name<AR_STRING("hello")>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::long_, "hello"});
        BOOST_CHECK(result);
    }

    {
        [[maybe_unused]] const auto f = flag(policy::long_name<AR_STRING("hello")>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::long_, "foo"});
        BOOST_CHECK(!result);
    }

    {
        [[maybe_unused]] const auto f = flag(policy::short_name<'H'>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::short_, "H"});
        BOOST_CHECK(result);
    }

    {
        [[maybe_unused]] const auto f = flag(policy::short_name<'H'>);
        const auto result =
            parsing::match<std::decay_t<decltype(f)>>({parsing::prefix_type::short_, "a"});
        BOOST_CHECK(!result);
    }

    {
        [[maybe_unused]] const auto a =
            arg<int>(policy::long_name<AR_STRING("arg")>, policy::value_separator<'='>);
        const auto result =
            parsing::match<std::decay_t<decltype(a)>>({parsing::prefix_type::long_, "arg"});
        BOOST_CHECK(result);
    }
}

BOOST_AUTO_TEST_CASE(get_token_type_test)
{
    auto f = [](auto token, auto expected_token) {
        const auto result = parsing::get_token_type(token);
        BOOST_CHECK_EQUAL(result, expected_token);
    };

    test::data_set(
        f,
        {std::tuple{"--hello", parsing::token_type{parsing::prefix_type::long_, "hello"}},
         std::tuple{"-h", parsing::token_type{parsing::prefix_type::short_, "h"}},
         std::tuple{"hello", parsing::token_type{parsing::prefix_type::none, "hello"}},
         std::tuple{"", parsing::token_type{parsing::prefix_type::none, ""}}});
}

BOOST_AUTO_TEST_CASE(get_token_type_test_with_node)
{
    auto f = [](auto node, auto token, auto expected_token) {
        const auto result = parsing::get_token_type(node, token);
        BOOST_CHECK_EQUAL(result, expected_token);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{stub_node{policy::long_name<AR_STRING("hello")>},
                                  "--hello",
                                  parsing::token_type{parsing::prefix_type::long_, "hello"}},
                       std::tuple{stub_node{policy::short_name<'h'>},
                                  "-h",
                                  parsing::token_type{parsing::prefix_type::short_, "h"}},
                       std::tuple{stub_node{policy::short_name<'h'>},
                                  "hello",
                                  parsing::token_type{parsing::prefix_type::none, "hello"}},
                       std::tuple{stub_node{policy::long_name<AR_STRING("hello")>},
                                  "",
                                  parsing::token_type{parsing::prefix_type::none, ""}},
                       std::tuple{stub_node{policy::long_name<AR_STRING("hello")>},
                                  "-h",
                                  parsing::token_type{parsing::prefix_type::none, "-h"}},
                       std::tuple{stub_node{},
                                  "--hello",
                                  parsing::token_type{parsing::prefix_type::none, "--hello"}},
                   });
}

BOOST_AUTO_TEST_CASE(string_from_prefix_test)
{
    auto f = [](auto prefix, auto expected) {
        const auto result = parsing::to_string(prefix);
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(f,
                   {
                       std::tuple{parsing::prefix_type::long_, "--"},
                       std::tuple{parsing::prefix_type::short_, "-"},
                       std::tuple{parsing::prefix_type::none, ""},
                   });
}

BOOST_AUTO_TEST_CASE(clean_parents_list_test)
{
    auto f = [](auto nodes, auto expected) {
        auto result =
            std::apply([&](auto&&... ns) { return parsing::clean_node_ancestry_list(ns...); },
                       nodes);
        utility::tuple_iterator(
            [&](auto i, auto&& v) {
                using result_element_type = std::decay_t<decltype(v.get())>;
                using expected_element_type = std::decay_t<decltype(std::get<i>(expected))>;
                static_assert(std::is_same_v<result_element_type, expected_element_type>,
                              "Incorrect result element type");
            },
            result);
    };

    test::data_set(f,
                   std::tuple{std::tuple{std::tuple{42}, std::tuple{42}},
                              std::tuple{
                                  std::tuple{42, 3.14},
                                  std::tuple{42, 3.14},
                              },
                              std::tuple{
                                  std::tuple{flag(policy::long_name<AR_STRING("foo")>),
                                             flag(policy::long_name<AR_STRING("bar")>),
                                             arg<int>(policy::long_name<AR_STRING("foo")>)},
                                  std::tuple{flag(policy::long_name<AR_STRING("foo")>),
                                             flag(policy::long_name<AR_STRING("bar")>),
                                             arg<int>(policy::long_name<AR_STRING("foo")>)},
                              },
                              std::tuple{
                                  std::tuple{flag(policy::long_name<AR_STRING("foo")>),
                                             custom_flag_t{policy::long_name<AR_STRING("foo")>},
                                             arg<int>(policy::long_name<AR_STRING("foo")>)},
                                  std::tuple{custom_flag_t{policy::long_name<AR_STRING("foo")>},
                                             arg<int>(policy::long_name<AR_STRING("foo")>)},
                              },
                              std::tuple{
                                  std::tuple{custom_flag_t{policy::long_name<AR_STRING("bar")>},
                                             flag(policy::long_name<AR_STRING("foo")>),
                                             arg<int>(policy::long_name<AR_STRING("foo")>)},
                                  std::tuple{custom_flag_t{policy::long_name<AR_STRING("bar")>},
                                             flag(policy::long_name<AR_STRING("foo")>),
                                             arg<int>(policy::long_name<AR_STRING("foo")>)},
                              }});
}

BOOST_AUTO_TEST_SUITE_END()
