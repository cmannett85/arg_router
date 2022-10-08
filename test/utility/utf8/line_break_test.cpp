// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/utility/utf8/line_break.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace utility;
using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(utf8_suite)

BOOST_AUTO_TEST_SUITE(no_break_rules_suite)

BOOST_AUTO_TEST_CASE(LB6_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB6(std::array{utf8::line_break_class::any},
                                                          utf8::line_break_class::BK);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB6(std::array{utf8::line_break_class::any},
                                                          utf8::line_break_class::CR);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB6(std::array{utf8::line_break_class::any},
                                                          utf8::line_break_class::LF);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB6(std::array{utf8::line_break_class::any},
                                                          utf8::line_break_class::NL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB6(std::array{utf8::line_break_class::NL},
                                                          utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB6(std::array{utf8::line_break_class::any},
                                                          utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB6(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::BK, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::CR, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::LF, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::NL, true},
            std::tuple{std::array{utf8::line_break_class::NL}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::AL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB7_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB7(std::array{utf8::line_break_class::any},
                                                          utf8::line_break_class::SP);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB7(std::array{utf8::line_break_class::any},
                                                          utf8::line_break_class::ZW);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB7(std::array{utf8::line_break_class::ZW},
                                                          utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB7(std::array{utf8::line_break_class::any},
                                                          utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB7(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::SP, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::ZW, true},
            std::tuple{std::array{utf8::line_break_class::ZW}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::AL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB8a_9_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB8a_9(std::array{utf8::line_break_class::CM},
                                                             utf8::line_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::LB8a_9(std::array{utf8::line_break_class::ZWJ},
                                         utf8::line_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::LB8a_9(std::array{utf8::line_break_class::ZWJ},
                                         utf8::line_break_class::AL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB8a_9(std::array{utf8::line_break_class::AL},
                                                             utf8::line_break_class::CM);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB8a_9(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::CM}, utf8::line_break_class::any, true},
            std::tuple{std::array{utf8::line_break_class::ZWJ}, utf8::line_break_class::any, true},
            std::tuple{std::array{utf8::line_break_class::ZWJ}, utf8::line_break_class::AL, true},
            std::tuple{std::array{utf8::line_break_class::AL}, utf8::line_break_class::CM, false},
        });
}

BOOST_AUTO_TEST_CASE(LB11_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB11(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::WJ);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB11(std::array{utf8::line_break_class::WJ},
                                                           utf8::line_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB11(std::array{utf8::line_break_class::WJ},
                                                           utf8::line_break_class::WJ);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB11(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB11(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::WJ, true},
            std::tuple{std::array{utf8::line_break_class::WJ}, utf8::line_break_class::any, true},
            std::tuple{std::array{utf8::line_break_class::WJ}, utf8::line_break_class::WJ, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::AL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB12_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB12(std::array{utf8::line_break_class::GL},
                                                           utf8::line_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB12(std::array{utf8::line_break_class::GL},
                                                           utf8::line_break_class::AL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB12(std::array{utf8::line_break_class::GL},
                                                           utf8::line_break_class::GL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB12(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::GL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB12(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::GL}, utf8::line_break_class::any, true},
            std::tuple{std::array{utf8::line_break_class::GL}, utf8::line_break_class::AL, true},
            std::tuple{std::array{utf8::line_break_class::GL}, utf8::line_break_class::GL, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::GL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB12a_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB12a(std::array{utf8::line_break_class::any},
                                                            utf8::line_break_class::GL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB12a(std::array{utf8::line_break_class::GL},
                                                            utf8::line_break_class::GL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB12a(std::array{utf8::line_break_class::SP},
                                                            utf8::line_break_class::GL);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB12a(std::array{utf8::line_break_class::BA},
                                                            utf8::line_break_class::GL);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB12a(std::array{utf8::line_break_class::HY},
                                                            utf8::line_break_class::GL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB12a(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::GL, true},
            std::tuple{std::array{utf8::line_break_class::GL}, utf8::line_break_class::GL, true},
            std::tuple{std::array{utf8::line_break_class::SP}, utf8::line_break_class::GL, false},
            std::tuple{std::array{utf8::line_break_class::BA}, utf8::line_break_class::GL, false},
            std::tuple{std::array{utf8::line_break_class::HY}, utf8::line_break_class::GL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB13_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB13(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::CL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB13(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::CP);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB13(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::EX);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB13(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::IS);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB13(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::SY);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB13(std::array{utf8::line_break_class::CL},
                                                           utf8::line_break_class::SY);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB13(std::array{utf8::line_break_class::CL},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB13(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::CL, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::CP, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::EX, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::IS, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::SY, true},
            std::tuple{std::array{utf8::line_break_class::CL}, utf8::line_break_class::SY, true},
            std::tuple{std::array{utf8::line_break_class::CL}, utf8::line_break_class::any, false},
        });
}

BOOST_AUTO_TEST_CASE(LB14_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB14(std::array{utf8::line_break_class::OP},
                                                           utf8::line_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB14(
            std::array{utf8::line_break_class::SP, utf8::line_break_class::OP},
            utf8::line_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB14(std::array{utf8::line_break_class::SP,
                                                                      utf8::line_break_class::SP,
                                                                      utf8::line_break_class::OP},
                                                           utf8::line_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB14(std::array{utf8::line_break_class::SP,
                                                                      utf8::line_break_class::CL,
                                                                      utf8::line_break_class::OP},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB14(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::OP}, utf8::line_break_class::any, true},
            std::tuple{std::array{utf8::line_break_class::SP, utf8::line_break_class::OP},
                       utf8::line_break_class::any,
                       true},
            std::tuple{std::array{utf8::line_break_class::SP,
                                  utf8::line_break_class::SP,
                                  utf8::line_break_class::OP},
                       utf8::line_break_class::any,
                       true},
            std::tuple{std::array{utf8::line_break_class::SP,
                                  utf8::line_break_class::CL,
                                  utf8::line_break_class::OP},
                       utf8::line_break_class::any,
                       false},
        });
}

BOOST_AUTO_TEST_CASE(LB15_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB15(std::array{utf8::line_break_class::QU},
                                                           utf8::line_break_class::OP);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB15(
            std::array{utf8::line_break_class::SP, utf8::line_break_class::QU},
            utf8::line_break_class::OP);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB15(std::array{utf8::line_break_class::SP,
                                                                      utf8::line_break_class::SP,
                                                                      utf8::line_break_class::QU},
                                                           utf8::line_break_class::OP);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB15(std::array{utf8::line_break_class::SP,
                                                                      utf8::line_break_class::AL,
                                                                      utf8::line_break_class::QU},
                                                           utf8::line_break_class::OP);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB15(std::array{utf8::line_break_class::QU},
                                                           utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB15(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::QU}, utf8::line_break_class::OP, true},
            std::tuple{std::array{utf8::line_break_class::SP, utf8::line_break_class::QU},
                       utf8::line_break_class::OP,
                       true},
            std::tuple{std::array{utf8::line_break_class::SP,
                                  utf8::line_break_class::SP,
                                  utf8::line_break_class::QU},
                       utf8::line_break_class::OP,
                       true},
            std::tuple{std::array{utf8::line_break_class::SP,
                                  utf8::line_break_class::AL,
                                  utf8::line_break_class::QU},
                       utf8::line_break_class::OP,
                       false},
            std::tuple{std::array{utf8::line_break_class::QU}, utf8::line_break_class::AL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB16_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB16(std::array{utf8::line_break_class::CL},
                                                           utf8::line_break_class::NS);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB16(std::array{utf8::line_break_class::CP},
                                                           utf8::line_break_class::NS);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::LB16(std::array{utf8::line_break_class::SP,  //
                                                  utf8::line_break_class::CL},
                                       utf8::line_break_class::NS);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::LB16(std::array{utf8::line_break_class::SP,  //
                                                  utf8::line_break_class::CP},
                                       utf8::line_break_class::NS);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB16(std::array{utf8::line_break_class::SP,
                                                                      utf8::line_break_class::SP,
                                                                      utf8::line_break_class::CL},
                                                           utf8::line_break_class::NS);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB16(std::array{utf8::line_break_class::SP,
                                                                      utf8::line_break_class::SP,
                                                                      utf8::line_break_class::CP},
                                                           utf8::line_break_class::NS);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB16(std::array{utf8::line_break_class::CL},
                                                           utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB16(std::array{utf8::line_break_class::CP},
                                                           utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB16(std::array{utf8::line_break_class::SP,
                                                                      utf8::line_break_class::AL,
                                                                      utf8::line_break_class::CL},
                                                           utf8::line_break_class::NS);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB16(std::array{utf8::line_break_class::SP,
                                                                      utf8::line_break_class::AL,
                                                                      utf8::line_break_class::CP},
                                                           utf8::line_break_class::NS);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB16(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::CL}, utf8::line_break_class::NS, true},
            std::tuple{std::array{utf8::line_break_class::CP}, utf8::line_break_class::NS, true},
            std::tuple{std::array{utf8::line_break_class::SP, utf8::line_break_class::CL},
                       utf8::line_break_class::NS,
                       true},
            std::tuple{std::array{utf8::line_break_class::SP, utf8::line_break_class::CP},
                       utf8::line_break_class::NS,
                       true},
            std::tuple{std::array{utf8::line_break_class::SP,
                                  utf8::line_break_class::SP,
                                  utf8::line_break_class::CL},
                       utf8::line_break_class::NS,
                       true},
            std::tuple{std::array{utf8::line_break_class::SP,
                                  utf8::line_break_class::SP,
                                  utf8::line_break_class::CP},
                       utf8::line_break_class::NS,
                       true},
            std::tuple{std::array{utf8::line_break_class::CL}, utf8::line_break_class::AL, false},
            std::tuple{std::array{utf8::line_break_class::CP}, utf8::line_break_class::AL, false},
            std::tuple{std::array{utf8::line_break_class::SP,
                                  utf8::line_break_class::AL,
                                  utf8::line_break_class::CL},
                       utf8::line_break_class::NS,
                       false},
            std::tuple{std::array{utf8::line_break_class::SP,
                                  utf8::line_break_class::AL,
                                  utf8::line_break_class::CP},
                       utf8::line_break_class::NS,
                       false},
        });
}

BOOST_AUTO_TEST_CASE(LB17_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB17(std::array{utf8::line_break_class::B2},
                                                           utf8::line_break_class::B2);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB17(
            std::array{utf8::line_break_class::SP, utf8::line_break_class::B2},
            utf8::line_break_class::B2);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB17(std::array{utf8::line_break_class::SP,
                                                                      utf8::line_break_class::SP,
                                                                      utf8::line_break_class::B2},
                                                           utf8::line_break_class::B2);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB17(std::array{utf8::line_break_class::SP,
                                                                      utf8::line_break_class::AL,
                                                                      utf8::line_break_class::B2},
                                                           utf8::line_break_class::B2);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB17(std::array{utf8::line_break_class::B2},
                                                           utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB17(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::B2}, utf8::line_break_class::B2, true},
            std::tuple{std::array{utf8::line_break_class::SP, utf8::line_break_class::B2},
                       utf8::line_break_class::B2,
                       true},
            std::tuple{std::array{utf8::line_break_class::SP,
                                  utf8::line_break_class::SP,
                                  utf8::line_break_class::B2},
                       utf8::line_break_class::B2,
                       true},
            std::tuple{std::array{utf8::line_break_class::SP,
                                  utf8::line_break_class::AL,
                                  utf8::line_break_class::B2},
                       utf8::line_break_class::B2,
                       false},
            std::tuple{std::array{utf8::line_break_class::B2}, utf8::line_break_class::AL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB19_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB19(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::QU);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB19(std::array{utf8::line_break_class::QU},
                                                           utf8::line_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB19(std::array{utf8::line_break_class::QU},
                                                           utf8::line_break_class::QU);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB19(std::array{utf8::line_break_class::AL},
                                                           utf8::line_break_class::QU);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB19(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB19(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::QU, true},
            std::tuple{std::array{utf8::line_break_class::QU}, utf8::line_break_class::any, true},
            std::tuple{std::array{utf8::line_break_class::QU}, utf8::line_break_class::QU, true},
            std::tuple{std::array{utf8::line_break_class::AL}, utf8::line_break_class::QU, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::AL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB21_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB21(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::BA);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB21(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::HY);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB21(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::NS);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB21(std::array{utf8::line_break_class::BB},
                                                           utf8::line_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB21(std::array{utf8::line_break_class::BA},
                                                           utf8::line_break_class::BA);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB21(std::array{utf8::line_break_class::BA},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB21(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB21(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::BA, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::HY, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::NS, true},
            std::tuple{std::array{utf8::line_break_class::BB}, utf8::line_break_class::any, true},
            std::tuple{std::array{utf8::line_break_class::BA}, utf8::line_break_class::BA, true},
            std::tuple{std::array{utf8::line_break_class::BA}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::AL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB21a_test)
{
    {
        constexpr auto result =
            utf8::no_break_rules::LB21a(std::array{utf8::line_break_class::HY,  //
                                                   utf8::line_break_class::HL},
                                        utf8::line_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::LB21a(std::array{utf8::line_break_class::BA,  //
                                                   utf8::line_break_class::HL},
                                        utf8::line_break_class::any);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::LB21a(std::array{utf8::line_break_class::AL,  //
                                                   utf8::line_break_class::HL},
                                        utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result =
            utf8::no_break_rules::LB21a(std::array{utf8::line_break_class::HY,  //
                                                   utf8::line_break_class::AL},
                                        utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB21a(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::HY, utf8::line_break_class::HL},
                       utf8::line_break_class::any,
                       true},
            std::tuple{std::array{utf8::line_break_class::BA, utf8::line_break_class::HL},
                       utf8::line_break_class::any,
                       true},
            std::tuple{std::array{utf8::line_break_class::AL, utf8::line_break_class::HL},
                       utf8::line_break_class::any,
                       false},
            std::tuple{std::array{utf8::line_break_class::HY, utf8::line_break_class::AL},
                       utf8::line_break_class::any,
                       false},
        });
}

BOOST_AUTO_TEST_CASE(LB21b_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB21b(std::array{utf8::line_break_class::SY},
                                                            utf8::line_break_class::HL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB21b(std::array{utf8::line_break_class::SY},
                                                            utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB21b(std::array{utf8::line_break_class::AL},
                                                            utf8::line_break_class::HL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB21b(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::SY}, utf8::line_break_class::HL, true},
            std::tuple{std::array{utf8::line_break_class::SY}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::AL}, utf8::line_break_class::HL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB22_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB22(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::IN_);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB22(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB22(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::IN_, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::AL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB23_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB23(std::array{utf8::line_break_class::AL},
                                                           utf8::line_break_class::NU);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23(std::array{utf8::line_break_class::HL},
                                                           utf8::line_break_class::NU);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::NU);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23(std::array{utf8::line_break_class::AL},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23(std::array{utf8::line_break_class::NU},
                                                           utf8::line_break_class::AL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23(std::array{utf8::line_break_class::NU},
                                                           utf8::line_break_class::HL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23(std::array{utf8::line_break_class::NU},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23(std::array{utf8::line_break_class::NU},
                                                           utf8::line_break_class::NU);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB23(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::AL}, utf8::line_break_class::NU, true},
            std::tuple{std::array{utf8::line_break_class::HL}, utf8::line_break_class::NU, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::NU, false},
            std::tuple{std::array{utf8::line_break_class::AL}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::NU}, utf8::line_break_class::AL, true},
            std::tuple{std::array{utf8::line_break_class::NU}, utf8::line_break_class::HL, true},
            std::tuple{std::array{utf8::line_break_class::NU}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::NU}, utf8::line_break_class::NU, false},
        });
}

BOOST_AUTO_TEST_CASE(LB23a_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB23a(std::array{utf8::line_break_class::PR},
                                                            utf8::line_break_class::ID);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23a(std::array{utf8::line_break_class::PR},
                                                            utf8::line_break_class::EB);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23a(std::array{utf8::line_break_class::PR},
                                                            utf8::line_break_class::EM);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23a(std::array{utf8::line_break_class::any},
                                                            utf8::line_break_class::ID);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23a(std::array{utf8::line_break_class::PR},
                                                            utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23a(std::array{utf8::line_break_class::ID},
                                                            utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23a(std::array{utf8::line_break_class::EB},
                                                            utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23a(std::array{utf8::line_break_class::EM},
                                                            utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23a(std::array{utf8::line_break_class::any},
                                                            utf8::line_break_class::PO);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB23a(std::array{utf8::line_break_class::ID},
                                                            utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB23a(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::ID, true},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::EB, true},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::EM, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::ID, false},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::ID}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::EB}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::EM}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::PO, false},
            std::tuple{std::array{utf8::line_break_class::ID}, utf8::line_break_class::any, false},
        });
}

BOOST_AUTO_TEST_CASE(LB24_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::PR},
                                                           utf8::line_break_class::AL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::PO},
                                                           utf8::line_break_class::AL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::PR},
                                                           utf8::line_break_class::HL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::PO},
                                                           utf8::line_break_class::HL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::HL);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::PO},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::AL},
                                                           utf8::line_break_class::PR);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::HL},
                                                           utf8::line_break_class::PR);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::AL},
                                                           utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::HL},
                                                           utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::HL},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB24(std::array{utf8::line_break_class::HL},
                                                           utf8::line_break_class::HL);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB24(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::AL, true},
            std::tuple{std::array{utf8::line_break_class::PO}, utf8::line_break_class::AL, true},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::HL, true},
            std::tuple{std::array{utf8::line_break_class::PO}, utf8::line_break_class::HL, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::HL, false},
            std::tuple{std::array{utf8::line_break_class::PO}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::AL}, utf8::line_break_class::PR, true},
            std::tuple{std::array{utf8::line_break_class::HL}, utf8::line_break_class::PR, true},
            std::tuple{std::array{utf8::line_break_class::AL}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::HL}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::HL}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::HL}, utf8::line_break_class::HL, false},
        });
}

BOOST_AUTO_TEST_CASE(LB25_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::CL},
                                                           utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::CP},
                                                           utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::CL},
                                                           utf8::line_break_class::PR);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::CP},
                                                           utf8::line_break_class::PR);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::NU},
                                                           utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::NU},
                                                           utf8::line_break_class::PR);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::PO},
                                                           utf8::line_break_class::OP);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::PO},
                                                           utf8::line_break_class::NU);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::PR},
                                                           utf8::line_break_class::OP);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::PR},
                                                           utf8::line_break_class::NU);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::HY},
                                                           utf8::line_break_class::NU);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::IS},
                                                           utf8::line_break_class::NU);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::NU},
                                                           utf8::line_break_class::NU);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::SY},
                                                           utf8::line_break_class::NU);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::PO);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB25(std::array{utf8::line_break_class::SY},
                                                           utf8::line_break_class::CP);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB25(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::CL}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::CP}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::CL}, utf8::line_break_class::PR, true},
            std::tuple{std::array{utf8::line_break_class::CP}, utf8::line_break_class::PR, true},
            std::tuple{std::array{utf8::line_break_class::NU}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::NU}, utf8::line_break_class::PR, true},
            std::tuple{std::array{utf8::line_break_class::PO}, utf8::line_break_class::OP, true},
            std::tuple{std::array{utf8::line_break_class::PO}, utf8::line_break_class::NU, true},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::OP, true},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::NU, true},
            std::tuple{std::array{utf8::line_break_class::HY}, utf8::line_break_class::NU, true},
            std::tuple{std::array{utf8::line_break_class::IS}, utf8::line_break_class::NU, true},
            std::tuple{std::array{utf8::line_break_class::NU}, utf8::line_break_class::NU, true},
            std::tuple{std::array{utf8::line_break_class::SY}, utf8::line_break_class::NU, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::PO, false},
            std::tuple{std::array{utf8::line_break_class::SY}, utf8::line_break_class::CP, false},
        });
}

