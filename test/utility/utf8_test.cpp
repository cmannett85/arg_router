// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/utility/utf8.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace utility;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(utf8_suite)

BOOST_AUTO_TEST_CASE(code_point_iterator_test)
{
    using str = AR_STRING("a🙂bΔ猫");
    {
        constexpr auto it = utf8::code_point::iterator{str::get()};
        static_assert(*it == "a");
        static_assert(it != utf8::code_point::iterator{});
    }
    {
        auto it = utf8::code_point::iterator{str::get()};
        ++it;
        BOOST_CHECK_EQUAL(*it, "🙂"sv);
        BOOST_CHECK(it != utf8::code_point::iterator{});
        ++it;
        BOOST_CHECK_EQUAL(*it, "b"sv);
        BOOST_CHECK(it != utf8::code_point::iterator{});
        ++it;
        ++it;
        ++it;
        BOOST_CHECK(it == utf8::code_point::iterator{});
    }
    {
        auto result = std::vector<std::string_view>{};
        for (auto it = utf8::code_point::iterator{str::get()}; it != utf8::code_point::iterator{};
             ++it) {
            result.push_back(*it);
        }

        const auto expected = std::vector{"a"sv, "🙂"sv, "b"sv, "Δ"sv, "猫"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        auto result = std::vector<std::string_view>{};
        for (auto cp : utf8::code_point::iterator::range(str::get())) {
            result.push_back(cp);
        }

        const auto expected = std::vector{"a"sv, "🙂"sv, "b"sv, "Δ"sv, "猫"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        using empty_str = AR_STRING("");

        auto it = utf8::code_point::iterator{empty_str::get()};
        BOOST_CHECK(it == utf8::code_point::iterator{});
    }
}

BOOST_AUTO_TEST_CASE(iterator_test)
{
    using str = AR_STRING("क़m̃🙂b🇦🇬Δ猫");
    {
        constexpr auto it = utf8::iterator{str::get()};
        static_assert(*it == "क़");
        static_assert(it != utf8::iterator{});
    }
    {
        auto it = utf8::iterator{str::get()};
        BOOST_CHECK_EQUAL(*it, "क़");
        BOOST_CHECK(it != utf8::iterator{});
        ++it;
        BOOST_CHECK_EQUAL(*it, "m̃");
        ++it;
        BOOST_CHECK_EQUAL(*it, "🙂");
        ++it;
        ++it;
        ++it;
        ++it;
        ++it;
        BOOST_CHECK(it == utf8::iterator{});
    }
    {
        auto result = std::vector<std::string_view>{};
        for (auto it = utf8::iterator{str::get()}; it != utf8::iterator{}; ++it) {
            result.push_back(*it);
        }

        const auto expected = std::vector{"क़"sv, "m̃"sv, "🙂"sv, "b"sv, "🇦🇬"sv, "Δ"sv, "猫"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        auto result = std::vector<std::string_view>{};
        for (auto cp : utf8::iterator::range(str::get())) {
            result.push_back(cp);
        }

        const auto expected = std::vector{"क़"sv, "m̃"sv, "🙂"sv, "b"sv, "🇦🇬"sv, "Δ"sv, "猫"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        using empty_str = AR_STRING("");

        auto it = utf8::iterator{empty_str::get()};
        BOOST_CHECK(it == utf8::iterator{});
    }
}

BOOST_AUTO_TEST_CASE(count_test)
{
    {
        using str = AR_STRING("");
        static_assert(utf8::count(str::get()) == 0);
    }
    {
        using str = AR_STRING("🇦🇬");
        static_assert(utf8::count(str::get()) == 1);
    }
    {
        using str = AR_STRING("🇦🇬m̃");
        static_assert(utf8::count(str::get()) == 2);
    }
    {
        using str = AR_STRING("hello");
        static_assert(utf8::count(str::get()) == 5);
    }
}

BOOST_AUTO_TEST_CASE(is_whitespace_test)
{
    {
        using str = AR_STRING("");
        static_assert(!utf8::is_whitespace(str::get()));
    }
    {
        using str = AR_STRING("a");
        static_assert(!utf8::is_whitespace(str::get()));
    }
    {
        using str = AR_STRING(" ");
        static_assert(utf8::is_whitespace(str::get()));
    }
    {
        using str = AR_STRING("🙂");
        static_assert(!utf8::is_whitespace(str::get()));
    }
    {
        using str = AR_STRING(" ");  // Thin space
        static_assert(utf8::is_whitespace(str::get()));
    }
}

BOOST_AUTO_TEST_CASE(contains_whitespace_test)
{
    {
        using str = AR_STRING("");
        static_assert(!utf8::contains_whitespace(str::get()));
    }
    {
        using str = AR_STRING("hello");
        static_assert(!utf8::contains_whitespace(str::get()));
    }
    {
        using str = AR_STRING("zß水🍌");
        static_assert(!utf8::contains_whitespace(str::get()));
    }
    {
        using str = AR_STRING(" ");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = AR_STRING(" hello");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = AR_STRING("hello ");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = AR_STRING("hel lo");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    // Thin space
    {
        using str = AR_STRING(" hello");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = AR_STRING("hello ");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = AR_STRING("hel lo");
        static_assert(utf8::contains_whitespace(str::get()));
    }
}

BOOST_AUTO_TEST_CASE(terminal_width_test)
{
    {
        using str = AR_STRING("");
        static_assert(utf8::terminal_width(str::get()) == 0);
    }
    {
        using str = AR_STRING("hello");
        static_assert(utf8::terminal_width(str::get()) == 5);
    }
    {
        using str = AR_STRING("zß水🍌");
        static_assert(utf8::terminal_width(str::get()) == 6);
    }
    {
        using str = AR_STRING("🙂");
        static_assert(utf8::terminal_width(str::get()) == 2);
    }
    {
        using str = AR_STRING("猫");
        static_assert(utf8::terminal_width(str::get()) == 2);
    }
    {
        using str = AR_STRING("🇦🇬");
        static_assert(utf8::terminal_width(str::get()) == 2);
    }
    {
        using str = AR_STRING("m̃");
        static_assert(utf8::terminal_width(str::get()) == 1);
    }
}

BOOST_AUTO_TEST_CASE(line_iterator_test)
{
    {
        using str = AR_STRING("hello 🙂 zß水🍌   goodbye");
        {
            constexpr auto it = utf8::line_iterator{str::get(), 21};
            static_assert(it.max_columns() == 21);
            static_assert(it == utf8::line_iterator{str::get(), 21});
            static_assert(*it == "hello 🙂 zß水🍌   ");
            static_assert(it != utf8::line_iterator{});
        }
        {
            constexpr auto it = utf8::line_iterator{str::get(), 0};
            static_assert(it.max_columns() == 0);
            static_assert(it == utf8::line_iterator{});
        }

        auto result = std::vector<std::string_view>{};
        auto it = utf8::line_iterator{str::get(), 7};
        BOOST_CHECK_EQUAL(it.max_columns(), 7);
        for (; it != utf8::line_iterator{}; ++it) {
            result.push_back(*it);
        }
        BOOST_CHECK(it == utf8::line_iterator{});

        const auto expected = std::vector{"hello "sv, "🙂 zß水"sv, "🍌   "sv, "goodbye"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        using str = AR_STRING("hello 🙂 zß水🍌   goodbye");

        {
            constexpr auto it = utf8::line_iterator{str::get(), 80};
            static_assert(it.max_columns() == 80);
            static_assert(*it == "hello 🙂 zß水🍌   goodbye");
        }

        auto result = std::vector<std::string_view>{};
        auto it = utf8::line_iterator{str::get(), 80};
        BOOST_CHECK_EQUAL(it.max_columns(), 80);
        for (; it != utf8::line_iterator{}; ++it) {
            result.push_back(*it);
        }
        BOOST_CHECK(it == utf8::line_iterator{});

        const auto expected = std::vector{"hello 🙂 zß水🍌   goodbye"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        using str = AR_STRING("");

        constexpr auto it = utf8::line_iterator{str::get(), 11};
        static_assert(it.max_columns() == 11);
        static_assert(it == utf8::line_iterator{});
    }
    {
        using str = AR_STRING("hello🙂zß水🍌 goodbye");
        {
            constexpr auto it = utf8::line_iterator{str::get(), 11};
            static_assert(it.max_columns() == 11);
            static_assert(it == utf8::line_iterator{str::get(), 11});
            static_assert(*it == "hello🙂zß水");
            static_assert(it != utf8::line_iterator{});
        }

        auto result = std::vector<std::string_view>{};
        auto it = utf8::line_iterator{str::get(), 7};
        BOOST_CHECK_EQUAL(it.max_columns(), 7);
        for (; it != utf8::line_iterator{}; ++it) {
            result.push_back(*it);
        }
        BOOST_CHECK(it == utf8::line_iterator{});

        const auto expected = std::vector{"hello🙂"sv, "zß水🍌 "sv, "goodbye"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
