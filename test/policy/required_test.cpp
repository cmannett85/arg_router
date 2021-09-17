#include "arg_router/policy/required.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(required_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::required_t<>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(is_required_test)
{
    static_assert(!policy::is_required_v<float>);
    static_assert(!(policy::is_required_v<policy::long_name_t<S_("hello")>>));
    static_assert(policy::is_required_v<policy::required_t<>>);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