BOOST_AUTO_TEST_CASE(LB26_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::JL},
                                                           utf8::line_break_class::JL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::JL},
                                                           utf8::line_break_class::JV);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::JL},
                                                           utf8::line_break_class::H2);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::JL},
                                                           utf8::line_break_class::H3);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::JL},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::CP},
                                                           utf8::line_break_class::H3);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::JV},
                                                           utf8::line_break_class::JV);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::JV},
                                                           utf8::line_break_class::JT);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::H2},
                                                           utf8::line_break_class::JV);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::H2},
                                                           utf8::line_break_class::JT);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::JV},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::JT},
                                                           utf8::line_break_class::JT);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::H3},
                                                           utf8::line_break_class::JT);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB26(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::JT);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB26(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::JL}, utf8::line_break_class::JL, true},
            std::tuple{std::array{utf8::line_break_class::JL}, utf8::line_break_class::JV, true},
            std::tuple{std::array{utf8::line_break_class::JL}, utf8::line_break_class::H2, true},
            std::tuple{std::array{utf8::line_break_class::JL}, utf8::line_break_class::H3, true},
            std::tuple{std::array{utf8::line_break_class::JL}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::CP}, utf8::line_break_class::H3, false},
            std::tuple{std::array{utf8::line_break_class::JV}, utf8::line_break_class::JV, true},
            std::tuple{std::array{utf8::line_break_class::JV}, utf8::line_break_class::JT, true},
            std::tuple{std::array{utf8::line_break_class::H2}, utf8::line_break_class::JV, true},
            std::tuple{std::array{utf8::line_break_class::H2}, utf8::line_break_class::JT, true},
            std::tuple{std::array{utf8::line_break_class::JV}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::JT}, utf8::line_break_class::JT, true},
            std::tuple{std::array{utf8::line_break_class::H3}, utf8::line_break_class::JT, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::JT, false},
        });
}

