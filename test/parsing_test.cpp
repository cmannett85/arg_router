#include "arg_router/parsing.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/policy/validator.hpp"
#include "arg_router/root.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(parsing_suite)

BOOST_AUTO_TEST_CASE(flag_default_match_test)
{
    {
        const auto f =
            flag{policy::long_name<S_("hello")>, policy::short_name<'H'>};
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>("hello");
        BOOST_CHECK(result.matched == parsing::match_result::MATCH);
        BOOST_CHECK(result.has_argument ==
                    parsing::match_result::HAS_NO_ARGUMENT);
    }

    {
        const auto f =
            flag{policy::long_name<S_("hello")>, policy::short_name<'H'>};
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>("H");
        BOOST_CHECK(result.matched == parsing::match_result::MATCH);
        BOOST_CHECK(result.has_argument ==
                    parsing::match_result::HAS_NO_ARGUMENT);
    }

    {
        const auto f =
            flag{policy::long_name<S_("hello")>, policy::short_name<'H'>};
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>("foo");
        BOOST_CHECK(result.matched == parsing::match_result::NO_MATCH);
        BOOST_CHECK(result.has_argument ==
                    parsing::match_result::HAS_NO_ARGUMENT);
    }

    {
        const auto f = flag{policy::long_name<S_("hello")>};
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>("hello");
        BOOST_CHECK(result.matched == parsing::match_result::MATCH);
        BOOST_CHECK(result.has_argument ==
                    parsing::match_result::HAS_NO_ARGUMENT);
    }

    {
        const auto f = flag{policy::long_name<S_("hello")>};
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>("foo");
        BOOST_CHECK(result.matched == parsing::match_result::NO_MATCH);
        BOOST_CHECK(result.has_argument ==
                    parsing::match_result::HAS_NO_ARGUMENT);
    }

    {
        const auto f = flag{policy::short_name<'H'>};
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>("H");
        BOOST_CHECK(result.matched == parsing::match_result::MATCH);
        BOOST_CHECK(result.has_argument ==
                    parsing::match_result::HAS_NO_ARGUMENT);
    }

    {
        const auto f = flag{policy::short_name<'H'>};
        const auto result =
            parsing::default_match<std::decay_t<decltype(f)>>("a");
        BOOST_CHECK(result.matched == parsing::match_result::NO_MATCH);
        BOOST_CHECK(result.has_argument ==
                    parsing::match_result::HAS_NO_ARGUMENT);
    }
}

BOOST_AUTO_TEST_CASE(get_prefix_type_test)
{
    auto f = [](auto token, auto expected_prefix, auto expected_stripped) {
        const auto [prefix, stripped] = parsing::get_prefix_type(token);
        BOOST_CHECK(prefix == expected_prefix);
        BOOST_CHECK_EQUAL(stripped, expected_stripped);
    };

    test::data_set(f,
                   {std::tuple{"--hello", parsing::prefix_type::LONG, "hello"},
                    std::tuple{"-h", parsing::prefix_type::SHORT, "h"},
                    std::tuple{"hello", parsing::prefix_type::NONE, "hello"},
                    std::tuple{"", parsing::prefix_type::NONE, ""}});
}

BOOST_AUTO_TEST_CASE(expand_arguments_test)
{
    auto f = [](auto input, auto expected) {
        const auto result =
            parsing::expand_arguments(input.size(), input.data());
        BOOST_REQUIRE_EQUAL(result.size(), expected.size());
        for (auto i = 0u; i < result.size(); ++i) {
            BOOST_CHECK_EQUAL(result[i], expected[i]);
        }
    };

    test::data_set(
        f,
        {
            std::tuple{std::vector{"--f", "foo", "-g", "-d", "42"},
                       std::deque{"--f", "foo", "-g", "-d", "42"}},
            std::tuple{std::vector{"-fwed"},
                       std::deque{"-f", "-w", "-e", "-d"}},
            std::tuple{std::vector{"--foo", "42", "-venv", "-d", "-abc"},
                       std::deque{"--foo",
                                  "42",
                                  "-v",
                                  "-e",
                                  "-n",
                                  "-v",
                                  "-d",
                                  "-a",
                                  "-b",
                                  "-c"}},
        });
}

