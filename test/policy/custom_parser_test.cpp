#include "arg_router/policy/custom_parser.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(custom_parser_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::custom_parser<int>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(echo_parse_test)
{
    auto cp =
        policy::custom_parser<std::string_view>{[](auto str) { return str; }};
    const auto result = cp.parse("hello");

    BOOST_CHECK_EQUAL(result, "hello");
    BOOST_CHECK((std::is_same_v<decltype(cp)::value_type, std::string_view>));
    BOOST_CHECK(
        (std::is_same_v<decltype(cp)::parser_type,
                        std::function<std::string_view(std::string_view)>>));
}

BOOST_AUTO_TEST_CASE(num_parse_test)
{
    auto cp = policy::custom_parser<int>{[](auto /*str*/) { return 42; }};
    const auto result = cp.parse("hello");

    BOOST_CHECK_EQUAL(result, 42);
    BOOST_CHECK((std::is_same_v<decltype(cp)::value_type, int>));
    BOOST_CHECK((std::is_same_v<decltype(cp)::parser_type,
                                std::function<int(std::string_view)>>));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