BOOST_AUTO_TEST_CASE(LB27_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::JL},
                                                           utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::JV},
                                                           utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::JT},
                                                           utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::H2},
                                                           utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::H3},
                                                           utf8::line_break_class::PO);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::PO);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::JL},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::PR},
                                                           utf8::line_break_class::JL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::PR},
                                                           utf8::line_break_class::JV);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::PR},
                                                           utf8::line_break_class::JT);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::PR},
                                                           utf8::line_break_class::H2);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::PR},
                                                           utf8::line_break_class::H3);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::JL);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB27(std::array{utf8::line_break_class::PR},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB27(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::JL}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::JV}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::JT}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::H2}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::H3}, utf8::line_break_class::PO, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::PO, false},
            std::tuple{std::array{utf8::line_break_class::JL}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::JL, true},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::JV, true},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::JT, true},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::H2, true},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::H3, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::JL, false},
            std::tuple{std::array{utf8::line_break_class::PR}, utf8::line_break_class::any, false},
        });
}

BOOST_AUTO_TEST_CASE(LB28_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB28(std::array{utf8::line_break_class::AL},
                                                           utf8::line_break_class::AL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB28(std::array{utf8::line_break_class::AL},
                                                           utf8::line_break_class::HL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB28(std::array{utf8::line_break_class::HL},
                                                           utf8::line_break_class::AL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB28(std::array{utf8::line_break_class::HL},
                                                           utf8::line_break_class::HL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB28(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB28(std::array{utf8::line_break_class::AL},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB28(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::HL);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB28(std::array{utf8::line_break_class::HL},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB28(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::AL}, utf8::line_break_class::AL, true},
            std::tuple{std::array{utf8::line_break_class::AL}, utf8::line_break_class::HL, true},
            std::tuple{std::array{utf8::line_break_class::HL}, utf8::line_break_class::AL, true},
            std::tuple{std::array{utf8::line_break_class::HL}, utf8::line_break_class::HL, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::AL, false},
            std::tuple{std::array{utf8::line_break_class::AL}, utf8::line_break_class::any, false},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::HL, false},
            std::tuple{std::array{utf8::line_break_class::HL}, utf8::line_break_class::any, false},
        });
}

BOOST_AUTO_TEST_CASE(LB29_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB29(std::array{utf8::line_break_class::IS},
                                                           utf8::line_break_class::AL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB29(std::array{utf8::line_break_class::IS},
                                                           utf8::line_break_class::HL);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB29(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::AL);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB29(std::array{utf8::line_break_class::IS},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB29(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::IS}, utf8::line_break_class::AL, true},
            std::tuple{std::array{utf8::line_break_class::IS}, utf8::line_break_class::HL, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::AL, false},
            std::tuple{std::array{utf8::line_break_class::IS}, utf8::line_break_class::any, false},
        });
}

BOOST_AUTO_TEST_CASE(LB30_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB30(std::array{utf8::line_break_class::AL},
                                                           utf8::line_break_class::OP);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB30(std::array{utf8::line_break_class::HL},
                                                           utf8::line_break_class::OP);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB30(std::array{utf8::line_break_class::NU},
                                                           utf8::line_break_class::OP);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB30(std::array{utf8::line_break_class::any},
                                                           utf8::line_break_class::OP);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB30(std::array{utf8::line_break_class::NU},
                                                           utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB30(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::AL}, utf8::line_break_class::OP, true},
            std::tuple{std::array{utf8::line_break_class::HL}, utf8::line_break_class::OP, true},
            std::tuple{std::array{utf8::line_break_class::NU}, utf8::line_break_class::OP, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::OP, false},
            std::tuple{std::array{utf8::line_break_class::NU}, utf8::line_break_class::any, false},
        });
}

