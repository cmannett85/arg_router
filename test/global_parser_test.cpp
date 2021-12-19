#include "arg_router/global_parser.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(global_parser_suite)

BOOST_AUTO_TEST_CASE(numeric_parse_test)
{
    auto f = [](auto input, auto expected, std::string_view fail_message) {
        using T = std::decay_t<decltype(expected)>;

        try {
            const auto result = parser<T>::parse(input);
            static_assert(std::is_same_v<std::decay_t<decltype(result)>, T>,
                          "Parse result unexpected type");
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{"42", 42, ""},
                       std::tuple{"+42", 42, ""},
                       std::tuple{"-42", -42, ""},
                       std::tuple{"3.14", 3.14, ""},
                       std::tuple{"3.14", 3.14f, ""},
                       std::tuple{"+3.14", 3.14f, ""},
                       std::tuple{"-3.14", -3.14f, ""},
                       std::tuple{"hello", 42, "Failed to parse: hello"},
                       std::tuple{"23742949",
                                  std::uint8_t{0},
                                  "Value out of range for argument: 23742949"},
                   });
}

BOOST_AUTO_TEST_CASE(string_view_parse_test)
{
    auto f = [](auto input, auto expected) {
        const auto result = parser<std::string_view>::parse(input);
        static_assert(
            std::is_same_v<std::decay_t<decltype(result)>, std::string_view>,
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
    auto f = [](auto input, auto expected, std::string_view fail_message) {
        try {
            const auto result = parser<bool>::parse(input);
            static_assert(std::is_same_v<std::decay_t<decltype(result)>, bool>,
                          "Parse result unexpected type");
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(f,
                   {
                       std::tuple{"true", true, ""},
                       std::tuple{"yes", true, ""},
                       std::tuple{"y", true, ""},
                       std::tuple{"on", true, ""},
                       std::tuple{"1", true, ""},
                       std::tuple{"enable", true, ""},
                       std::tuple{"false", false, ""},
                       std::tuple{"no", false, ""},
                       std::tuple{"n", false, ""},
                       std::tuple{"off", false, ""},
                       std::tuple{"0", false, ""},
                       std::tuple{"disable", false, ""},
                       std::tuple{"hello", false, "Failed to parse: hello"},
                   });
}

BOOST_AUTO_TEST_CASE(container_parse_test)
{
    auto f = [](auto input, auto expected, std::string_view fail_message) {
        using T = std::vector<std::decay_t<decltype(expected)>>;

        try {
            const auto result = parser<T>::parse(input);
            static_assert(std::is_same_v<std::decay_t<decltype(result)>,
                                         typename T::value_type>,
                          "Parse result unexpected type");
            BOOST_CHECK(fail_message.empty());
            BOOST_CHECK_EQUAL(result, expected);
        } catch (parse_exception& e) {
            BOOST_CHECK_EQUAL(e.what(), fail_message);
        }
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{"42", 42, ""},
                       std::tuple{"true", true, ""},
                       std::tuple{"3.14", 3.14f, ""},
                       std::tuple{"hello", "hello"sv, ""},
                       std::tuple{"hello", false, "Failed to parse: hello"},
                       std::tuple{"23742949",
                                  std::uint8_t{0},
                                  "Value out of range for argument: 23742949"},
                   });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(unimplemented_parse_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/global_parser.hpp"

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
