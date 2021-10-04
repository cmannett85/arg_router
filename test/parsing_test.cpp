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
    using flag_type = flag_t<policy::long_name_t<S_("flag1")>,
                             policy::alias_t<policy::long_name_t<S_("Hello")>>>;

    static_assert(
        traits::is_detected_v<parsing::has_aliased_node_indices, flag_type>,
        "Fail");
}

BOOST_AUTO_TEST_CASE(flag_default_match_test)
{
    {
        const auto f =
            flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "hello"});
        BOOST_CHECK_EQUAL(result,
                          parsing::match_result{parsing::match_result::MATCH});
    }

    {
        const auto f =
            flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>('H');
        BOOST_CHECK_EQUAL(result,
                          parsing::match_result{parsing::match_result::MATCH});
    }

    {
        const auto f =
            flag(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "foo"});
        BOOST_CHECK_EQUAL(result, parsing::match_result{});
    }

    {
        const auto f = flag(policy::long_name<S_("hello")>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "hello"});
        BOOST_CHECK_EQUAL(result,
                          parsing::match_result{parsing::match_result::MATCH});
    }

    {
        const auto f = flag(policy::long_name<S_("hello")>);
        const auto result = parsing::default_match<std::decay_t<decltype(f)>>(
            {parsing::prefix_type::LONG, "foo"});
        BOOST_CHECK_EQUAL(result, parsing::match_result{});
    }

    {
        const auto f = flag(policy::short_name<'H'>);
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>('H');
        BOOST_CHECK_EQUAL(result,
                          parsing::match_result{parsing::match_result::MATCH});
    }

    {
        const auto f = flag(policy::short_name<'H'>);
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>('a');
        BOOST_CHECK_EQUAL(result, parsing::match_result{});
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

BOOST_AUTO_TEST_CASE(build_router_args_test)
{
    {
        using type = root_t<
            flag_t<policy::long_name_t<S_("hello")>>,
            std::decay_t<decltype(policy::validation::default_validator)>>;
        static_assert(std::is_same_v<parsing::build_router_args_t<type>,
                                     std::tuple<bool>>,
                      "Build router args test 1 fail");
    }

    {
        using type = root_t<
            flag_t<policy::long_name_t<S_("hello")>>,
            flag_t<policy::long_name_t<S_("goodbye")>>,
            std::decay_t<decltype(policy::validation::default_validator)>>;
        static_assert(std::is_same_v<parsing::build_router_args_t<type>,
                                     std::tuple<bool, bool>>,
                      "Build router args test 1 fail");
    }

    {
        using type = flag_t<policy::long_name_t<S_("hello")>>;
        static_assert(std::is_same_v<parsing::build_router_args_t<type>,
                                     std::tuple<bool>>,
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

    auto visitor_hit_count = 0u;
    auto v1 = [&](auto i, auto&& child, auto match) {
        BOOST_CHECK_EQUAL(i, 0);
        if constexpr (std::is_same_v<std::decay_t<decltype(child)>,
                                     std::tuple_element_t<
                                         0,
                                         typename std::decay_t<
                                             decltype(r)>::children_type>>) {
            ++visitor_hit_count;
        }

        BOOST_CHECK_EQUAL(match,
                          parsing::match_result{parsing::match_result::MATCH});
    };

    parsing::visit_child({parsing::prefix_type::LONG, "hello"},
                         r.children(),
                         v1);
    BOOST_CHECK_EQUAL(visitor_hit_count, 1);

    visitor_hit_count = 0;
    auto v2 = [&](auto i, auto&& child, auto match) {
        BOOST_CHECK_EQUAL(i, 1);
        if constexpr (std::is_same_v<std::decay_t<decltype(child)>,
                                     std::tuple_element_t<
                                         1,
                                         typename std::decay_t<
                                             decltype(r)>::children_type>>) {
            ++visitor_hit_count;
        }

        BOOST_CHECK_EQUAL(match,
                          parsing::match_result{parsing::match_result::MATCH});
    };

    parsing::visit_child('h', r.children(), v2);
    BOOST_CHECK_EQUAL(visitor_hit_count, 1);

    visitor_hit_count = 0;
    auto v3 = [&](auto i, auto&& child, auto match) {
        BOOST_CHECK_EQUAL(i, 2);
        if constexpr (std::is_same_v<std::decay_t<decltype(child)>,
                                     std::tuple_element_t<
                                         2,
                                         typename std::decay_t<
                                             decltype(r)>::children_type>>) {
            ++visitor_hit_count;
        }

        BOOST_CHECK_EQUAL(match,
                          parsing::match_result{parsing::match_result::MATCH});
    };

    parsing::visit_child('b', r.children(), v3);
    BOOST_CHECK_EQUAL(visitor_hit_count, 1);
}

BOOST_AUTO_TEST_CASE(numeric_parse_test)
{
    auto f = [](auto input, auto expected, std::string_view fail_message) {
        using T = std::decay_t<decltype(expected)>;

        try {
            const auto result = parse<T>(input);
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
        const auto result = parse<std::string_view>(input);
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
            const auto result = parse<bool>(input);
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

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(unimplemented_parse_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/parsing.hpp"

struct my_struct{};

int main() {
    const auto v = arg_router::parse<my_struct>("foo");
    return 0;
}
    )",
        "No parse function for this type, use a custom_parser policy or define "
        "a parse(std::string_view) specialisation");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