BOOST_AUTO_TEST_CASE(LB30a_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB30a(std::array{utf8::line_break_class::RI},
                                                            utf8::line_break_class::RI);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB30a(std::array{utf8::line_break_class::RI,
                                                                       utf8::line_break_class::RI,
                                                                       utf8::line_break_class::RI},
                                                            utf8::line_break_class::RI);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB30a(std::array{utf8::line_break_class::RI,
                                                                       utf8::line_break_class::any,
                                                                       utf8::line_break_class::RI},
                                                            utf8::line_break_class::RI);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB30a(
            std::array{utf8::line_break_class::RI, utf8::line_break_class::RI},
            utf8::line_break_class::RI);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB30a(std::array{utf8::line_break_class::any},
                                                            utf8::line_break_class::RI);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB30a(std::array{utf8::line_break_class::RI},
                                                            utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB30a(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::RI}, utf8::line_break_class::RI, true},
            std::tuple{std::array{utf8::line_break_class::RI,
                                  utf8::line_break_class::RI,
                                  utf8::line_break_class::RI},
                       utf8::line_break_class::RI,
                       true},
            std::tuple{std::array{utf8::line_break_class::RI,
                                  utf8::line_break_class::any,
                                  utf8::line_break_class::RI},
                       utf8::line_break_class::RI,
                       true},
            std::tuple{std::array{utf8::line_break_class::RI, utf8::line_break_class::RI},
                       utf8::line_break_class::RI,
                       false},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::RI, false},
            std::tuple{std::array{utf8::line_break_class::RI}, utf8::line_break_class::any, false},
        });
}

