#include "arg_router/arg.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/short_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(arg_suite)

BOOST_AUTO_TEST_CASE(is_tree_node_test)
{
    static_assert(is_tree_node_v<arg_t<int>>, "Tree node test has failed");
}

BOOST_AUTO_TEST_CASE(policies_test)
{
    auto f = arg<int>(policy::long_name<S_("hello")>, policy::short_name<'H'>);
    static_assert(f.long_name() == "hello"sv, "Long name test fail");
    static_assert(f.short_name() == "H", "Short name test fail");
}

BOOST_AUTO_TEST_CASE(match_test)
{
    {
        const auto f =
            arg<int>(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result = f.match({parsing::prefix_type::LONG, "hello"});
        BOOST_CHECK_EQUAL(
            result,
            (parsing::match_result{parsing::match_result::MATCH,
                                   parsing::match_result::HAS_ARGUMENT}));
    }

    {
        const auto f =
            arg<int>(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result = f.match('H');
        BOOST_CHECK_EQUAL(
            result,
            (parsing::match_result{parsing::match_result::MATCH,
                                   parsing::match_result::HAS_ARGUMENT}));
    }

    {
        const auto f =
            arg<int>(policy::long_name<S_("hello")>, policy::short_name<'H'>);
        const auto result = f.match({parsing::prefix_type::LONG, "foo"});
        BOOST_CHECK_EQUAL(
            result,
            (parsing::match_result{parsing::match_result::NO_MATCH,
                                   parsing::match_result::HAS_ARGUMENT}));
    }

    {
        const auto f = arg<int>(policy::long_name<S_("hello")>);
        const auto result = f.match({parsing::prefix_type::LONG, "hello"});
        BOOST_CHECK_EQUAL(
            result,
            (parsing::match_result{parsing::match_result::MATCH,
                                   parsing::match_result::HAS_ARGUMENT}));
    }

    {
        const auto f = arg<int>(policy::long_name<S_("hello")>);
        const auto result = f.match({parsing::prefix_type::LONG, "foo"});
        BOOST_CHECK_EQUAL(
            result,
            (parsing::match_result{parsing::match_result::NO_MATCH,
                                   parsing::match_result::HAS_ARGUMENT}));
    }

    {
        const auto f = arg<int>(policy::short_name<'H'>);
        const auto result = f.match('H');
        BOOST_CHECK_EQUAL(
            result,
            (parsing::match_result{parsing::match_result::MATCH,
                                   parsing::match_result::HAS_ARGUMENT}));
    }

    {
        const auto f = arg<int>(policy::short_name<'H'>);
        const auto result = f.match('a');
        BOOST_CHECK_EQUAL(
            result,
            (parsing::match_result{parsing::match_result::NO_MATCH,
                                   parsing::match_result::HAS_ARGUMENT}));
    }
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
