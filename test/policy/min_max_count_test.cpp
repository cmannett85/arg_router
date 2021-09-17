#include "arg_router/policy/min_max_count.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(min_max_count_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    BOOST_CHECK(policy::is_policy_v<
                policy::min_count_t<traits::integral_constant<42>>>);
}

BOOST_AUTO_TEST_CASE(min_count_test)
{
    const auto min_val = policy::min_count<42>;
    BOOST_CHECK_EQUAL(min_val.minimum_count(), 42);
}

BOOST_AUTO_TEST_CASE(max_count_test)
{
    const auto max_val = policy::max_count<42>;
    BOOST_CHECK_EQUAL(max_val.maximum_count(), 42);
}

BOOST_AUTO_TEST_CASE(validation_test)
{
    policy::min_count<42>.validate<policy::min_count_t<traits::integral_constant<3>>>();
    policy::max_count<24>.validate<policy::max_count_t<traits::integral_constant<5>>>();

    policy::min_count<42>.validate<policy::min_count_t<traits::integral_constant<3>>, //
        policy::max_count_t<traits::integral_constant<5>>>();
    policy::min_count<42>.validate<policy::min_count_t<traits::integral_constant<5>>, //
        policy::max_count_t<traits::integral_constant<5>>>();
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(min_count_not_integral_value_type_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/min_max_count.hpp"

struct string_constant {
    using value_type = std::string_view;
};

int main() {
    const auto my_min = arg_router::policy::min_count_t<string_constant>{};
    return 0;
}
    )",
        "T must have a value_type that is an integral");
}

BOOST_AUTO_TEST_CASE(max_count_not_integral_value_type_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/min_max_count.hpp"

struct string_constant {
    using value_type = std::string_view;
};

int main() {
    const auto my_max = arg_router::policy::max_count_t<string_constant>{};
    return 0;
}
    )",
        "T must have a value_type that is an integral");
}

BOOST_AUTO_TEST_CASE(failed_validation_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/min_max_count.hpp"
int main() {
    arg_router::policy::min_count<42>.validate<
        arg_router::policy::min_count_t<arg_router::traits::integral_constant<5>>,
        arg_router::policy::max_count_t<arg_router::traits::integral_constant<3>>>();
    return 0;
}
    )",
        "Minimum count must be less than maximum count");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
