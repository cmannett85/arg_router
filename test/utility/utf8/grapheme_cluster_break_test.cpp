/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/utility/utf8/grapheme_cluster_break.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace utility;
using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(utf8_suite)

BOOST_AUTO_TEST_SUITE(no_break_rules_suite)

BOOST_AUTO_TEST_CASE(GB3_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB3(std::array{utf8::grapheme_cluster_break_class::CR},
                                      utf8::grapheme_cluster_break_class::LF);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB3(std::array{utf8::grapheme_cluster_break_class::LF},
                                      utf8::grapheme_cluster_break_class::LF);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB3(std::array{utf8::grapheme_cluster_break_class::CR,
                                                 utf8::grapheme_cluster_break_class::LF},
                                      utf8::grapheme_cluster_break_class::LF);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB3(std::array{utf8::grapheme_cluster_break_class::any},
                                      utf8::grapheme_cluster_break_class::LF);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB3(std::array{utf8::grapheme_cluster_break_class::LF},
                                      utf8::grapheme_cluster_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::GB3(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::CR},
                                  utf8::grapheme_cluster_break_class::LF,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::LF},
                                  utf8::grapheme_cluster_break_class::LF,
                                  false},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::CR,
                                             utf8::grapheme_cluster_break_class::LF},
                                  utf8::grapheme_cluster_break_class::LF,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::any},
                                  utf8::grapheme_cluster_break_class::LF,
                                  false},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::LF},
                                  utf8::grapheme_cluster_break_class::any,
                                  false},
                   });
}

BOOST_AUTO_TEST_CASE(GB6_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_class::L},
                                      utf8::grapheme_cluster_break_class::L);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_class::L},
                                      utf8::grapheme_cluster_break_class::V);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_class::L},
                                      utf8::grapheme_cluster_break_class::LV);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_class::L},
                                      utf8::grapheme_cluster_break_class::LVT);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_class::any},
                                      utf8::grapheme_cluster_break_class::LVT);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_class::L},
                                      utf8::grapheme_cluster_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_class::V},
                                      utf8::grapheme_cluster_break_class::L);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::GB6(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::L},
                                  utf8::grapheme_cluster_break_class::L,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::L},
                                  utf8::grapheme_cluster_break_class::V,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::L},
                                  utf8::grapheme_cluster_break_class::LV,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::L},
                                  utf8::grapheme_cluster_break_class::LVT,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::any},
                                  utf8::grapheme_cluster_break_class::LVT,
                                  false},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::L},
                                  utf8::grapheme_cluster_break_class::any,
                                  false},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::V},
                                  utf8::grapheme_cluster_break_class::L,
                                  false},
                   });
}

BOOST_AUTO_TEST_CASE(GB7_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_class::LV},
                                      utf8::grapheme_cluster_break_class::V);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_class::LV},
                                      utf8::grapheme_cluster_break_class::T);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_class::V},
                                      utf8::grapheme_cluster_break_class::V);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_class::V},
                                      utf8::grapheme_cluster_break_class::T);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_class::LVT},
                                      utf8::grapheme_cluster_break_class::V);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_class::L},
                                      utf8::grapheme_cluster_break_class::T);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_class::LV},
                                      utf8::grapheme_cluster_break_class::regional_indicator);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_class::V},
                                      utf8::grapheme_cluster_break_class::regional_indicator);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::GB7(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::LV},
                                  utf8::grapheme_cluster_break_class::V,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::LV},
                                  utf8::grapheme_cluster_break_class::T,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::V},
                                  utf8::grapheme_cluster_break_class::V,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::V},
                                  utf8::grapheme_cluster_break_class::T,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::LVT},
                                  utf8::grapheme_cluster_break_class::V,
                                  false},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::L},
                                  utf8::grapheme_cluster_break_class::T,
                                  false},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::LV},
                                  utf8::grapheme_cluster_break_class::regional_indicator,
                                  false},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::V},
                                  utf8::grapheme_cluster_break_class::regional_indicator,
                                  false},
                   });
}

BOOST_AUTO_TEST_CASE(GB8_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB8(std::array{utf8::grapheme_cluster_break_class::LVT},
                                      utf8::grapheme_cluster_break_class::T);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB8(std::array{utf8::grapheme_cluster_break_class::T},
                                      utf8::grapheme_cluster_break_class::T);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB8(std::array{utf8::grapheme_cluster_break_class::prepend},
                                      utf8::grapheme_cluster_break_class::T);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB8(std::array{utf8::grapheme_cluster_break_class::LVT},
                                      utf8::grapheme_cluster_break_class::prepend);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::GB8(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::LVT},
                                  utf8::grapheme_cluster_break_class::T,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::T},
                                  utf8::grapheme_cluster_break_class::T,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::prepend},
                                  utf8::grapheme_cluster_break_class::T,
                                  false},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::LVT},
                                  utf8::grapheme_cluster_break_class::prepend,
                                  false},
                   });
}