BOOST_AUTO_TEST_CASE(build_router_args_test)
{
    {
        using type =
            root<flag<policy::long_name_t<S_("hello")>>,
                 std::decay_t<decltype(policy::validation::default_validator)>>;
        static_assert(std::is_same_v<parsing::build_router_args_t<type>,
                                     std::tuple<bool>>,
                      "Build router args test 1 fail");
    }

    {
        using type =
            root<flag<policy::long_name_t<S_("hello")>>,
                 flag<policy::long_name_t<S_("goodbye")>>,
                 std::decay_t<decltype(policy::validation::default_validator)>>;
        static_assert(std::is_same_v<parsing::build_router_args_t<type>,
                                     std::tuple<bool, bool>>,
                      "Build router args test 1 fail");
    }

    {
        using type = flag<policy::long_name_t<S_("hello")>>;
        static_assert(std::is_same_v<parsing::build_router_args_t<type>,
                                     std::tuple<bool>>,
                      "Build router args test 1 fail");
    }
}

BOOST_AUTO_TEST_CASE(visit_child_test)
{
    const auto r = root{flag{policy::long_name<S_("hello")>,
                             policy::description<S_("Hello description")>,
                             policy::router{[]() {}}},
                        flag{policy::short_name<'h'>,
                             policy::description<S_("h description")>,
                             policy::router{[]() {}}},
                        flag{policy::short_name<'b'>,
                             policy::description<S_("b description")>,
                             policy::router{[]() {}}},
                        policy::validation::default_validator};

    auto visitor_hit_count = 0u;
    auto v1 = [&](auto i, auto&& child, auto match) {
        BOOST_CHECK_EQUAL(i, 0);
        if constexpr (std::is_same_v<
                          std::decay_t<decltype(child)>,
                          std::tuple_element_t<0,
                                               typename std::decay_t<decltype(
                                                   r)>::children_type>>) {
            ++visitor_hit_count;
        }

        BOOST_CHECK(match.matched == parsing::match_result::MATCH);
        BOOST_CHECK(match.has_argument ==
                    parsing::match_result::HAS_NO_ARGUMENT);
    };

    parsing::visit_child<parsing::prefix_type::LONG>(r.children(), "hello", v1);
    BOOST_CHECK_EQUAL(visitor_hit_count, 1);

    visitor_hit_count = 0;
    auto v2 = [&](auto i, auto&& child, auto match) {
        BOOST_CHECK_EQUAL(i, 1);
        if constexpr (std::is_same_v<
                          std::decay_t<decltype(child)>,
                          std::tuple_element_t<1,
                                               typename std::decay_t<decltype(
                                                   r)>::children_type>>) {
            ++visitor_hit_count;
        }

        BOOST_CHECK(match.matched == parsing::match_result::MATCH);
        BOOST_CHECK(match.has_argument ==
                    parsing::match_result::HAS_NO_ARGUMENT);
    };

    parsing::visit_child<parsing::prefix_type::LONG>(r.children(), "h", v2);
    BOOST_CHECK_EQUAL(visitor_hit_count, 1);

    visitor_hit_count = 0;
    auto v3 = [&](auto i, auto&& child, auto match) {
        BOOST_CHECK_EQUAL(i, 2);
        if constexpr (std::is_same_v<
                          std::decay_t<decltype(child)>,
                          std::tuple_element_t<2,
                                               typename std::decay_t<decltype(
                                                   r)>::children_type>>) {
            ++visitor_hit_count;
        }

        BOOST_CHECK(match.matched == parsing::match_result::MATCH);
        BOOST_CHECK(match.has_argument ==
                    parsing::match_result::HAS_NO_ARGUMENT);
    };

    parsing::visit_child<parsing::prefix_type::LONG>(r.children(), "b", v3);
    BOOST_CHECK_EQUAL(visitor_hit_count, 1);
}

BOOST_AUTO_TEST_SUITE_END()
