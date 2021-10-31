#include "arg_router/parsing.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/policy/alias.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(parsing_suite)

BOOST_AUTO_TEST_CASE(has_aliased_node_indices_test)
{
    using aliased_type =
        flag_t<policy::long_name_t<S_("flag1")>,
               policy::alias_t<policy::long_name_t<S_("Hello")>>>;
    static_assert(traits::has_aliased_policies_type_v<aliased_type>, "Fail");

    using not_aliased_type = flag_t<policy::long_name_t<S_("flag1")>>;
    static_assert(!traits::has_aliased_policies_type_v<not_aliased_type>,
                  "Fail");
}

BOOST_AUTO_TEST_CASE(flag_default_match_test)
{
    {
        const auto f =
            flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "hello"});
        BOOST_CHECK(result);
    }

    {
        const auto f =
            flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>('H');
        BOOST_CHECK(result);
    }

    {
        const auto f =
            flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "foo"});
        BOOST_CHECK(!result);
    }

    {
        const auto f = flag(policy::long_name<S_("hello")>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "hello"});
        BOOST_CHECK(result);
    }

    {
        const auto f = flag(policy::long_name<S_("hello")>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "foo"});
        BOOST_CHECK(!result);
    }

    {
        const auto f = flag(policy::short_name<'H'>);
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>('H');
        BOOST_CHECK(result);
    }

    {
        const auto f = flag(policy::short_name<'H'>);
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>('a');
        BOOST_CHECK(!result);
    }
}

BOOST_AUTO_TEST_CASE(get_token_type_test)
{
    auto f = [](auto token, auto expected_token) {
        const auto result = parsing::get_token_type(token);
        BOOST_CHECK(result == expected_token);
    };

    test::data_set(
        f,
        {std::tuple{"--hello",
                    parsing::token_type{parsing::prefix_type::LONG, "hello"}},
         std::tuple{"-h",
                    parsing::token_type{parsing::prefix_type::SHORT, "h"}},
         std::tuple{"hello",
                    parsing::token_type{parsing::prefix_type::NONE, "hello"}},
         std::tuple{"", parsing::token_type{parsing::prefix_type::NONE, ""}}});
}

BOOST_AUTO_TEST_CASE(expand_arguments_test)
{
    auto f = [](auto input, auto expected) {
        const auto result =
            parsing::expand_arguments(input.size(), input.data());
        BOOST_REQUIRE_EQUAL(result.size(), expected.size());
        for (auto i = 0u; i < result.size(); ++i) {
            BOOST_CHECK(result[i] == expected[i]);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"program name", "--foo", "-g", "-d", "42"},
                       parsing::token_list{{parsing::prefix_type::LONG, "foo"},
                                           {parsing::prefix_type::SHORT, "g"},
                                           {parsing::prefix_type::SHORT, "d"},
                                           {parsing::prefix_type::NONE, "42"}}},
            std::tuple{std::vector{"program name", "-fwed"},
                       parsing::token_list{{parsing::prefix_type::SHORT, "f"},
                                           {parsing::prefix_type::SHORT, "w"},
                                           {parsing::prefix_type::SHORT, "e"},
                                           {parsing::prefix_type::SHORT, "d"}}},
            std::tuple{std::vector{"program name",
                                   "--foo",
                                   "42",
                                   "-venv",
                                   "-d",
                                   "-abc"},
                       parsing::token_list{{parsing::prefix_type::LONG, "foo"},
                                           {parsing::prefix_type::NONE, "42"},
                                           {parsing::prefix_type::SHORT, "v"},
                                           {parsing::prefix_type::SHORT, "e"},
                                           {parsing::prefix_type::SHORT, "n"},
                                           {parsing::prefix_type::SHORT, "v"},
                                           {parsing::prefix_type::SHORT, "d"},
                                           {parsing::prefix_type::SHORT, "a"},
                                           {parsing::prefix_type::SHORT, "b"},
                                           {parsing::prefix_type::SHORT, "c"}}},
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
                       std::tuple{parsing::prefix_type::LONG, "--"},
                       std::tuple{parsing::prefix_type::SHORT, "-"},
                       std::tuple{parsing::prefix_type::NONE, ""},
                   });
}

