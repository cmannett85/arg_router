// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/utility/string_to_policy.hpp"
#include "arg_router/literals.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/display_name.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/min_max_value.hpp"
#include "arg_router/policy/short_name.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace arg_router::literals;

namespace
{
template <typename ExpectedTuple, typename... Mappings, typename... Params>
void test_core(Params&&... params)
{
    const auto result = utility::string_to_policy::convert<Mappings...>(std::move(params)...);

    using result_type = std::decay_t<decltype(result)>;
    static_assert(std::tuple_size_v<result_type> == std::tuple_size_v<ExpectedTuple>);

    utility::tuple_iterator(
        [&](auto i, auto&&) {
            static_assert(std::is_same_v<std::tuple_element_t<i, result_type>,
                                         std::tuple_element_t<i, ExpectedTuple>>);
        },
        result);
};
}  // namespace

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(string_to_policy_suite)

BOOST_AUTO_TEST_CASE(first_string_mapper_test)
{
    using mapper = utility::string_to_policy::first_string_mapper<policy::long_name_t>;

    using test1 = typename mapper::type<std::tuple<>>;
    static_assert(std::is_void_v<test1>);

    using test2 = typename mapper::type<std::tuple<AR_STRING("h"), AR_STRING("hello")>>;
    static_assert(std::is_same_v<test2, policy::long_name_t<AR_STRING("hello")>>);

    using test3 = typename mapper::type<std::tuple<AR_STRING("hello"), AR_STRING("h")>>;
    static_assert(std::is_same_v<test3, policy::long_name_t<AR_STRING("hello")>>);

    using test4 = typename mapper::type<std::tuple<AR_STRING("h"), AR_STRING("h")>>;
    static_assert(std::is_void_v<test4>);

    using test5 = typename mapper::type<std::tuple<AR_STRING("hello"), AR_STRING("world")>>;
    static_assert(std::is_same_v<test5, policy::long_name_t<AR_STRING("hello")>>);
}

BOOST_AUTO_TEST_CASE(second_string_mapper_test)
{
    using mapper = utility::string_to_policy::second_string_mapper<policy::long_name_t>;

    using test1 = typename mapper::type<std::tuple<>>;
    static_assert(std::is_void_v<test1>);

    using test2 = typename mapper::type<std::tuple<AR_STRING("h"), AR_STRING("hello")>>;
    static_assert(std::is_void_v<test2>);

    using test3 = typename mapper::type<std::tuple<AR_STRING("hello"), AR_STRING("h")>>;
    static_assert(std::is_void_v<test3>);

    using test4 = typename mapper::type<std::tuple<AR_STRING("h"), AR_STRING("h")>>;
    static_assert(std::is_void_v<test4>);

    using test5 = typename mapper::type<std::tuple<AR_STRING("hello"), AR_STRING("world")>>;
    static_assert(std::is_same_v<test5, policy::long_name_t<AR_STRING("world")>>);
}

BOOST_AUTO_TEST_CASE(single_char_mapper_test)
{
    using mapper = utility::string_to_policy::single_char_mapper<policy::short_name_t>;

    using test1 = typename mapper::type<std::tuple<>>;
    static_assert(std::is_void_v<test1>);

    using test2 = typename mapper::type<std::tuple<AR_STRING("h"), AR_STRING("hello")>>;
    static_assert(std::is_same_v<test2, policy::short_name_t<AR_STRING("h")>>);

    using test3 = typename mapper::type<std::tuple<AR_STRING("hello"), AR_STRING("h")>>;
    static_assert(std::is_same_v<test3, policy::short_name_t<AR_STRING("h")>>);

    using test4 = typename mapper::type<std::tuple<AR_STRING("h"), AR_STRING("h")>>;
    static_assert(std::is_same_v<test4, policy::short_name_t<AR_STRING("h")>>);

    using test5 = typename mapper::type<std::tuple<AR_STRING("hello"), AR_STRING("world")>>;
    static_assert(std::is_void_v<test5>);
}

BOOST_AUTO_TEST_CASE(first_text_mapper_test)
{
    using mapper = utility::string_to_policy::first_text_mapper<policy::display_name_t>;

    using test1 = typename mapper::type<std::tuple<>>;
    static_assert(std::is_void_v<test1>);

    using test2 = typename mapper::type<std::tuple<AR_STRING("h"), AR_STRING("hello")>>;
    static_assert(std::is_same_v<test2, policy::display_name_t<AR_STRING("h")>>);

    using test3 = typename mapper::type<std::tuple<AR_STRING("hello"), AR_STRING("h")>>;
    static_assert(std::is_same_v<test3, policy::display_name_t<AR_STRING("hello")>>);

    using test4 = typename mapper::type<std::tuple<AR_STRING("h"), AR_STRING("h")>>;
    static_assert(std::is_same_v<test4, policy::display_name_t<AR_STRING("h")>>);

    using test5 = typename mapper::type<std::tuple<AR_STRING("hello"), AR_STRING("world")>>;
    static_assert(std::is_same_v<test5, policy::display_name_t<AR_STRING("hello")>>);
}

