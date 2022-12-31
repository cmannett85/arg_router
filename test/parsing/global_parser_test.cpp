// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/parsing/global_parser.hpp"

#include "test_helpers.hpp"
#include "test_printers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(global_parser_suite)

BOOST_AUTO_TEST_CASE(numeric_parse_test)
{
    auto f = [](auto input, auto expected, auto ec) {
        using T = std::decay_t<decltype(expected)>;

        try {
            const auto result = parser<T>::parse(input);
            static_assert(std::is_same_v<std::decay_t<decltype(result)>, T>,
                          "Parse result unexpected type");
            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(result, expected);
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{"42", 42, std::optional<multi_lang_exception>{}},
            std::tuple{"+42", 42, std::optional<multi_lang_exception>{}},
            std::tuple{"-42", -42, std::optional<multi_lang_exception>{}},
            std::tuple{"3.14", 3.14, std::optional<multi_lang_exception>{}},
            std::tuple{"3.14", 3.14f, std::optional<multi_lang_exception>{}},
            std::tuple{"+3.14", 3.14f, std::optional<multi_lang_exception>{}},
            std::tuple{"-3.14", -3.14f, std::optional<multi_lang_exception>{}},
            std::tuple{"hello",
                       42,
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::failed_to_parse, {"hello"})}},
            std::tuple{"23742949",
                       std::uint8_t{0},
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::failed_to_parse, {"23742949"})}},
        });
}

BOOST_AUTO_TEST_CASE(string_view_parse_test)
{
    auto f = [](auto input, auto expected) {
        const auto result = parser<std::string_view>::parse(input);
        static_assert(std::is_same_v<std::decay_t<decltype(result)>, std::string_view>,
                      "Parse result unexpected type");
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(f,
                   {
                       std::tuple{"hello", "hello"},
                       std::tuple{"a", "a"},
                       std::tuple{"", ""},
                   });
}

BOOST_AUTO_TEST_CASE(string_parse_test)
{
    auto f = [](auto input, auto expected) {
        const auto result = parser<std::string>::parse(input);
        static_assert(std::is_same_v<std::decay_t<decltype(result)>, std::string>,
                      "Parse result unexpected type");
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(f,
                   {
                       std::tuple{"hello", "hello"},
                       std::tuple{"a", "a"},
                       std::tuple{"", ""},
                   });
}

BOOST_AUTO_TEST_CASE(bool_parse_test)
{
    auto f = [](auto input, auto expected, auto ec) {
        try {
            const auto result = parser<bool>::parse(input);
            static_assert(std::is_same_v<std::decay_t<decltype(result)>, bool>,
                          "Parse result unexpected type");
            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(result, expected);
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        {
            std::tuple{"true", true, std::optional<multi_lang_exception>{}},
            std::tuple{"yes", true, std::optional<multi_lang_exception>{}},
            std::tuple{"y", true, std::optional<multi_lang_exception>{}},
            std::tuple{"on", true, std::optional<multi_lang_exception>{}},
            std::tuple{"1", true, std::optional<multi_lang_exception>{}},
            std::tuple{"enable", true, std::optional<multi_lang_exception>{}},
            std::tuple{"false", false, std::optional<multi_lang_exception>{}},
            std::tuple{"no", false, std::optional<multi_lang_exception>{}},
            std::tuple{"n", false, std::optional<multi_lang_exception>{}},
            std::tuple{"off", false, std::optional<multi_lang_exception>{}},
            std::tuple{"0", false, std::optional<multi_lang_exception>{}},
            std::tuple{"disable", false, std::optional<multi_lang_exception>{}},
            std::tuple{"hello",
                       false,
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::failed_to_parse, {"hello"})}},
        });
}

BOOST_AUTO_TEST_CASE(container_parse_test)
{
    auto f = [](auto input, auto expected, auto ec) {
        using T = std::vector<std::decay_t<decltype(expected)>>;

        try {
            const auto result = parser<T>::parse(input);
            static_assert(std::is_same_v<std::decay_t<decltype(result)>, typename T::value_type>,
                          "Parse result unexpected type");
            BOOST_CHECK(!ec);
            BOOST_CHECK_EQUAL(result, expected);
        } catch (multi_lang_exception& e) {
            BOOST_REQUIRE(ec);
            BOOST_CHECK_EQUAL(e.ec(), ec->ec());
            BOOST_CHECK_EQUAL(e.tokens(), ec->tokens());
        }
    };

    test::data_set(
        f,
        std::tuple{
            std::tuple{"42", 42, std::optional<multi_lang_exception>{}},
            std::tuple{"true", true, std::optional<multi_lang_exception>{}},
            std::tuple{"3.14", 3.14f, std::optional<multi_lang_exception>{}},
            std::tuple{"hello", "hello"sv, std::optional<multi_lang_exception>{}},
            std::tuple{"hello",
                       false,
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::failed_to_parse, {"hello"})}},
            std::tuple{"23742949",
                       std::uint8_t{0},
                       std::optional<multi_lang_exception>{
                           test::create_exception(error_code::failed_to_parse, {"23742949"})}},
        });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(unimplemented_parse_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/parsing/global_parser.hpp"

struct my_struct{};

int main() {
    const auto v = arg_router::parser<my_struct>::parse("foo");
    return 0;
}
    )",
        "No parse function for this type, use a custom_parser policy or define "
        "a parser<T>::parse(std::string_view) specialisation");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