BOOST_AUTO_TEST_CASE(optional_router_args_test)
{
    {
        using type = root_t<
            flag_t<policy::long_name_t<S_("hello")>>,
            std::decay_t<decltype(policy::validation::default_validator)>>;
        static_assert(std::is_same_v<parsing::optional_router_args_t<type>,
                                     std::tuple<std::optional<bool>>>,
                      "Build router args test 1 fail");
    }

    {
        using type = root_t<
            flag_t<policy::long_name_t<S_("hello")>>,
            arg_t<int, policy::long_name_t<S_("goodbye")>>,
            std::decay_t<decltype(policy::validation::default_validator)>>;
        static_assert(
            std::is_same_v<parsing::optional_router_args_t<type>,
                           std::tuple<std::optional<bool>, std::optional<int>>>,
            "Build router args test 1 fail");
    }

    {
        using type = flag_t<policy::long_name_t<S_("hello")>>;
        static_assert(std::is_same_v<parsing::optional_router_args_t<type>,
                                     std::tuple<std::optional<bool>>>,
                      "Build router args test 1 fail");
    }
}

BOOST_AUTO_TEST_CASE(visit_child_test)
{
    const auto r = root(flag(policy::long_name<S_("hello")>,
                             policy::description<S_("Hello description")>,
                             policy::router{[]() {}}),
                        flag(policy::short_name<'h'>,
                             policy::description<S_("h description")>,
                             policy::router{[]() {}}),
                        flag(policy::short_name<'b'>,
                             policy::description<S_("b description")>,
                             policy::router{[]() {}}),
                        policy::validation::default_validator);

    auto f = [&](auto token, auto expected_child_index) {
        auto visitor_hit_count = 0u;
        auto v = [&](auto i, auto&& child) {
            static_assert(
                std::is_same_v<
                    std::tuple_element_t<
                        i,
                        typename std::decay_t<decltype(r)>::children_type>,
                    std::decay_t<decltype(child)>>,
                "Child index and child type at index differ");
            BOOST_CHECK_EQUAL(i, expected_child_index);
            ++visitor_hit_count;
        };

        parsing::visit_child(token, r.children(), v);
        BOOST_CHECK_EQUAL(visitor_hit_count, 1);
    };

    test::data_set(
        f,
        {std::tuple{parsing::token_type{parsing::prefix_type::LONG, "hello"},
                    0},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "h"}, 1},
         std::tuple{parsing::token_type{parsing::prefix_type::SHORT, "b"}, 2}});
}

BOOST_AUTO_TEST_CASE(pos_arg_visit_child_test)
{
    using router_args_type =
        std::tuple<std::optional<bool>,
                   std::optional<std::vector<std::string_view>>,
                   std::optional<int>,
                   std::optional<std::vector<double>>>;

    const auto m =
        mode(flag(policy::long_name<S_("hello")>,
                  policy::description<S_("Hello description")>),
             positional_arg<std::vector<std::string_view>>(
                 policy::long_name<S_("p1")>,
                 policy::description<S_("p1 description")>,
                 policy::count<2>),
             positional_arg<int>(policy::long_name<S_("p2")>,
                                 policy::description<S_("p2 description")>,
                                 policy::count<1>),
             positional_arg<std::vector<double>>(
                 policy::long_name<S_("p3")>,
                 policy::description<S_("p3 description")>));

    auto f = [&](auto token, auto expected_child_index, auto router_args) {
        auto visitor_hit_count = 0u;
        auto v = [&](auto i, auto&& child) {
            static_assert(
                std::is_same_v<
                    std::tuple_element_t<
                        i,
                        typename std::decay_t<decltype(m)>::children_type>,
                    std::decay_t<decltype(child)>>,
                "Child index and child type at index differ");
            BOOST_CHECK_EQUAL(i, expected_child_index);
            ++visitor_hit_count;
        };

        parsing::visit_child(token, m.children(), router_args, v);
        BOOST_CHECK_EQUAL(visitor_hit_count, 1);
    };

    test::data_set(
        f,
        {std::tuple{parsing::token_type{parsing::prefix_type::LONG, "hello"},
                    0,
                    router_args_type{}},
         std::tuple{parsing::token_type{parsing::prefix_type::NONE, "one"},
                    1,
                    router_args_type{}},
         std::tuple{parsing::token_type{parsing::prefix_type::NONE, "two"},
                    1,
                    router_args_type{
                        false,
                        std::optional{std::vector<std::string_view>{"one"}},
                        {},
                        std::optional<std::vector<double>>{}}},
         std::tuple{
             parsing::token_type{parsing::prefix_type::NONE, "42"},
             2,
             router_args_type{
                 {},
                 std::optional{std::vector<std::string_view>{"one", "two"}},
                 {},
                 std::optional<std::vector<double>>{}}},
         std::tuple{
             parsing::token_type{parsing::prefix_type::NONE, "3.0"},
             3,
             router_args_type{
                 false,
                 std::optional{std::vector<std::string_view>{"one", "two"}},
                 42,
                 std::optional<std::vector<double>>{}}},
         std::tuple{
             parsing::token_type{parsing::prefix_type::NONE, "3.14"},
             3,
             router_args_type{
                 false,
                 std::optional{std::vector<std::string_view>{"one", "two"}},
                 42,
                 std::optional<std::vector<double>>{3.0}}}});
}

