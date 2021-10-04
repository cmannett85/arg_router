#include "arg_router/algorithm.hpp"

#include "test_helpers.hpp"

#include <boost/preprocessor/seq/for_each.hpp>

#include <string>
#include <vector>

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(algorithm_suite)

BOOST_AUTO_TEST_CASE(is_alnum_test)
{
    utility::tuple_iterator(
        [](auto i, auto) {
            constexpr auto valid = std::string_view{"abcdefghijklmnopqrstuvwxyz"
                                                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                    "0123456789"};
            constexpr auto expected = valid.find(i) != std::string_view::npos;

            constexpr auto result = algorithm::is_alnum(i);
            static_assert(expected == result, "is_alnum_test failed");
        },
        std::make_index_sequence<std::numeric_limits<char>::max()>());
}

BOOST_AUTO_TEST_CASE(is_whitespace_test)
{
    utility::tuple_iterator(
        [](auto i, auto) {
            constexpr auto expected = (i == ' ') || (i == '\f') ||
                                      (i == '\n') || (i == '\r') ||
                                      (i == '\t') || (i == '\v');

            constexpr auto result = algorithm::is_whitespace(i);
            static_assert(expected == result, "is_whitespace failed");
        },
        std::make_index_sequence<std::numeric_limits<char>::max()>());
}

BOOST_AUTO_TEST_CASE(contains_whitespace_test)  //
{
    // Although std::string_view is constexpr, it's type is the same between
    // our strings, which means we can't use it as above because you can't
    // initialise a constexpr value from a function parameter.  So we'll have
    // to use the preprocessor to generate the code instead
#define CONTAINS_SEQ      \
    ("he llo")("hello ")( \
        " hello")("hell\to")("hel\nlo")("he     llo")("h\fello")("h\rello")("h\vello")

#define NOT_CONTAINS_SEQ ("")("hello")

#define MACRO(r, expected, elem)                                         \
    {                                                                    \
        constexpr auto result = algorithm::contains_whitespace(elem);    \
        static_assert(expected == result, "contains_whitespace failed"); \
    }

    BOOST_PP_SEQ_FOR_EACH(MACRO, true, CONTAINS_SEQ);
    BOOST_PP_SEQ_FOR_EACH(MACRO, false, NOT_CONTAINS_SEQ);

#undef CONTAINS_SEQ
#undef NOT_CONTAINS_SEQ
#undef MACRO
}

BOOST_AUTO_TEST_CASE(is_unique_set_test)
{
    BOOST_CHECK((algorithm::is_unique_set_v<
                 std::tuple<int, float, double, std::string_view>>));
    BOOST_CHECK(
        !(algorithm::is_unique_set_v<
            std::tuple<int, std::vector<double>, double, std::vector<int>>>));
    BOOST_CHECK(
        !(algorithm::is_unique_set_v<
            std::
                tuple<int, std::vector<double>, double, std::vector<double>>>));
    BOOST_CHECK(!(algorithm::is_unique_set_v<
                  std::tuple<double, float, double, std::string_view>>));
    BOOST_CHECK((algorithm::is_unique_set_v<std::tuple<int>>));
    BOOST_CHECK((algorithm::is_unique_set_v<std::tuple<>>));
}

BOOST_AUTO_TEST_CASE(find_specialisation_test)
{
    BOOST_CHECK_EQUAL(
        (algorithm::find_specialisation<
            std::vector,
            std::tuple<int, std::string, double, std::vector<int>, float>>::
             value),
        3);
    BOOST_CHECK_EQUAL(
        (algorithm::find_specialisation<std::vector,
                                        std::tuple<int,
                                                   std::vector<double>,
                                                   double,
                                                   std::vector<int>,
                                                   float>>::value),
        1);
    BOOST_CHECK_EQUAL((algorithm::find_specialisation<
                          std::vector,
                          std::tuple<int, std::string, double, float>>::value),
                      4);
    BOOST_CHECK_EQUAL(
        (algorithm::find_specialisation<std::vector, std::tuple<>>::value),
        0);
}

BOOST_AUTO_TEST_CASE(find_specialisation_v_test)
{
    BOOST_CHECK_EQUAL(
        (algorithm::find_specialisation_v<
            std::vector,
            std::tuple<int, std::string, double, std::vector<int>, float>>),
        3);
    BOOST_CHECK_EQUAL(
        (algorithm::find_specialisation_v<std::vector,
                                          std::tuple<int,
                                                     std::vector<double>,
                                                     double,
                                                     std::vector<int>,
                                                     float>>),
        1);
    BOOST_CHECK_EQUAL((algorithm::find_specialisation_v<
                          std::vector,
                          std::tuple<int, std::string, double, float>>),
                      4);
    BOOST_CHECK_EQUAL(
        (algorithm::find_specialisation_v<std::vector, std::tuple<>>),
        0);
}

