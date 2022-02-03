/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(compile_time_string_suite)

BOOST_AUTO_TEST_CASE(declaration_test)
{
    using hello_str = utility::compile_time_string<'h', 'e', 'l', 'l', 'o'>;
    static_assert(hello_str::get().size() == 5);
    static_assert(hello_str::size() == 5);
    static_assert(!hello_str::empty());
    BOOST_CHECK_EQUAL(hello_str::get(), "hello");
}

BOOST_AUTO_TEST_CASE(macro_test)
{
    using hello_str = utility::compile_time_string<'h', 'e', 'l', 'l', 'o'>;
    using macro_str = S_("hello");

    static_assert(std::is_same_v<hello_str, macro_str>);
    static_assert(macro_str::get().size() == 5);
    static_assert(hello_str::size() == 5);
    static_assert(!hello_str::empty());
    BOOST_CHECK_EQUAL(macro_str::get(), "hello");
}

BOOST_AUTO_TEST_CASE(define_test)
{
#define MY_STR "hello"

    using hello_str = utility::compile_time_string<'h', 'e', 'l', 'l', 'o'>;
    using macro_str = S_(MY_STR);

    static_assert(std::is_same_v<hello_str, macro_str>);
    static_assert(macro_str::get().size() == 5);
    static_assert(hello_str::size() == 5);
    static_assert(!hello_str::empty());
    BOOST_CHECK_EQUAL(macro_str::get(), "hello");

#undef MY_STR
}

BOOST_AUTO_TEST_CASE(empty_test)
{
    using empty_str = utility::compile_time_string<>;
    using macro_str = S_("");

    static_assert(std::is_same_v<empty_str, macro_str>);
    static_assert(macro_str::get().size() == 0);
    static_assert(macro_str::size() == 0);
    static_assert(macro_str::empty());
    BOOST_CHECK_EQUAL(macro_str::get(), "");
}

BOOST_AUTO_TEST_CASE(append_string_type_test)
{
    using str1 = S_("hello ");
    using str2 = S_("world");
    using str3 = S_("");

    using appended = str1::append_t<str2::append_t<str3>>;

    static_assert(std::is_same_v<appended, S_("hello world")>);
}

BOOST_AUTO_TEST_CASE(append_string_operator_test)
{
    constexpr auto str1 = S_("hello "){};
    constexpr auto str2 = S_("world"){};
    constexpr auto str3 = S_(""){};

    using appended = decltype(str1 + str2 + str3);

    static_assert(std::is_same_v<appended, S_("hello world")>);
}

BOOST_AUTO_TEST_CASE(convert_integral_to_cts_test)
{
    {
        using value = utility::convert_integral_to_cts_t<0>;
        static_assert(std::is_same_v<value, S_("0")>);
    }

    {
        using value = utility::convert_integral_to_cts_t<42>;
        static_assert(std::is_same_v<value, S_("42")>);
    }

    {
        using value = utility::convert_integral_to_cts_t<2345324>;
        static_assert(std::is_same_v<value, S_("2345324")>);
    }

    {
        using value = utility::convert_integral_to_cts_t<-5>;
        static_assert(std::is_same_v<value, S_("-5")>);
    }

    {
        using value = utility::convert_integral_to_cts_t<-0>;
        static_assert(std::is_same_v<value, S_("0")>);
    }

    {
        using value = utility::convert_integral_to_cts_t<-34534>;
        static_assert(std::is_same_v<value, S_("-34534")>);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