BOOST_AUTO_TEST_CASE(numeric_parse_test)
{
    auto f = [](auto input, auto expected, std::string_view fail_message) {
        using T = std::decay_t<decltype(expected)>;

        try {
            const auto result = parser<T>::parse(input);
            static_assert(std::is_same_v<std::decay_t<decltype(result)>, T>,
                          "Parse result unexpected type");
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{"42", 42, ""},
                       std::tuple{"+42", 42, ""},
                       std::tuple{"-42", -42, ""},
                       std::tuple{"3.14", 3.14, ""},
                       std::tuple{"3.14", 3.14f, ""},
                       std::tuple{"+3.14", 3.14f, ""},
                       std::tuple{"-3.14", -3.14f, ""},
                       std::tuple{"hello", 42, "Failed to parse: hello"},
                       std::tuple{"23742949",
                                  std::uint8_t{0},
                                  "Value out of range for argument: 23742949"},
                   });
}

BOOST_AUTO_TEST_CASE(string_view_parse_test)
{
    auto f = [](auto input, auto expected) {
        const auto result = parser<std::string_view>::parse(input);
        static_assert(
            std::is_same_v<std::decay_t<decltype(result)>, std::string_view>,
            "Parse result unexpected type");
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(f,
                   {
                       std::tuple{"hello", "hello"},
                       std::tuple{"a", "a"},
                       std::tuple{"", ""},
                   });
}

BOOST_AUTO_TEST_CASE(bool_parse_test)
{
    auto f = [](auto input, auto expected, std::string_view fail_message) {
        try {
            const auto result = parser<bool>::parse(input);
            static_assert(std::is_same_v<std::decay_t<decltype(result)>, bool>,
                          "Parse result unexpected type");
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(f,
                   {
                       std::tuple{"true", true, ""},
                       std::tuple{"yes", true, ""},
                       std::tuple{"y", true, ""},
                       std::tuple{"on", true, ""},
                       std::tuple{"1", true, ""},
                       std::tuple{"enable", true, ""},
                       std::tuple{"false", false, ""},
                       std::tuple{"no", false, ""},
                       std::tuple{"n", false, ""},
                       std::tuple{"off", false, ""},
                       std::tuple{"0", false, ""},
                       std::tuple{"disable", false, ""},
                       std::tuple{"hello", false, "Failed to parse: hello"},
                   });
}

BOOST_AUTO_TEST_CASE(container_parse_test)
{
    auto f = [](auto input, auto expected, std::string_view fail_message) {
        using T = std::vector<std::decay_t<decltype(expected)>>;

        try {
            const auto result = parser<T>::parse(input);
            static_assert(std::is_same_v<std::decay_t<decltype(result)>,
                                         typename T::value_type>,
                          "Parse result unexpected type");
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{"42", 42, ""},
                       std::tuple{"true", true, ""},
                       std::tuple{"3.14", 3.14f, ""},
                       std::tuple{"hello", "hello"sv, ""},
                       std::tuple{"hello", false, "Failed to parse: hello"},
                       std::tuple{"23742949",
                                  std::uint8_t{0},
                                  "Value out of range for argument: 23742949"},
                   });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(unimplemented_parse_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/parsing.hpp"

struct my_struct{};

int main() {
    const auto v = arg_router::parser<my_struct>::parse("foo");
    return 0;
}
    )",
        "No parse function for this type, use a custom_parser policy or define "
        "a parse(std::string_view) specialisation");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
