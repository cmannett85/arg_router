// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/utility/exception_formatter.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(exception_formatter_suite)

BOOST_AUTO_TEST_CASE(single_token_placeholder_test)
{
    auto f = [](auto cts, auto tokens, auto expected) {
        using string_type = std::decay_t<decltype(cts)>;

        const auto formatted = utility::exception_formatter<string_type>::format(tokens);
        BOOST_CHECK_EQUAL(formatted, expected);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{AR_STRING("Hello {}"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::long_, "world"},
                                  },
                                  "Hello --world"sv},
                       std::tuple{AR_STRING("Hello {}!"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::long_, "world"},
                                  },
                                  "Hello --world!"sv},
                       std::tuple{AR_STRING("{} world!"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "Hello"},
                                  },
                                  "Hello world!"sv},
                       std::tuple{AR_STRING("{} {}!"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "Hello"},
                                      {parsing::prefix_type::long_, "world"},
                                  },
                                  "Hello --world!"sv},
                       std::tuple{AR_STRING("Hello {}, {}, {}"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::short_, "a"},
                                      {parsing::prefix_type::long_, "b"},
                                      {parsing::prefix_type::none, "c"},
                                  },
                                  "Hello -a, --b, c"sv},
                       std::tuple{AR_STRING("{} Cam!"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "Hello"},
                                      {parsing::prefix_type::long_, "world"},
                                  },
                                  "Hello Cam!"sv},
                       std::tuple{AR_STRING("{} {}!"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::none, "Hello"},
                                  },
                                  "Hello !"sv},
                       std::tuple{AR_STRING("{} {}!"){}, vector<parsing::token_type>{}, " !"sv},
                   });
}

BOOST_AUTO_TEST_CASE(mixed_placeholder_test)
{
    auto f = [](auto cts, auto tokens, auto expected) {
        using string_type = std::decay_t<decltype(cts)>;

        const auto formatted = utility::exception_formatter<string_type>::format(tokens);
        BOOST_CHECK_EQUAL(formatted, expected);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{AR_STRING("Hello {, }"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::short_, "a"},
                                      {parsing::prefix_type::long_, "b"},
                                      {parsing::prefix_type::none, "c"},
                                  },
                                  "Hello -a, --b, c"sv},
                       std::tuple{AR_STRING("Hello {}, {, }"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::short_, "a"},
                                      {parsing::prefix_type::long_, "b"},
                                  },
                                  "Hello -a, --b"sv},
                       std::tuple{AR_STRING("Hello {}, {, }, d"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::short_, "a"},
                                      {parsing::prefix_type::long_, "b"},
                                      {parsing::prefix_type::none, "c"},
                                  },
                                  "Hello -a, --b, c, d"sv},
                       std::tuple{AR_STRING("Hello {}, {} - {, }, d"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::short_, "a"},
                                      {parsing::prefix_type::long_, "b"},
                                      {parsing::prefix_type::none, "c"},
                                  },
                                  "Hello -a, --b - c, d"sv},
                       std::tuple{AR_STRING("Hello {}, {} - {, }, d"){},
                                  vector<parsing::token_type>{},
                                  "Hello ,  - , d"sv},
                       std::tuple{AR_STRING("Hello {}, { -> }"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::short_, "a"},
                                      {parsing::prefix_type::long_, "b"},
                                      {parsing::prefix_type::none, "c"},
                                  },
                                  "Hello -a, --b -> c"sv},
                   });
}

BOOST_AUTO_TEST_CASE(no_placeholder_test)
{
    auto f = [](auto cts, auto tokens, auto expected) {
        using string_type = std::decay_t<decltype(cts)>;

        const auto formatted = utility::exception_formatter<string_type>::format(tokens);
        BOOST_CHECK_EQUAL(formatted, expected);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{AR_STRING("Hello"){},
                                  vector<parsing::token_type>{
                                      {parsing::prefix_type::short_, "a"},
                                      {parsing::prefix_type::long_, "b"},
                                      {parsing::prefix_type::none, "c"},
                                  },
                                  "Hello: -a, --b, c"sv},
                       std::tuple{AR_STRING("Hello"){}, vector<parsing::token_type>{}, "Hello"sv},
                   });
}

BOOST_AUTO_TEST_CASE(death_test)
{
    test::death_test_compile({
        {R"(
#include "arg_router/utility/exception_formatter.hpp"
int main() {
    const auto fmt = arg_router::utility::exception_formatter<AR_STRING("{...} {...}")>{};
    return 0;
}
    )",
         "Can only be one greedy entry in the formatted string",
         "can_only_be_one_greedy_placeholder_test"},
        {R"(
#include "arg_router/utility/exception_formatter.hpp"
int main() {
    const auto fmt = arg_router::utility::exception_formatter<AR_STRING("{...} {}")>{};
    return 0;
}
    )",
         "Greedy entry must be last in the formatted string",
         "greedy_placeholder_must_be_last_placeholder_test"},
    });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
