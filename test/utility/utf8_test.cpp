/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/utility/utf8.hpp"
#include "arg_router/utility/compile_time_string.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace utility;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(utf8_suite)

BOOST_AUTO_TEST_CASE(num_code_points_test)
{
    {
        using str = S_("");
        static_assert(utf8::num_code_points(str::get()) == 0);
    }
    {
        using str = S_("hello");
        static_assert(utf8::num_code_points(str::get()) == 5);
    }
    {
        using str = S_("zß水🍌");
        static_assert(utf8::num_code_points(str::get()) == 4);
    }
    {
        using str = S_("Δàrö");
        static_assert(utf8::num_code_points(str::get()) == 4);
    }

    // That's right, code points are not the same as grapheme clusters!
    {
        using str = S_("🇦🇬");
        static_assert(utf8::num_code_points(str::get()) == 2);
    }
    {
        using str = S_("m̃");
        static_assert(utf8::num_code_points(str::get()) == 2);
    }
    {
        using str = S_("क़");
        static_assert(utf8::num_code_points(str::get()) == 2);
    }
}

BOOST_AUTO_TEST_CASE(code_point_size_test)
{
    {
        using str = S_("");
        static_assert(utf8::code_point_size(str::get()) == 0);
    }
    {
        using str = S_("h");
        static_assert(utf8::code_point_size(str::get()) == 1);
    }
    {
        using str = S_("hello");
        static_assert(utf8::code_point_size(str::get()) == 1);
    }
    {
        using str = S_("Δ");
        static_assert(utf8::code_point_size(str::get()) == 2);
    }
    {
        using str = S_("猫");
        static_assert(utf8::code_point_size(str::get()) == 3);
    }
    {
        using str = S_("🍌");
        static_assert(utf8::code_point_size(str::get()) == 4);
    }
}

BOOST_AUTO_TEST_CASE(code_point_to_number_test)
{
    {
        using str = S_("");
        static_assert(!utf8::code_point_to_number(str::get()).has_value());
    }
    {
        using str = S_("g");
        static_assert(utf8::code_point_to_number(str::get()).has_value());
        static_assert(*utf8::code_point_to_number(str::get()) == 103);
    }
    {
        using str = S_("gh");
        static_assert(utf8::code_point_to_number(str::get()).has_value());
        static_assert(*utf8::code_point_to_number(str::get()) == 103);
    }
    {
        using str = S_("Δ");
        static_assert(utf8::code_point_to_number(str::get()).has_value());
        static_assert(*utf8::code_point_to_number(str::get()) == 916);
    }
    {
        using str = S_("Δh");
        static_assert(utf8::code_point_to_number(str::get()).has_value());
        static_assert(*utf8::code_point_to_number(str::get()) == 916);
    }
    {
        using str = S_("gΔ");
        static_assert(utf8::code_point_to_number(str::get()).has_value());
        static_assert(*utf8::code_point_to_number(str::get()) == 103);
    }
    {
        using str = S_("🙂");
        static_assert(utf8::code_point_to_number(str::get()).has_value());
        static_assert(*utf8::code_point_to_number(str::get()) == 128578);
    }
}

