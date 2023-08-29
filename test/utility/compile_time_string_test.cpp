// Copyright (C) 2022-2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/literals.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace arg_router::literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(compile_time_string_suite)

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

BOOST_AUTO_TEST_CASE(append_string_type_test)
{
    using str1 = str<"hello ">;
    using str2 = str<"world">;
    using str3 = str<"">;

    using appended = str1::append_t<str2::append_t<str3>>;

    static_assert(std::is_same_v<appended, str<"hello world">>);
}

BOOST_AUTO_TEST_CASE(append_string_operator_test)
{
    constexpr auto str1 = "hello "_S;
    constexpr auto str2 = "world"_S;
    constexpr auto str3 = ""_S;

    using appended = decltype(str1 + str2 + str3);

    static_assert(std::is_same_v<appended, str<"hello world">>);
}

BOOST_AUTO_TEST_CASE(convert_integral_to_cts_test)
{
    {
        using value = utility::convert_integral_to_cts_t<0>;
        static_assert(std::is_same_v<value, str<"0">>);
    }

    {
        using value = utility::convert_integral_to_cts_t<42>;
        static_assert(std::is_same_v<value, str<"42">>);
    }

    {
        using value = utility::convert_integral_to_cts_t<2345324>;
        static_assert(std::is_same_v<value, str<"2345324">>);
    }

    {
        using value = utility::convert_integral_to_cts_t<-5>;
        static_assert(std::is_same_v<value, str<"-5">>);
    }

    {
        using value = utility::convert_integral_to_cts_t<-0>;
        static_assert(std::is_same_v<value, str<"0">>);
    }

    {
        using value = utility::convert_integral_to_cts_t<-34534>;
        static_assert(std::is_same_v<value, str<"-34534">>);
    }
}

BOOST_AUTO_TEST_CASE(is_compile_time_string_like_test)
{
    static_assert(traits::is_compile_time_string_like_v<str<"hello">>);
    static_assert(traits::is_compile_time_string_like_v<str<"">>);
    static_assert(!traits::is_compile_time_string_like_v<int>);
    static_assert(!traits::is_compile_time_string_like_v<std::vector<int>>);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
