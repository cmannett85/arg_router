#include "arg_router/math.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(math_suite)

BOOST_AUTO_TEST_CASE(abs_test)
{
    static_assert(math::abs(0) == 0);
    static_assert(math::abs(42) == 42);
    static_assert(math::abs(-0) == 0);
    static_assert(math::abs(-42) == 42);
}

BOOST_AUTO_TEST_CASE(num_digits)
{
    static_assert(math::num_digits(0) == 1);
    static_assert(math::num_digits(42) == 2);
    static_assert(math::num_digits(100) == 3);
    static_assert(math::num_digits(4345432) == 7);
    static_assert(math::num_digits(-0) == 1);
    static_assert(math::num_digits(-42) == 2);
}

BOOST_AUTO_TEST_CASE(pow)
{
    static_assert(math::pow<3>(0) == 1);
    static_assert(math::pow<3>(3) == 27);
    static_assert(math::pow<10>(3) == 1000);
    static_assert(math::pow<10>(-3) == 1);
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(abs_must_be_integral_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/math.hpp"

using namespace arg_router;

int main() {
    const auto result = math::abs("hello");
    return 0;
}
    )",
        "T must be an integral");
}

BOOST_AUTO_TEST_CASE(num_digits_must_be_integral_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/math.hpp"

using namespace arg_router;

int main() {
    const auto result = math::num_digits("hello");
    return 0;
}
    )",
        "T must be an integral");
}

BOOST_AUTO_TEST_CASE(pow_must_be_integral_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/math.hpp"

using namespace arg_router;

int main() {
    const auto result = math::pow<10>("hello");
    return 0;
}
    )",
        "T must be an integral");
}

BOOST_AUTO_TEST_CASE(pow_base_must_be_positive_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/math.hpp"

using namespace arg_router;

int main() {
    const auto result = math::pow<-10>(3);
    return 0;
}
    )",
        "Base must be greater than zero");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