BOOST_AUTO_TEST_CASE(count_specialisation_test)
{
    BOOST_CHECK_EQUAL(
        (algorithm::count_specialisation<
            std::vector,
            std::tuple<int, std::string, double, std::vector<int>, float>>::
             value),
        1);
    BOOST_CHECK_EQUAL(
        (algorithm::count_specialisation<std::vector,
                                         std::tuple<int,
                                                    std::vector<double>,
                                                    double,
                                                    std::vector<int>,
                                                    float>>::value),
        2);
    BOOST_CHECK_EQUAL((algorithm::count_specialisation<
                          std::vector,
                          std::tuple<int, std::string, double, float>>::value),
                      0);
    BOOST_CHECK_EQUAL(
        (algorithm::count_specialisation<std::vector, std::tuple<>>::value),
        0);
}

BOOST_AUTO_TEST_CASE(count_specialisation_v_test)
{
    BOOST_CHECK_EQUAL(
        (algorithm::count_specialisation_v<
            std::vector,
            std::tuple<int, std::string, double, std::vector<int>, float>>),
        1);
    BOOST_CHECK_EQUAL(
        (algorithm::count_specialisation_v<std::vector,
                                           std::tuple<int,
                                                      std::vector<double>,
                                                      double,
                                                      std::vector<int>,
                                                      float>>),
        2);
    BOOST_CHECK_EQUAL((algorithm::count_specialisation_v<
                          std::vector,
                          std::tuple<int, std::string, double, float>>),
                      0);
    BOOST_CHECK_EQUAL(
        (algorithm::count_specialisation_v<std::vector, std::tuple<>>),
        0);
}

BOOST_AUTO_TEST_CASE(count_despecialised_test)
{
    BOOST_CHECK_EQUAL(
        (algorithm::count_despecialised<
            std::vector<double>,
            std::tuple<int, std::string, double, std::vector<int>, float>>::
             value),
        1);
    BOOST_CHECK_EQUAL(
        (algorithm::count_despecialised<std::vector<double>,
                                        std::tuple<int,
                                                   std::vector<double>,
                                                   double,
                                                   std::vector<int>,
                                                   float>>::value),
        2);
    BOOST_CHECK_EQUAL((algorithm::count_despecialised<
                          std::vector<double>,
                          std::tuple<int, std::string, double, float>>::value),
                      0);
    BOOST_CHECK_EQUAL((algorithm::count_despecialised<std::vector<double>,
                                                      std::tuple<>>::value),
                      0);
}

BOOST_AUTO_TEST_CASE(count_despecialised_v_test)
{
    BOOST_CHECK_EQUAL(
        (algorithm::count_despecialised_v<
            std::vector<double>,
            std::tuple<int, std::string, double, std::vector<int>, float>>),
        1);
    BOOST_CHECK_EQUAL(
        (algorithm::count_despecialised_v<std::vector<double>,
                                          std::tuple<int,
                                                     std::vector<double>,
                                                     double,
                                                     std::vector<int>,
                                                     float>>),
        2);
    BOOST_CHECK_EQUAL((algorithm::count_despecialised_v<
                          std::vector<double>,
                          std::tuple<int, std::string, double, float>>),
                      0);
    BOOST_CHECK_EQUAL(
        (algorithm::count_despecialised_v<std::vector<double>, std::tuple<>>),
        0);
}

BOOST_AUTO_TEST_CASE(has_specialisation_test)
{
    BOOST_CHECK_EQUAL(
        (algorithm::has_specialisation<
            std::vector,
            std::tuple<int, std::string, double, std::vector<int>, float>>::
             value),
        true);
    BOOST_CHECK_EQUAL(
        (algorithm::has_specialisation<std::vector,
                                       std::tuple<int,
                                                  std::vector<double>,
                                                  double,
                                                  std::vector<int>,
                                                  float>>::value),
        true);
    BOOST_CHECK_EQUAL((algorithm::has_specialisation<
                          std::vector,
                          std::tuple<int, std::string, double, float>>::value),
                      false);
    BOOST_CHECK_EQUAL(
        (algorithm::has_specialisation<std::vector, std::tuple<>>::value),
        false);
}

BOOST_AUTO_TEST_CASE(has_specialisation_v_test)
{
    BOOST_CHECK_EQUAL(
        (algorithm::has_specialisation_v<
            std::vector,
            std::tuple<int, std::string, double, std::vector<int>, float>>),
        true);
    BOOST_CHECK_EQUAL(
        (algorithm::has_specialisation_v<std::vector,
                                         std::tuple<int,
                                                    std::vector<double>,
                                                    double,
                                                    std::vector<int>,
                                                    float>>),
        true);
    BOOST_CHECK_EQUAL((algorithm::has_specialisation_v<
                          std::vector,
                          std::tuple<int, std::string, double, float>>),
                      false);
    BOOST_CHECK_EQUAL(
        (algorithm::has_specialisation_v<std::vector, std::tuple<>>),
        false);
}