BOOST_AUTO_TEST_CASE(GB9_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB9(std::array{utf8::grapheme_cluster_break_class::any},
                                      utf8::grapheme_cluster_break_class::ZWJ);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB9(std::array{utf8::grapheme_cluster_break_class::any},
                                      utf8::grapheme_cluster_break_class::extend);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB9(std::array{utf8::grapheme_cluster_break_class::any},
                                      utf8::grapheme_cluster_break_class::regional_indicator);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB9(std::array{utf8::grapheme_cluster_break_class::extend},
                                      utf8::grapheme_cluster_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::GB9(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::any},
                                  utf8::grapheme_cluster_break_class::ZWJ,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::any},
                                  utf8::grapheme_cluster_break_class::extend,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::any},
                                  utf8::grapheme_cluster_break_class::regional_indicator,
                                  false},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::extend},
                                  utf8::grapheme_cluster_break_class::any,
                                  false},
                   });
}

BOOST_AUTO_TEST_CASE(GB9a_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB9a(std::array{utf8::grapheme_cluster_break_class::any},
                                       utf8::grapheme_cluster_break_class::spacing_mark);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB9a(std::array{utf8::grapheme_cluster_break_class::any},
                                       utf8::grapheme_cluster_break_class::regional_indicator);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB9a(std::array{utf8::grapheme_cluster_break_class::spacing_mark},
                                       utf8::grapheme_cluster_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::GB9a(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::any},
                                  utf8::grapheme_cluster_break_class::spacing_mark,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::any},
                                  utf8::grapheme_cluster_break_class::regional_indicator,
                                  false},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::spacing_mark},
                                  utf8::grapheme_cluster_break_class::any,
                                  false},
                   });
}

BOOST_AUTO_TEST_CASE(GB9b_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB9b(std::array{utf8::grapheme_cluster_break_class::prepend},
                                       utf8::grapheme_cluster_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB9b(std::array{utf8::grapheme_cluster_break_class::any},
                                       utf8::grapheme_cluster_break_class::prepend);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::GB9b(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::prepend},
                                  utf8::grapheme_cluster_break_class::any,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::any},
                                  utf8::grapheme_cluster_break_class::prepend,
                                  false},
                   });
}

BOOST_AUTO_TEST_CASE(GB11_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB11(std::array{utf8::grapheme_cluster_break_class::ZWJ},
                                       utf8::grapheme_cluster_break_class::extended_pictographic);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB11(std::array{utf8::grapheme_cluster_break_class::any},
                                       utf8::grapheme_cluster_break_class::extended_pictographic);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB11(std::array{utf8::grapheme_cluster_break_class::ZWJ},
                                       utf8::grapheme_cluster_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::GB11(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::ZWJ},
                                  utf8::grapheme_cluster_break_class::extended_pictographic,
                                  true},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::any},
                                  utf8::grapheme_cluster_break_class::extended_pictographic,
                                  false},
                       std::tuple{std::array{utf8::grapheme_cluster_break_class::ZWJ},
                                  utf8::grapheme_cluster_break_class::any,
                                  false},
                   });
}

BOOST_AUTO_TEST_CASE(GB12_13_test)
{
    {
        constexpr auto result = utf8::no_break_rules::GB12_13(
            std::array{utf8::grapheme_cluster_break_class::regional_indicator,
                       utf8::grapheme_cluster_break_class::any},
            utf8::grapheme_cluster_break_class::regional_indicator);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::GB12_13(
            std::array{utf8::grapheme_cluster_break_class::regional_indicator,
                       utf8::grapheme_cluster_break_class::regional_indicator,
                       utf8::grapheme_cluster_break_class::ZWJ},
            utf8::grapheme_cluster_break_class::regional_indicator);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::GB12_13(
            std::array{utf8::grapheme_cluster_break_class::regional_indicator,
                       utf8::grapheme_cluster_break_class::regional_indicator,
                       utf8::grapheme_cluster_break_class::regional_indicator},
            utf8::grapheme_cluster_break_class::regional_indicator);
        static_assert(result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::GB12_13(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::grapheme_cluster_break_class::regional_indicator,
                                  utf8::grapheme_cluster_break_class::any},
                       utf8::grapheme_cluster_break_class::regional_indicator,
                       true},
            std::tuple{std::array{utf8::grapheme_cluster_break_class::regional_indicator,
                                  utf8::grapheme_cluster_break_class::regional_indicator,
                                  utf8::grapheme_cluster_break_class::ZWJ},
                       utf8::grapheme_cluster_break_class::regional_indicator,
                       false},
            std::tuple{std::array{utf8::grapheme_cluster_break_class::regional_indicator,
                                  utf8::grapheme_cluster_break_class::regional_indicator,
                                  utf8::grapheme_cluster_break_class::regional_indicator},
                       utf8::grapheme_cluster_break_class::regional_indicator,
                       true},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(GB_1_element_trailing_window_tests)
{
    auto f = [](auto test_name) {
        test::death_test_compile("#include \"arg_router/utility/utf8/grapheme_cluster_break.hpp\"\n"
                                 "using namespace arg_router;"
                                 "int main() {"
                                 "const auto no_break = utility::utf8::no_break_rules::"s +
                                     test_name +
                                     "("
                                     "std::array<utility::utf8::grapheme_cluster_break_class, 0>{},"
                                     "utility::utf8::grapheme_cluster_break_class::CR);"
                                     "return 0;"
                                     "}",
                                 "Trailing window must be at least 1 element");
    };

    test::data_set(f,
                   {std::tuple{"GB3"},
                    std::tuple{"GB6"},
                    std::tuple{"GB7"},
                    std::tuple{"GB8"},
                    std::tuple{"GB9b"},
                    std::tuple{"GB11"},
                    std::tuple{"GB12_13"}});
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