BOOST_AUTO_TEST_CASE(code_point_iterator_test)
{
    using str = S_("a🙂bΔ猫");
    {
        constexpr auto it = utf8::code_point_iterator{str::get()};
        static_assert(*it == "a");
        static_assert(it != utf8::code_point_iterator{});
    }
    {
        auto it = utf8::code_point_iterator{str::get()};
        ++it;
        BOOST_CHECK_EQUAL(*it, "🙂"sv);
        BOOST_CHECK(it != utf8::code_point_iterator{});
        ++it;
        BOOST_CHECK_EQUAL(*it, "b"sv);
        BOOST_CHECK(it != utf8::code_point_iterator{});
        ++it;
        ++it;
        ++it;
        BOOST_CHECK(it == utf8::code_point_iterator{});
    }
    {
        auto result = std::vector<std::string_view>{};
        for (auto it = utf8::code_point_iterator{str::get()}; it != utf8::code_point_iterator{};
             ++it) {
            result.push_back(*it);
        }

        const auto expected = std::vector{"a"sv, "🙂"sv, "b"sv, "Δ"sv, "猫"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        auto result = std::vector<std::string_view>{};
        for (auto cp : utf8::code_point_iterator_wrapper{str::get()}) {
            result.push_back(cp);
        }

        const auto expected = std::vector{"a"sv, "🙂"sv, "b"sv, "Δ"sv, "猫"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
}

BOOST_AUTO_TEST_CASE(code_point_index_to_byte_index_test)
{
    {
        using str = S_("Δàrö");

        static_assert(utf8::code_point_index_to_byte_index(0, str::get()) ==
                      std::optional<std::size_t>{0});
        static_assert(utf8::code_point_index_to_byte_index(1, str::get()) ==
                      std::optional<std::size_t>{2});
        static_assert(utf8::code_point_index_to_byte_index(2, str::get()) ==
                      std::optional<std::size_t>{4});
        static_assert(utf8::code_point_index_to_byte_index(3, str::get()) ==
                      std::optional<std::size_t>{5});
        static_assert(utf8::code_point_index_to_byte_index(4, str::get()) ==
                      std::optional<std::size_t>{});
        static_assert(utf8::code_point_index_to_byte_index(100, str::get()) ==
                      std::optional<std::size_t>{});
    }
    {
        using str = S_("");
        static_assert(utf8::code_point_index_to_byte_index(0, str::get()) ==
                      std::optional<std::size_t>{});
    }
}

BOOST_AUTO_TEST_CASE(is_whitespace_test)
{
    {
        using str = S_("");
        static_assert(!utf8::is_whitespace(str::get()));
    }
    {
        using str = S_("a");
        static_assert(!utf8::is_whitespace(str::get()));
    }
    {
        using str = S_(" ");
        static_assert(utf8::is_whitespace(str::get()));
    }
    {
        using str = S_("🙂");
        static_assert(!utf8::is_whitespace(str::get()));
    }
    {
        using str = S_(" ");  // Thin space
        static_assert(utf8::is_whitespace(str::get()));
    }
}

BOOST_AUTO_TEST_CASE(contains_whitespace_test)
{
    {
        using str = S_("");
        static_assert(!utf8::contains_whitespace(str::get()));
    }
    {
        using str = S_("hello");
        static_assert(!utf8::contains_whitespace(str::get()));
    }
    {
        using str = S_("zß水🍌");
        static_assert(!utf8::contains_whitespace(str::get()));
    }
    {
        using str = S_(" ");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = S_(" hello");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = S_("hello ");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = S_("hel lo");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    // Thin space
    {
        using str = S_(" hello");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = S_("hello ");
        static_assert(utf8::contains_whitespace(str::get()));
    }
    {
        using str = S_("hel lo");
        static_assert(utf8::contains_whitespace(str::get()));
    }
}

BOOST_AUTO_TEST_CASE(terminal_width_test)
{
    {
        using str = S_("");
        static_assert(utf8::terminal_width(str::get()) == 0);
    }
    {
        using str = S_("hello");
        static_assert(utf8::terminal_width(str::get()) == 5);
    }
    {
        using str = S_("zß水🍌");
        static_assert(utf8::terminal_width(str::get()) == 6);
    }
    {
        using str = S_("🙂");
        static_assert(utf8::terminal_width(str::get()) == 2);
    }
    {
        using str = S_("猫");
        static_assert(utf8::terminal_width(str::get()) == 2);
    }
    {
        using str = S_("🇦🇬");
        static_assert(utf8::terminal_width(str::get()) == 2);
    }
}

BOOST_AUTO_TEST_CASE(line_iterator_test)
{
    {
        using str = S_("hello 🙂 zß水🍌   goodbye");
        {
            constexpr auto it = utf8::line_iterator{str::get(), 11};
            static_assert(it.max_columns() == 11);
            static_assert(it == utf8::line_iterator{str::get(), 11});
            static_assert(*it == "hello 🙂 ");
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

        const auto expected = std::vector{"hello "sv, "🙂 "sv, "zß水🍌 "sv, "goodbye"sv};
        BOOST_CHECK_EQUAL(result, expected);
    }
    {
        using str = S_("hello 🙂 zß水🍌   goodbye");

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
        using str = S_("");

        constexpr auto it = utf8::line_iterator{str::get(), 11};
        static_assert(it.max_columns() == 11);
        static_assert(it == utf8::line_iterator{});
    }
    {
        using str = S_("hello🙂zß水🍌 goodbye");
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
