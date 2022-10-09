// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/utility/utf8/code_point.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"

using namespace arg_router::utility;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(utf8_suite)

BOOST_AUTO_TEST_SUITE(code_point_suite)

BOOST_AUTO_TEST_CASE(count_test)
{
    {
        using str = S_("");
        static_assert(utf8::code_point::count(str::get()) == 0);
    }
    {
        using str = S_("hello");
        static_assert(utf8::code_point::count(str::get()) == 5);
    }
    {
        using str = S_("zß水🍌");
        static_assert(utf8::code_point::count(str::get()) == 4);
    }
    {
        using str = S_("Δàrö");
        static_assert(utf8::code_point::count(str::get()) == 4);
    }

    // That's right, code points are not the same as grapheme clusters!
    {
        using str = S_("🇦🇬");
        static_assert(utf8::code_point::count(str::get()) == 2);
    }
    {
        using str = S_("m̃");
        static_assert(utf8::code_point::count(str::get()) == 2);
    }
}

BOOST_AUTO_TEST_CASE(size_test)
{
    {
        using str = S_("");
        static_assert(utf8::code_point::size(str::get()) == 0);
    }
    {
        using str = S_("h");
        static_assert(utf8::code_point::size(str::get()) == 1);
    }
    {
        using str = S_("hello");
        static_assert(utf8::code_point::size(str::get()) == 1);
    }
    {
        using str = S_("Δ");
        static_assert(utf8::code_point::size(str::get()) == 2);
    }
    {
        using str = S_("猫");
        static_assert(utf8::code_point::size(str::get()) == 3);
    }
    {
        using str = S_("🍌");
        static_assert(utf8::code_point::size(str::get()) == 4);
    }
}

BOOST_AUTO_TEST_CASE(decode_test)
{
    {
        using str = S_("");
        static_assert(!utf8::code_point::decode(str::get()).has_value());
    }
    {
        using str = S_("g");
        static_assert(utf8::code_point::decode(str::get()).has_value());
        static_assert(*utf8::code_point::decode(str::get()) == 103);
    }
    {
        using str = S_("gh");
        static_assert(utf8::code_point::decode(str::get()).has_value());
        static_assert(*utf8::code_point::decode(str::get()) == 103);
    }
    {
        using str = S_("Δ");
        static_assert(utf8::code_point::decode(str::get()).has_value());
        static_assert(*utf8::code_point::decode(str::get()) == 916);
    }
    {
        using str = S_("Δh");
        static_assert(utf8::code_point::decode(str::get()).has_value());
        static_assert(*utf8::code_point::decode(str::get()) == 916);
    }
    {
        using str = S_("gΔ");
        static_assert(utf8::code_point::decode(str::get()).has_value());
        static_assert(*utf8::code_point::decode(str::get()) == 103);
    }
    {
        using str = S_("🙂");
        static_assert(utf8::code_point::decode(str::get()).has_value());
        static_assert(*utf8::code_point::decode(str::get()) == 128578);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