BOOST_AUTO_TEST_CASE(second_text_mapper_test)
{
    using mapper = utility::string_to_policy::second_text_mapper<policy::display_name_t>;

    using test1 = typename mapper::type<std::tuple<>>;
    static_assert(std::is_void_v<test1>);

    using test2 = typename mapper::type<std::tuple<AR_STRING("hello")>>;
    static_assert(std::is_void_v<test2>);

    using test3 = typename mapper::type<std::tuple<AR_STRING("h"), AR_STRING("hello")>>;
    static_assert(std::is_same_v<test3, policy::display_name_t<AR_STRING("hello")>>);

    using test4 = typename mapper::type<std::tuple<AR_STRING("hello"), AR_STRING("h")>>;
    static_assert(std::is_same_v<test4, policy::display_name_t<AR_STRING("h")>>);

    using test5 = typename mapper::type<std::tuple<AR_STRING("h"), AR_STRING("h")>>;
    static_assert(std::is_same_v<test5, policy::display_name_t<AR_STRING("h")>>);

    using test6 = typename mapper::type<std::tuple<AR_STRING("hello"), AR_STRING("world")>>;
    static_assert(std::is_same_v<test6, policy::display_name_t<AR_STRING("world")>>);
}

BOOST_AUTO_TEST_CASE(convert_empty_test)
{
    const auto result = utility::string_to_policy::convert<
        utility::string_to_policy::first_string_mapper<policy::long_name_t>>();

    using result_type = std::decay_t<decltype(result)>;
    static_assert(std::tuple_size_v<result_type> == 0);
}

BOOST_AUTO_TEST_CASE(convert_first_string_test)
{
    auto f = [](auto expected, auto... params) {
        using expected_type = std::decay_t<decltype(expected)>;
        test_core<expected_type,
                  utility::string_to_policy::first_string_mapper<policy::long_name_t>>(
            std::move(params)...);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::tuple{policy::description_t{AR_STRING("hello"){}}},
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("long"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  policy::max_value<42>(),
                                  AR_STRING("long"){},
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("long"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  AR_STRING("long"){},
                                  policy::max_value<42>(),
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("long"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  policy::max_value<42>(),
                                  policy::description_t{AR_STRING("hello"){}},
                                  AR_STRING("long"){}},
                   });
}

BOOST_AUTO_TEST_CASE(convert_second_string_test)
{
    auto f = [](auto expected, auto... params) {
        using expected_type = std::decay_t<decltype(expected)>;
        test_core<expected_type,
                  utility::string_to_policy::first_string_mapper<policy::long_name_t>,
                  utility::string_to_policy::second_string_mapper<policy::description_t>>(
            std::move(params)...);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::tuple{policy::description_t{AR_STRING("hello"){}}},
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("hello"){}},
                                             policy::description_t{AR_STRING("desc"){}},
                                             policy::max_value<42>()},
                                  policy::max_value<42>(),
                                  AR_STRING("hello"){},
                                  AR_STRING("desc"){}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("hello"){}},
                                             policy::description_t{AR_STRING("desc"){}},
                                             policy::max_value<42>()},
                                  AR_STRING("hello"){},
                                  policy::max_value<42>(),
                                  AR_STRING("desc"){}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("hello"){}},
                                             policy::description_t{AR_STRING("desc"){}},
                                             policy::max_value<42>()},
                                  policy::max_value<42>(),
                                  AR_STRING("hello"){},
                                  AR_STRING("desc"){}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("hello"){}},
                                             policy::max_value<42>()},
                                  policy::max_value<42>(),
                                  AR_STRING("hello"){}},
                   });
}

