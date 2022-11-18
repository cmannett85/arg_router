// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/literals.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(compile_time_string_suite)

#ifdef ENABLE_CPP20_STRINGS
BOOST_AUTO_TEST_CASE(empty_test)
{
    using empty_str = utility::str<>;
    static_assert(empty_str::get().size() == 0);
    static_assert(empty_str::size() == 0);
    static_assert(empty_str::empty());
    static_assert(empty_str::get() == "");
}

BOOST_AUTO_TEST_CASE(char_literal_declaration_test)
{
    using hello_str = utility::str<"hello">;
    static_assert(hello_str::get().size() == 5);
    static_assert(hello_str::size() == 5);
    static_assert(!hello_str::empty());
    static_assert(hello_str::get() == "hello");
}

BOOST_AUTO_TEST_CASE(char_array_declaration_test)
{
    using hello_str = utility::str<std::array{'h', 'e', 'l', 'l', 'o'}>;
    static_assert(hello_str::get().size() == 5);
    static_assert(hello_str::size() == 5);
    static_assert(!hello_str::empty());
    static_assert(hello_str::get() == "hello");
}

BOOST_AUTO_TEST_CASE(char_span_declaration_test)
{
    constexpr auto hello_array = std::array{'h', 'e', 'l', 'l', 'o'};
    using hello_str = utility::str<std::span{hello_array}>;
    static_assert(hello_str::get().size() == 5);
    static_assert(hello_str::size() == 5);
    static_assert(!hello_str::empty());
    static_assert(hello_str::get() == "hello");
}

BOOST_AUTO_TEST_CASE(char_declaration_test)
{
    using char_str = utility::str<'a'>;
    static_assert(char_str::get().size() == 1);
    static_assert(char_str::size() == 1);
    static_assert(!char_str::empty());
    static_assert(char_str::get() == "a");
}

BOOST_AUTO_TEST_CASE(literal_declaration_test)
{
    constexpr auto hello_str = "hello"_S;
    static_assert(hello_str.get().size() == 5);
    static_assert(hello_str.size() == 5);
    static_assert(!hello_str.empty());
    static_assert(hello_str.get() == "hello");
}

BOOST_AUTO_TEST_CASE(empty_literal_declaration_test)
{
    constexpr auto empty_str = ""_S;
    static_assert(empty_str.get().size() == 0);
    static_assert(empty_str.size() == 0);
    static_assert(empty_str.empty());
    static_assert(empty_str.get() == "");
}
#else
BOOST_AUTO_TEST_CASE(empty_test)
{
    using empty_str = utility::compile_time_string<>;
    using macro_str = S_("");

    static_assert(std::is_same_v<empty_str, macro_str>);
    static_assert(macro_str::get().size() == 0);
    static_assert(macro_str::size() == 0);
    static_assert(macro_str::empty());
    static_assert(macro_str::get() == "");
}

BOOST_AUTO_TEST_CASE(declaration_test)
{
    using hello_str = utility::compile_time_string<'h', 'e', 'l', 'l', 'o'>;
    static_assert(hello_str::get().size() == 5);
    static_assert(hello_str::size() == 5);
    static_assert(!hello_str::empty());
    static_assert(hello_str::get() == "hello");
}

BOOST_AUTO_TEST_CASE(macro_test)
{
    using hello_str = utility::compile_time_string<'h', 'e', 'l', 'l', 'o'>;
    using macro_str = S_("hello");

    static_assert(std::is_same_v<hello_str, macro_str>);
    static_assert(macro_str::get().size() == 5);
    static_assert(hello_str::size() == 5);
    static_assert(!hello_str::empty());
    static_assert(macro_str::get() == "hello");
}

BOOST_AUTO_TEST_CASE(define_test)
{
#    define MY_STR "hello"

    using hello_str = utility::compile_time_string<'h', 'e', 'l', 'l', 'o'>;
    using macro_str = S_(MY_STR);

    static_assert(std::is_same_v<hello_str, macro_str>);
    static_assert(macro_str::get().size() == 5);
    static_assert(hello_str::size() == 5);
    static_assert(!hello_str::empty());
    static_assert(macro_str::get() == "hello");

#    undef MY_STR
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/utility/compile_time_string.hpp"

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

using namespace arg_router;

#define DECL(z, n, text) a
#define REPEATER(n) BOOST_PP_REPEAT(n, DECL, _)

int main() {
    constexpr auto big_literal = BOOST_PP_STRINGIZE(REPEATER(AR_MAX_CTS_SIZE));

    using too_big = S_(big_literal);
    return 0;
}
    )",
        "Compile-time string limit reached, consider increasing AR_MAX_CTS_SIZE");
}
#endif

BOOST_AUTO_TEST_CASE(append_string_type_test)
{
    using str1 = AR_STRING("hello ");
    using str2 = AR_STRING("world");
    using str3 = AR_STRING("");

    using appended = str1::append_t<str2::append_t<str3>>;

    static_assert(std::is_same_v<appended, AR_STRING("hello world")>);
}

BOOST_AUTO_TEST_CASE(append_string_operator_test)
{
    constexpr auto str1 = AR_STRING("hello "){};
    constexpr auto str2 = AR_STRING("world"){};
    constexpr auto str3 = AR_STRING(""){};

    using appended = decltype(str1 + str2 + str3);

    static_assert(std::is_same_v<appended, AR_STRING("hello world")>);
}

BOOST_AUTO_TEST_CASE(convert_integral_to_cts_test)
{
    {
        using value = utility::convert_integral_to_cts_t<0>;
        static_assert(std::is_same_v<value, AR_STRING("0")>);
    }

    {
        using value = utility::convert_integral_to_cts_t<42>;
        static_assert(std::is_same_v<value, AR_STRING("42")>);
    }

    {
        using value = utility::convert_integral_to_cts_t<2345324>;
        static_assert(std::is_same_v<value, AR_STRING("2345324")>);
    }

    {
        using value = utility::convert_integral_to_cts_t<-5>;
        static_assert(std::is_same_v<value, AR_STRING("-5")>);
    }

    {
        using value = utility::convert_integral_to_cts_t<-0>;
        static_assert(std::is_same_v<value, AR_STRING("0")>);
    }

    {
        using value = utility::convert_integral_to_cts_t<-34534>;
        static_assert(std::is_same_v<value, AR_STRING("-34534")>);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
