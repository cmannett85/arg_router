/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/utility/string_view_ops.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace utility::string_view_ops;
using namespace std::string_literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(string_view_ops_suite)

BOOST_AUTO_TEST_CASE(concatenation_operator)
{
    auto f = [](auto lhs, auto rhs, auto expected) {
        const auto result = lhs + rhs;
        static_assert(
            std::is_same_v<std::decay_t<decltype(result)>, std::decay_t<decltype(expected)>>,
            "Result is incorrect type");
        BOOST_CHECK_EQUAL(result, expected);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{"hello "s, "world"sv, "hello world"s},
                       std::tuple{"hello "sv, "world"s, "hello world"s},
                   });
}

BOOST_AUTO_TEST_SUITE_END()
