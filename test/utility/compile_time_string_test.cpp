#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(compile_time_string_suite)

BOOST_AUTO_TEST_CASE(declaration_test)
{
    using hello_str = utility::compile_time_string<'h', 'e', 'l', 'l', 'o'>;
    BOOST_CHECK_EQUAL(hello_str::get().size(), 5);
    BOOST_CHECK_EQUAL(hello_str::get(), "hello");
}

BOOST_AUTO_TEST_CASE(macro_test)
{
    using hello_str = utility::compile_time_string<'h', 'e', 'l', 'l', 'o'>;
    using macro_str = S_("hello");

    BOOST_CHECK((std::is_same_v<hello_str, macro_str>));
    BOOST_CHECK_EQUAL(macro_str::get().size(), 5);
    BOOST_CHECK_EQUAL(macro_str::get(), "hello");
}

BOOST_AUTO_TEST_CASE(empty_test)
{
    using empty_str = utility::compile_time_string<>;
    using macro_str = S_("");

    BOOST_CHECK((std::is_same_v<empty_str, macro_str>));
    BOOST_CHECK_EQUAL(macro_str::get().size(), 0);
    BOOST_CHECK_EQUAL(macro_str::get(), "");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
