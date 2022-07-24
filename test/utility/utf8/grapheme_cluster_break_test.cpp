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
            utf8::no_break_rules::GB3(std::array{utf8::grapheme_cluster_break_property::CR},
                                      utf8::grapheme_cluster_break_property::LF);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB3(std::array{utf8::grapheme_cluster_break_property::LF},
                                      utf8::grapheme_cluster_break_property::LF);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB3(std::array{utf8::grapheme_cluster_break_property::CR,
                                                 utf8::grapheme_cluster_break_property::LF},
                                      utf8::grapheme_cluster_break_property::LF);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB3(std::array{utf8::grapheme_cluster_break_property::any},
                                      utf8::grapheme_cluster_break_property::LF);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB3(std::array{utf8::grapheme_cluster_break_property::LF},
                                      utf8::grapheme_cluster_break_property::any);
        static_assert(!result, "Test fail");
    }
}

BOOST_AUTO_TEST_CASE(GB6_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_property::L},
                                      utf8::grapheme_cluster_break_property::L);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_property::L},
                                      utf8::grapheme_cluster_break_property::V);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_property::L},
                                      utf8::grapheme_cluster_break_property::LV);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_property::L},
                                      utf8::grapheme_cluster_break_property::LVT);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_property::any},
                                      utf8::grapheme_cluster_break_property::LVT);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_property::L},
                                      utf8::grapheme_cluster_break_property::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB6(std::array{utf8::grapheme_cluster_break_property::V},
                                      utf8::grapheme_cluster_break_property::L);
        static_assert(!result, "Test fail");
    }
}

BOOST_AUTO_TEST_CASE(GB7_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_property::LV},
                                      utf8::grapheme_cluster_break_property::V);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_property::LV},
                                      utf8::grapheme_cluster_break_property::T);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_property::V},
                                      utf8::grapheme_cluster_break_property::V);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_property::V},
                                      utf8::grapheme_cluster_break_property::T);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_property::LVT},
                                      utf8::grapheme_cluster_break_property::V);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_property::L},
                                      utf8::grapheme_cluster_break_property::T);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_property::LV},
                                      utf8::grapheme_cluster_break_property::regional_indicator);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB7(std::array{utf8::grapheme_cluster_break_property::V},
                                      utf8::grapheme_cluster_break_property::regional_indicator);
        static_assert(!result, "Test fail");
    }
}

BOOST_AUTO_TEST_CASE(GB8_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB8(std::array{utf8::grapheme_cluster_break_property::LVT},
                                      utf8::grapheme_cluster_break_property::T);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB8(std::array{utf8::grapheme_cluster_break_property::T},
                                      utf8::grapheme_cluster_break_property::T);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB8(std::array{utf8::grapheme_cluster_break_property::prepend},
                                      utf8::grapheme_cluster_break_property::T);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB8(std::array{utf8::grapheme_cluster_break_property::LVT},
                                      utf8::grapheme_cluster_break_property::prepend);
        static_assert(!result, "Test fail");
    }
}

BOOST_AUTO_TEST_CASE(GB9_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB9(std::array{utf8::grapheme_cluster_break_property::any},
                                      utf8::grapheme_cluster_break_property::ZWJ);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB9(std::array{utf8::grapheme_cluster_break_property::any},
                                      utf8::grapheme_cluster_break_property::extend);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB9(std::array{utf8::grapheme_cluster_break_property::any},
                                      utf8::grapheme_cluster_break_property::regional_indicator);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB9(std::array{utf8::grapheme_cluster_break_property::extend},
                                      utf8::grapheme_cluster_break_property::any);
        static_assert(!result, "Test fail");
    }
}

BOOST_AUTO_TEST_CASE(GB9a_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB9a(std::array{utf8::grapheme_cluster_break_property::any},
                                       utf8::grapheme_cluster_break_property::spacing_mark);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB9a(std::array{utf8::grapheme_cluster_break_property::any},
                                       utf8::grapheme_cluster_break_property::regional_indicator);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::GB9a(
            std::array{utf8::grapheme_cluster_break_property::spacing_mark},
            utf8::grapheme_cluster_break_property::any);
        static_assert(!result, "Test fail");
    }
}

BOOST_AUTO_TEST_CASE(GB9b_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::GB9b(std::array{utf8::grapheme_cluster_break_property::prepend},
                                       utf8::grapheme_cluster_break_property::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB9b(std::array{utf8::grapheme_cluster_break_property::any},
                                       utf8::grapheme_cluster_break_property::prepend);
        static_assert(!result, "Test fail");
    }
}

BOOST_AUTO_TEST_CASE(GB11_test)
{
    {
        constexpr auto result = utf8::no_break_rules::GB11(
            std::array{utf8::grapheme_cluster_break_property::ZWJ},
            utf8::grapheme_cluster_break_property::extended_pictographic);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::GB11(
            std::array{utf8::grapheme_cluster_break_property::any},
            utf8::grapheme_cluster_break_property::extended_pictographic);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::GB11(std::array{utf8::grapheme_cluster_break_property::ZWJ},
                                       utf8::grapheme_cluster_break_property::any);
        static_assert(!result, "Test fail");
    }
}

BOOST_AUTO_TEST_CASE(GB12_13_test)
{
    {
        constexpr auto result = utf8::no_break_rules::GB12_13(
            std::array{utf8::grapheme_cluster_break_property::regional_indicator,
                       utf8::grapheme_cluster_break_property::any},
            utf8::grapheme_cluster_break_property::regional_indicator);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::GB12_13(
            std::array{utf8::grapheme_cluster_break_property::regional_indicator,
                       utf8::grapheme_cluster_break_property::regional_indicator,
                       utf8::grapheme_cluster_break_property::ZWJ},
            utf8::grapheme_cluster_break_property::regional_indicator);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::GB12_13(
            std::array{utf8::grapheme_cluster_break_property::regional_indicator,
                       utf8::grapheme_cluster_break_property::regional_indicator,
                       utf8::grapheme_cluster_break_property::regional_indicator},
            utf8::grapheme_cluster_break_property::regional_indicator);
        static_assert(result, "Test fail");
    }
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(GB_tests)
{
    auto f = [](auto test_name) {
        test::death_test_compile(
            "#include \"arg_router/utility/utf8/grapheme_cluster_break.hpp\"\n"
            "using namespace arg_router;"
            "int main() {"
            "const auto no_break = utility::utf8::no_break_rules::"s +
                test_name +
                "("
                "std::array<utility::utf8::grapheme_cluster_break_property, 0>{},"
                "utility::utf8::grapheme_cluster_break_property::CR);"
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