BOOST_AUTO_TEST_CASE(convert_single_char_test)
{
    auto f = [](auto expected, auto... params) {
        using expected_type = std::decay_t<decltype(expected)>;
        test_core<expected_type,
                  utility::string_to_policy::single_char_mapper<policy::short_name_t>>(
            std::move(params)...);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::tuple{policy::description_t{AR_STRING("hello"){}}},
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::short_name_t{AR_STRING("l"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  policy::max_value<42>(),
                                  AR_STRING("l"){},
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::short_name_t{AR_STRING("l"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  AR_STRING("l"){},
                                  policy::max_value<42>(),
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::short_name_t{AR_STRING("l"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  policy::max_value<42>(),
                                  policy::description_t{AR_STRING("hello"){}},
                                  AR_STRING("l"){}},
                   });
}

BOOST_AUTO_TEST_CASE(convert_first_text_test)
{
    auto f = [](auto expected, auto... params) {
        using expected_type = std::decay_t<decltype(expected)>;
        test_core<expected_type,
                  utility::string_to_policy::first_text_mapper<policy::display_name_t>>(
            std::move(params)...);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::tuple{policy::display_name_t{AR_STRING("hello"){}}},
                                  policy::display_name_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::display_name_t{AR_STRING("l"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  policy::max_value<42>(),
                                  AR_STRING("l"){},
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::display_name_t{AR_STRING("l"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  AR_STRING("l"){},
                                  policy::max_value<42>(),
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::display_name_t{AR_STRING("l"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  policy::max_value<42>(),
                                  policy::description_t{AR_STRING("hello"){}},
                                  AR_STRING("l"){}},
                   });
}

BOOST_AUTO_TEST_CASE(convert_second_text_test)
{
    auto f = [](auto expected, auto... params) {
        using expected_type = std::decay_t<decltype(expected)>;
        test_core<expected_type,
                  utility::string_to_policy::first_text_mapper<policy::display_name_t>,
                  utility::string_to_policy::second_text_mapper<policy::description_t>>(
            std::move(params)...);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::tuple{policy::display_name_t{AR_STRING("hello"){}}},
                                  policy::display_name_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::display_name_t{AR_STRING("l"){}},
                                             policy::description_t{AR_STRING("hello"){}}},
                                  policy::display_name_t{AR_STRING("l"){}},
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::display_name_t{AR_STRING("l"){}},
                                             policy::description_t{AR_STRING("hello"){}},
                                             policy::max_value<42>()},
                                  AR_STRING("l"){},
                                  AR_STRING("hello"){},
                                  policy::max_value<42>()},
                       std::tuple{std::tuple{policy::display_name_t{AR_STRING("l"){}},
                                             policy::description_t{AR_STRING("hello"){}},
                                             policy::max_value<42>()},
                                  AR_STRING("l"){},
                                  policy::max_value<42>(),
                                  AR_STRING("hello"){}},
                       std::tuple{std::tuple{policy::display_name_t{AR_STRING("l"){}},
                                             policy::description_t{AR_STRING("hello"){}},
                                             policy::max_value<42>()},
                                  policy::max_value<42>(),
                                  AR_STRING("l"){},
                                  AR_STRING("hello"){}},
                   });
}

BOOST_AUTO_TEST_CASE(convert_main_three_test)
{
    auto f = [](auto expected, auto... params) {
        using expected_type = std::decay_t<decltype(expected)>;
        test_core<expected_type,
                  utility::string_to_policy::first_string_mapper<policy::long_name_t>,
                  utility::string_to_policy::second_string_mapper<policy::description_t>,
                  utility::string_to_policy::single_char_mapper<policy::short_name_t>>(
            std::move(params)...);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{std::tuple{policy::description_t{AR_STRING("hello"){}}},
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::short_name_t{AR_STRING("s"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  policy::max_value<42>(),
                                  AR_STRING("s"){},
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("long"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  policy::max_value<42>(),
                                  AR_STRING("long"){},
                                  policy::description_t{AR_STRING("hello"){}}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("long"){}},
                                             policy::short_name_t{AR_STRING("s"){}},
                                             policy::max_value<42>(),
                                             policy::description_t{AR_STRING("hello"){}}},
                                  policy::max_value<42>(),
                                  AR_STRING("long"){},
                                  policy::description_t{AR_STRING("hello"){}},
                                  AR_STRING("s"){}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("long"){}},
                                             policy::description_t{AR_STRING("hello"){}},
                                             policy::short_name_t{AR_STRING("s"){}},
                                             policy::max_value<42>()},
                                  policy::max_value<42>(),
                                  AR_STRING("long"){},
                                  AR_STRING("hello"){},
                                  AR_STRING("s"){}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("long"){}},
                                             policy::description_t{AR_STRING("hello"){}},
                                             policy::short_name_t{AR_STRING("s"){}},
                                             policy::max_value<42>()},
                                  AR_STRING("long"){},
                                  policy::max_value<42>(),
                                  AR_STRING("hello"){},
                                  AR_STRING("s"){}},
                       std::tuple{std::tuple{policy::long_name_t{AR_STRING("long"){}},
                                             policy::description_t{AR_STRING("hello"){}},
                                             policy::short_name_t{AR_STRING("s"){}},
                                             policy::max_value<42>()},
                                  AR_STRING("long"){},
                                  AR_STRING("s"){},
                                  policy::max_value<42>(),
                                  AR_STRING("hello"){}},
                   });
}

BOOST_AUTO_TEST_SUITE(death_suite)

BOOST_AUTO_TEST_CASE(unhandled_strings_test)
{
    test::death_test_compile(
        R"(
#include "arg_router/utility/string_to_policy.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/literals.hpp"

using namespace arg_router;
using namespace arg_router::literals;

int main() {
    const auto result = utility::string_to_policy::convert<
        utility::string_to_policy::first_string_mapper<policy::long_name_t>>(
            AR_STRING("long"){},
            AR_STRING("hello"){});
    return 0;
}
    )",
        "Unhandled bare strings passed");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
