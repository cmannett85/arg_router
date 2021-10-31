#include "arg_router/traits.hpp"

#include "test_helpers.hpp"

#include <array>
#include <deque>
#include <variant>

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(traits_suite)

BOOST_AUTO_TEST_CASE(is_tuple_like_test)
{
    auto f = [](auto type, auto expected) {
        const auto result =
            traits::is_tuple_like_v<std::decay_t<decltype(type)>>;
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::tuple{}, true},
                       std::tuple{std::tuple{42, 3.14}, true},
                       std::tuple{42, false},
                       std::tuple{3.14, false},
                       std::tuple{"hello", false},
                   });
}

BOOST_AUTO_TEST_CASE(is_specialisation_test)
{
    struct test {
    };

    auto result = traits::is_specialisation_v<std::vector<int>>;
    BOOST_CHECK_EQUAL(result, true);

    result = traits::is_specialisation_v<std::deque<int>>;
    BOOST_CHECK_EQUAL(result, true);

    result = traits::is_specialisation_v<std::string_view>;
    BOOST_CHECK_EQUAL(result, true);

    result = traits::is_specialisation_v<std::tuple<char, int, double>>;
    BOOST_CHECK_EQUAL(result, true);

    result = traits::is_specialisation_v<float>;
    BOOST_CHECK_EQUAL(result, false);

    result = traits::is_specialisation_v<test>;
    BOOST_CHECK_EQUAL(result, false);
}

BOOST_AUTO_TEST_CASE(is_specialisation_of_test)
{
    auto result = traits::is_specialisation_of_v<std::vector<int>, std::vector>;
    BOOST_CHECK_EQUAL(result, true);

    result = traits::is_specialisation_of_v<std::vector<int>,
                                            std::basic_string_view>;
    BOOST_CHECK_EQUAL(result, false);

    result = traits::is_specialisation_of_v<std::vector<int>, std::deque>;
    BOOST_CHECK_EQUAL(result, false);

    result = traits::is_specialisation_of_v<double, std::deque>;
    BOOST_CHECK_EQUAL(result, false);
}

BOOST_AUTO_TEST_CASE(is_same_when_despecialised_test)
{
    auto result = traits::is_same_when_despecialised_v<std::vector<int>,
                                                       std::vector<int>>;
    BOOST_CHECK_EQUAL(result, true);

    result = traits::is_same_when_despecialised_v<std::vector<int>,
                                                  std::vector<double>>;
    BOOST_CHECK_EQUAL(result, true);

    result =
        traits::is_same_when_despecialised_v<std::vector<int>, std::deque<int>>;
    BOOST_CHECK_EQUAL(result, false);

    result = traits::is_same_when_despecialised_v<std::vector<int>, int>;
    BOOST_CHECK_EQUAL(result, false);

    result = traits::is_same_when_despecialised_v<int, std::vector<int>>;
    BOOST_CHECK_EQUAL(result, false);

    result = traits::is_same_when_despecialised_v<int, int>;
    BOOST_CHECK_EQUAL(result, false);
}

BOOST_AUTO_TEST_CASE(integral_constant_test)
{
    const auto r1 = traits::integral_constant<-42>{};
    BOOST_CHECK((std::is_same_v<typename decltype(r1)::value_type, int>));
    BOOST_CHECK_EQUAL(r1.value, -42);

    const auto r2 = traits::integral_constant<std::size_t{42}>{};
    BOOST_CHECK(
        (std::is_same_v<typename decltype(r2)::value_type, std::size_t>));
    BOOST_CHECK_EQUAL(r2.value, 42);
}

BOOST_AUTO_TEST_SUITE_END()
