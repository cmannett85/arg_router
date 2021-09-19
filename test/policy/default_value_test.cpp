#include "arg_router/policy/default_value.hpp"

#include "test_helpers.hpp"

#include <string_view>

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(default_value_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::default_value<int>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(constructor_and_get_test)
{
    auto f = [](auto input, auto is_ref) {
        using T = std::decay_t<decltype(input)>;

        const auto dv = policy::default_value<T>{input};
        BOOST_CHECK((std::is_same_v<typename decltype(dv)::value_type, T>));

        const auto result = dv.get_default_value();
        BOOST_CHECK(input == result);

        using result_type = decltype(std::declval<policy::default_value<T>>()
                                         .get_default_value());
        if (is_ref) {
            BOOST_CHECK((std::is_same_v<result_type, const T&>));
        } else {
            BOOST_CHECK((std::is_same_v<result_type, T>));
        }
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{42, false},
                       std::tuple{3.14, false},
                       std::tuple{"hello"sv, false},
                       // Should be bigger than any 'normal' L1 cache
                       std::tuple{std::array<char, 256>{}, true},
                   });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
