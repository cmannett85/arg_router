// Copyright (C) 2022-2023 by Camden Mannett.
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
    using str = utility::str<"a🙂bΔ猫">;
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
        using empty_str = utility::str<"">;

        auto it = utf8::code_point::iterator{empty_str::get()};
        BOOST_CHECK(it == utf8::code_point::iterator{});
    }
}

BOOST_AUTO_TEST_CASE(iterator_test)
{
#if AR_ENABLE_UTF8_SUPPORT == 1
    using str = utility::str<"क़m̃🙂b🇦🇬Δ猫">;
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
#else
    using str = utility::str<"hello!!">;
    {
        constexpr auto it = utf8::iterator{str::get()};
        static_assert(*it == "h");
        static_assert(it != utf8::iterator{});
    }
    {
        auto it = utf8::iterator{str::get()};
        BOOST_CHECK_EQUAL(*it, "h");
        BOOST_CHECK(it != utf8::iterator{});
        ++it;
        BOOST_CHECK_EQUAL(*it, "e");
        ++it;
        BOOST_CHECK_EQUAL(*it, "l");
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

        const auto expected = std::vector{"h"sv, "e"sv, "l"sv, "l"sv, "o"sv, "!"sv, "!"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        auto result = std::vector<std::string_view>{};
        for (auto cp : utf8::iterator::range(str::get())) {
            result.push_back(cp);
        }

        const auto expected = std::vector{"h"sv, "e"sv, "l"sv, "l"sv, "o"sv, "!"sv, "!"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
#endif
    {
        using empty_str = utility::str<"">;

        auto it = utf8::iterator{empty_str::get()};
        BOOST_CHECK(it == utf8::iterator{});
    }
}

BOOST_AUTO_TEST_CASE(count_test)
{
    {
        using str = utility::str<"">;
        static_assert(utf8::count(str::get()) == 0);
    }
#if AR_ENABLE_UTF8_SUPPORT == 1
    {
        using str = utility::str<"🇦🇬">;
        static_assert(utf8::count(str::get()) == 1);
    }
    {
        using str = utility::str<"🇦🇬m̃">;
        static_assert(utf8::count(str::get()) == 2);
    }
#endif
    {
        using str = utility::str<"hello">;
        static_assert(utf8::count(str::get()) == 5);
    }
}

BOOST_AUTO_TEST_CASE(is_whitespace_test)
{
    {
        using str = utility::str<"">;
        static_assert(!utf8::is_whitespace(str::get()));
    }
    {
        using str = utility::str<"a">;
        static_assert(!utf8::is_whitespace(str::get()));
    }
    {
        using str = utility::str<" ">;
        static_assert(utf8::is_whitespace(str::get()));
    }
#if AR_ENABLE_UTF8_SUPPORT == 1
    {
        using str = utility::str<"🙂">;
        static_assert(!utf8::is_whitespace(str::get()));
    }
    {
        using str = utility::str<" ">;  // Thin space
        static_assert(utf8::is_whitespace(str::get()));
    }
#endif
}

BOOST_AUTO_TEST_CASE(contains_whitespace_test)
{
    {
        using str = utility::str<"">;
        static_assert(!utf8::contains_whitespace(str::get()));
    }
    {
        using str = utility::str<"hello">;
        static_assert(!utf8::contains_whitespace(str::get()));
    }
    {
        using str = utility::str<"zß水🍌">;
        static_assert(!utf8::contains_whitespace(str::get()));
    }
    {
        using str = utility::str<" ">;
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = utility::str<" hello">;
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = utility::str<"hello ">;
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = utility::str<"hel lo">;
        static_assert(utf8::contains_whitespace(str::get()));
    }
#if AR_ENABLE_UTF8_SUPPORT == 1
    // Thin space
    {
        using str = utility::str<" hello">;
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = utility::str<"hello ">;
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = utility::str<"hel lo">;
        static_assert(utf8::contains_whitespace(str::get()));
    }
#endif
}

BOOST_AUTO_TEST_CASE(strip_test)
{
    {
        using str = utility::str<"hello">;
        static_assert(utf8::strip(str::get()) == "hello");
    }
    {
        using str = utility::str<" hello">;
        static_assert(utf8::strip(str::get()) == "hello");
    }
    {
        using str = utility::str<"hello ">;
        static_assert(utf8::strip(str::get()) == "hello");
    }
    {
        using str = utility::str<" hello ">;
        static_assert(utf8::strip(str::get()) == "hello");
    }
    {
        using str = utility::str<" \thello ">;
        static_assert(utf8::strip(str::get()) == "hello");
    }
    {
        using str = utility::str<"hel  lo">;
        static_assert(utf8::strip(str::get()) == "hel  lo");
    }
    {
        using str = utility::str<"  hel  lo  ">;
        static_assert(utf8::strip(str::get()) == "hel  lo");
    }
}

BOOST_AUTO_TEST_CASE(terminal_width_test)
{
    {
        using str = utility::str<"">;
        static_assert(utf8::terminal_width(str::get()) == 0);
    }
    {
        using str = utility::str<"hello">;
        static_assert(utf8::terminal_width(str::get()) == 5);
    }
#if AR_ENABLE_UTF8_SUPPORT == 1
    {
        using str = utility::str<"zß水🍌">;
        static_assert(utf8::terminal_width(str::get()) == 6);
    }
    {
        using str = utility::str<"🙂">;
        static_assert(utf8::terminal_width(str::get()) == 2);
    }
    {
        using str = utility::str<"猫">;
        static_assert(utf8::terminal_width(str::get()) == 2);
    }
    {
        using str = utility::str<"🇦🇬">;
        static_assert(utf8::terminal_width(str::get()) == 2);
    }
    {
        using str = utility::str<"m̃">;
        static_assert(utf8::terminal_width(str::get()) == 1);
    }
#endif
}

BOOST_AUTO_TEST_CASE(line_iterator_test)
{
    {
        using str = utility::str<"hello world goodbye">;
        {
            constexpr auto it = utf8::line_iterator{str::get(), 13};
            static_assert(it.max_columns() == 13);
            static_assert(it == utf8::line_iterator{str::get(), 13});
            static_assert(*it == "hello world ");
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

        const auto expected = std::vector{"hello "sv, "world "sv, "goodbye"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
#if AR_ENABLE_UTF8_SUPPORT == 1
    {
        using str = utility::str<"hello 🙂 zß水🍌   goodbye">;
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
        using str = utility::str<"hello 🙂 zß水🍌   goodbye">;

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
        using str = utility::str<"">;

        constexpr auto it = utf8::line_iterator{str::get(), 11};
        static_assert(it.max_columns() == 11);
        static_assert(it == utf8::line_iterator{});
    }
    {
        using str = utility::str<"hello🙂zß水🍌 goodbye">;
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
#endif
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
