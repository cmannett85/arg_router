#include "arg_router/policy/max_count.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(max_count_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(
        policy::is_policy_v<policy::max_count_t<traits::integral_constant<42>>>,
        "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(max_count_test)
{
    static_assert(policy::max_count<42u>.maximum_count() == 42, "Fail");
    static_assert(policy::max_count<5>.maximum_count() == 5, "Fail");
    static_assert(policy::max_count<0>.maximum_count() == 0, "Fail");
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(value_type_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/max_count.hpp"

struct my_type {};

int main() {
    const auto tmp = arg_router::policy::max_count_t<my_type>{};
    return 0;
}
    )",
        "T must have a value_type");
}

BOOST_AUTO_TEST_CASE(integral_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/max_count.hpp"

struct my_type {
    using value_type = std::string;
};

int main() {
    const auto tmp = arg_router::policy::max_count_t<my_type>{};
    return 0;
}
    )",
        "T must have a value_type that is implicitly convertible to "
        "std::size_t");
}

BOOST_AUTO_TEST_CASE(conversion_test)
{
    test::death_test_compile(
        R"(
#include <string>
#include "arg_router/policy/max_count.hpp"

struct my_type {
    using value_type = double;
};

int main() {
    const auto tmp = arg_router::policy::max_count_t<my_type>{};
    return 0;
}
    )",
        "T must be an integral type");
}

BOOST_AUTO_TEST_CASE(greater_than_or_equal_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/policy/max_count.hpp"

using namespace arg_router;
int main() {
    const auto tmp = policy::max_count_t<traits::integral_constant<-5>>{};
    return 0;
}
    )",
        "T must have a value_type that is a positive number");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