BOOST_AUTO_TEST_CASE(zip_test)
{
    auto f = [](auto first, auto second, auto expected) {
        using first_type = std::decay_t<decltype(first)>;
        using second_type = std::decay_t<decltype(second)>;
        using expected_type = std::decay_t<decltype(expected)>;
        using result_type = algorithm::zip_t<first_type, second_type>;

        BOOST_CHECK((std::is_same_v<result_type, expected_type>));
    };

    using data_set = std::tuple<
        std::tuple<
            std::tuple<std::integral_constant<std::size_t, 0>,
                       std::integral_constant<std::size_t, 1>,
                       std::integral_constant<std::size_t, 2>>,
            std::tuple<float, int, std::string_view>,
            std::tuple<std::pair<std::integral_constant<std::size_t, 0>, float>,
                       std::pair<std::integral_constant<std::size_t, 1>, int>,
                       std::pair<std::integral_constant<std::size_t, 2>,
                                 std::string_view>>>,
        std::tuple<
            std::tuple<float, int, std::string_view>,
            std::tuple<std::integral_constant<std::size_t, 0>,
                       std::integral_constant<std::size_t, 1>,
                       std::integral_constant<std::size_t, 2>>,
            std::tuple<std::pair<float, std::integral_constant<std::size_t, 0>>,
                       std::pair<int, std::integral_constant<std::size_t, 1>>,
                       std::pair<std::string_view,
                                 std::integral_constant<std::size_t, 2>>>>>;

    test::data_set(f, data_set{});
}

BOOST_AUTO_TEST_CASE(unzip_test)
{
    auto f = [](auto input, auto expected_first, auto expected_second) {
        using input_type = std::decay_t<decltype(input)>;
        using expected_first_type = std::decay_t<decltype(expected_first)>;
        using expected_second_type = std::decay_t<decltype(expected_second)>;

        using result_type = algorithm::unzip<input_type>;
        using first_type = typename result_type::first_type;
        using second_type = typename result_type::second_type;

        BOOST_CHECK((std::is_same_v<first_type, expected_first_type>));
        BOOST_CHECK((std::is_same_v<second_type, expected_second_type>));
    };

    using data_set = std::tuple<
        std::tuple<
            std::tuple<std::pair<std::integral_constant<std::size_t, 0>, float>,
                       std::pair<std::integral_constant<std::size_t, 1>, int>,
                       std::pair<std::integral_constant<std::size_t, 2>,
                                 std::string_view>>,
            std::tuple<std::integral_constant<std::size_t, 0>,
                       std::integral_constant<std::size_t, 1>,
                       std::integral_constant<std::size_t, 2>>,
            std::tuple<float, int, std::string_view>>,
        std::tuple<
            std::tuple<std::pair<float, std::integral_constant<std::size_t, 0>>,
                       std::pair<int, std::integral_constant<std::size_t, 1>>,
                       std::pair<std::string_view,
                                 std::integral_constant<std::size_t, 2>>>,
            std::tuple<float, int, std::string_view>,
            std::tuple<std::integral_constant<std::size_t, 0>,
                       std::integral_constant<std::size_t, 1>,
                       std::integral_constant<std::size_t, 2>>>>;

    test::data_set(f, data_set{});
}

BOOST_AUTO_TEST_CASE(remove_bit_test)
{
    static_assert(algorithm::remove_bit<4>(0b1111u) == 0b1111u, "Fail");
    static_assert(algorithm::remove_bit<4>(0b11111u) == 0b1111u, "Fail");
    static_assert(algorithm::remove_bit<2>(0b100000u) == 0b10000u, "Fail");
    static_assert(algorithm::remove_bit<2>(0b100010u) == 0b10010u, "Fail");
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(zip_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/algorithm.hpp"
int main() {
    using tuple_a = std::tuple<int, float, double>;
    using tuple_b = std::tuple<double, int>;
    using my_zip = arg_router::algorithm::zip_t<tuple_a, tuple_b>;
    return 0;
}
    )",
        "First and Second tuples must contain the same number of elements");
}

BOOST_AUTO_TEST_CASE(not_integral_remove_bit_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/algorithm.hpp"
int main() {
    return arg_router::algorithm::remove_bit<4>(3.14);
}
    )",
        "T must be an unsigned integral");
}

BOOST_AUTO_TEST_CASE(not_unsigned_remove_bit_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/algorithm.hpp"
int main() {
    return arg_router::algorithm::remove_bit<4>(0b11111);
}
    )",
        "T must be an unsigned integral");
}

BOOST_AUTO_TEST_CASE(n_too_large_remove_bit_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/algorithm.hpp"
int main() {
    return arg_router::algorithm::remove_bit<8, std::uint8_t>(0b11111);
}
    )",
        "N is larger than the number of bit available");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
