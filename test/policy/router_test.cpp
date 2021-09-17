#include "arg_router/policy/router.hpp"

#include "test_helpers.hpp"

#include <optional>

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(policy_suite)

BOOST_AUTO_TEST_SUITE(router_suite)

BOOST_AUTO_TEST_CASE(is_policy_test)
{
    static_assert(policy::is_policy_v<policy::router<std::function<void()>>>,
                  "Policy test has failed");
}

BOOST_AUTO_TEST_CASE(type_test)
{
    {
        using router_type = policy::router<std::function<void()>>;
        BOOST_CHECK((std::is_same_v<typename router_type::callable_type,
                                    std::function<void()>>));
    }

    {
        using router_type = policy::router<std::function<double()>>;
        BOOST_CHECK((std::is_same_v<typename router_type::callable_type,
                                    std::function<double()>>));
    }

    {
        using router_type = policy::router<std::function<double(float, int)>>;
        BOOST_CHECK((std::is_same_v<typename router_type::callable_type,
                                    std::function<double(float, int)>>));
    }
}

BOOST_AUTO_TEST_CASE(constructor_test)
{
    auto fn_hit = false;
    auto f = [&](auto fn, auto args_tuple) {
        fn_hit = false;

        auto r = policy::router{std::move(fn)};

        std::apply(r, args_tuple);
        BOOST_CHECK(fn_hit);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{[&]() { fn_hit = true; },  //
                       std::tuple{}},
            std::tuple{[&](auto val) {
                           fn_hit = true;
                           BOOST_CHECK_EQUAL(val, 13.6);
                       },  //
                       std::tuple{13.6}},
            std::tuple{[&](auto... val) {
                           static_assert(sizeof...(val) == 5,
                                         "Incorrect pack size");
                           auto val_tuple = std::tuple{val...};
                           fn_hit = true;
                           BOOST_CHECK_EQUAL(std::get<0>(val_tuple), 1);
                           BOOST_CHECK_EQUAL(std::get<1>(val_tuple), 2);
                           BOOST_CHECK_EQUAL(std::get<2>(val_tuple), 3);
                           BOOST_CHECK_EQUAL(std::get<3>(val_tuple), 4);
                           BOOST_CHECK_EQUAL(std::get<4>(val_tuple), 5);
                       },  //
                       std::tuple{1, 2, 3, 4, 5}},
        });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