BOOST_AUTO_TEST_CASE(LB30b_test)
{
    {
        constexpr auto result = utf8::no_break_rules::LB30b(std::array{utf8::line_break_class::EB},
                                                            utf8::line_break_class::EM);
        static_assert(result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB30b(std::array{utf8::line_break_class::any},
                                                            utf8::line_break_class::EM);
        static_assert(!result, "Test fail");
    }
    {
        constexpr auto result = utf8::no_break_rules::LB30b(std::array{utf8::line_break_class::EB},
                                                            utf8::line_break_class::any);
        static_assert(!result, "Test fail");
    }

    auto f = [](auto trailing_window, auto next_class, auto expected_result) {
        const auto result = utf8::no_break_rules::LB30b(trailing_window, next_class);
        BOOST_CHECK_EQUAL(result, expected_result);
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{std::array{utf8::line_break_class::EB}, utf8::line_break_class::EM, true},
            std::tuple{std::array{utf8::line_break_class::any}, utf8::line_break_class::EM, false},
            std::tuple{std::array{utf8::line_break_class::EB}, utf8::line_break_class::any, false},
        });
}

BOOST_AUTO_TEST_CASE(line_break_death_test)
{
    auto tests = std::forward_list<test::death_test_info>{};
    for (auto test : {"LB8a_9", "LB11", "LB12",  "LB12a", "LB14",  "LB15", "LB16", "LB17",
                      "LB19",   "LB21", "LB21b", "LB23",  "LB23a", "LB24", "LB25", "LB26",
                      "LB27",   "LB28", "LB29",  "LB30",  "LB30a", "LB30b"}) {
        tests.push_front({"#include \"arg_router/utility/utf8/line_break.hpp\"\n"
                          "using namespace arg_router;"
                          "int main() {"
                          "const auto no_break = utility::utf8::no_break_rules::"s +
                              test +
                              "("
                              "std::array<utility::utf8::line_break_class, 0>{},"
                              "utility::utf8::line_break_class::AL);"
                              "return 0;"
                              "}",
                          "Trailing window must be at least 1 element",
                          test});
    }
    for (auto test : {"LB21a"}) {
        tests.push_front({"#include \"arg_router/utility/utf8/line_break.hpp\"\n"
                          "using namespace arg_router;"
                          "int main() {"
                          "const auto no_break = utility::utf8::no_break_rules::"s +
                              test +
                              "("
                              "std::array<utility::utf8::line_break_class, 1>{},"
                              "utility::utf8::line_break_class::AL);"
                              "return 0;"
                              "}",
                          "Trailing window must be at least 2 elements",
                          test});
    }

    test::death_test_compile(std::move(tests));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
