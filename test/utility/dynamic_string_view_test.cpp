// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arg_router/utility/dynamic_string_view.hpp"

#include "test_helpers.hpp"

using namespace arg_router;
using namespace std::string_literals;
using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(dynamic_string_view_suite)

BOOST_AUTO_TEST_CASE(default_construction)
{
    auto s1 = utility::dynamic_string_view{};
    BOOST_CHECK_EQUAL(s1.size(), 0);
    BOOST_CHECK_EQUAL(s1.internal_storage_size(), 0);
    BOOST_CHECK_EQUAL(s1.empty(), true);
    BOOST_CHECK_EQUAL((std::distance(s1.begin(), s1.end())), 0);
}

BOOST_AUTO_TEST_CASE(initial_construction)
{
    auto f = [](auto input, auto expected_size) {
        auto dsv = utility::dynamic_string_view{input};
        BOOST_CHECK_EQUAL(dsv.size(), expected_size);
        BOOST_CHECK_EQUAL(dsv.internal_storage_size(), 0);
        BOOST_CHECK_EQUAL(dsv.empty(), expected_size == 0);
        BOOST_CHECK_EQUAL((std::distance(dsv.begin(), dsv.end())), expected_size);
    };

    test::data_set(f,
                   std::tuple{
                       std::tuple{"", 0},
                       std::tuple{"hello", 5},
                       std::tuple{""sv, 0},
                       std::tuple{"hello"sv, 5},
                       std::tuple{""s, 0},
                       std::tuple{"hello"s, 5},
                   });
}

BOOST_AUTO_TEST_CASE(string_move_construction)
{
    auto s1 = "hello"s;
    auto s2 = utility::dynamic_string_view{std::move(s1)};

    BOOST_CHECK_EQUAL(s1.empty(), true);

    BOOST_CHECK_EQUAL(s2.size(), 5);
    BOOST_CHECK_EQUAL(s2.internal_storage_size(), 5);
    BOOST_CHECK_EQUAL(s2.empty(), false);
    BOOST_CHECK_EQUAL((std::distance(s2.begin(), s2.end())), 5);
}

BOOST_AUTO_TEST_CASE(view_copy_construction)
{
    auto s1 = utility::dynamic_string_view{"hello"};
    auto s2 = s1;

    BOOST_CHECK_EQUAL(s1.size(), 5);
    BOOST_CHECK_EQUAL(s1.internal_storage_size(), 0);
    BOOST_CHECK_EQUAL(s1.empty(), false);
    BOOST_CHECK_EQUAL((std::distance(s1.begin(), s1.end())), 5);

    BOOST_CHECK_EQUAL(s2.size(), 5);
    BOOST_CHECK_EQUAL(s2.internal_storage_size(), 0);
    BOOST_CHECK_EQUAL(s2.empty(), false);
    BOOST_CHECK_EQUAL((std::distance(s2.begin(), s2.end())), 5);
}

BOOST_AUTO_TEST_CASE(storage_copy_construction)
{
    auto s1 = utility::dynamic_string_view{"hello"s};
    auto s2 = s1;

    BOOST_CHECK_EQUAL(s1.size(), 5);
    BOOST_CHECK_EQUAL(s1.internal_storage_size(), 5);
    BOOST_CHECK_EQUAL(s1.empty(), false);
    BOOST_CHECK_EQUAL((std::distance(s1.begin(), s1.end())), 5);

    BOOST_CHECK_EQUAL(s2.size(), 5);
    BOOST_CHECK_EQUAL(s2.internal_storage_size(), 5);
    BOOST_CHECK_EQUAL(s2.empty(), false);
    BOOST_CHECK_EQUAL((std::distance(s2.begin(), s2.end())), 5);
}

BOOST_AUTO_TEST_CASE(move_construction)
{
    {
        auto s1 = utility::dynamic_string_view{"hello"};
        auto s2 = utility::dynamic_string_view{std::move(s1)};

        BOOST_CHECK_EQUAL(s1.size(), 0);
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 0);
        BOOST_CHECK_EQUAL(s1.empty(), true);
        BOOST_CHECK_EQUAL((std::distance(s1.begin(), s1.end())), 0);

        BOOST_CHECK_EQUAL(s2.size(), 5);
        BOOST_CHECK_EQUAL(s2.internal_storage_size(), 0);
        BOOST_CHECK_EQUAL(s2.empty(), false);
        BOOST_CHECK_EQUAL((std::distance(s2.begin(), s2.end())), 5);
    }

    {
        auto s1 = utility::dynamic_string_view{"hello"s};
        auto s2 = utility::dynamic_string_view{std::move(s1)};

        BOOST_CHECK_EQUAL(s1.size(), 0);
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 0);
        BOOST_CHECK_EQUAL(s1.empty(), true);
        BOOST_CHECK_EQUAL((std::distance(s1.begin(), s1.end())), 0);

        BOOST_CHECK_EQUAL(s2.size(), 5);
        BOOST_CHECK_EQUAL(s2.internal_storage_size(), 5);
        BOOST_CHECK_EQUAL(s2.empty(), false);
        BOOST_CHECK_EQUAL((std::distance(s2.begin(), s2.end())), 5);
    }
}

BOOST_AUTO_TEST_CASE(move_assignment_operator)
{
    {
        auto s1 = utility::dynamic_string_view{"hello"};
        auto s2 = utility::dynamic_string_view{};
        s2 = std::move(s1);

        BOOST_CHECK_EQUAL(s1.size(), 0);
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 0);
        BOOST_CHECK_EQUAL(s1.empty(), true);
        BOOST_CHECK_EQUAL((std::distance(s1.begin(), s1.end())), 0);

        BOOST_CHECK_EQUAL(s2.size(), 5);
        BOOST_CHECK_EQUAL(s2.internal_storage_size(), 0);
        BOOST_CHECK_EQUAL(s2.empty(), false);
        BOOST_CHECK_EQUAL((std::distance(s2.begin(), s2.end())), 5);
    }

    {
        auto s1 = utility::dynamic_string_view{"hello"s};
        auto s2 = utility::dynamic_string_view{};
        s2 = std::move(s1);

        BOOST_CHECK_EQUAL(s1.size(), 0);
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 0);
        BOOST_CHECK_EQUAL(s1.empty(), true);
        BOOST_CHECK_EQUAL((std::distance(s1.begin(), s1.end())), 0);

        BOOST_CHECK_EQUAL(s2.size(), 5);
        BOOST_CHECK_EQUAL(s2.internal_storage_size(), 5);
        BOOST_CHECK_EQUAL(s2.empty(), false);
        BOOST_CHECK_EQUAL((std::distance(s2.begin(), s2.end())), 5);
    }
}

BOOST_AUTO_TEST_CASE(copy_assignment_operator)
{
    {
        auto s1 = utility::dynamic_string_view{"hello"};
        auto s2 = utility::dynamic_string_view{};
        s2 = s1;

        BOOST_CHECK_EQUAL(s1.size(), 5);
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 0);
        BOOST_CHECK_EQUAL(s1.empty(), false);
        BOOST_CHECK_EQUAL((std::distance(s1.begin(), s1.end())), 5);

        BOOST_CHECK_EQUAL(s2.size(), 5);
        BOOST_CHECK_EQUAL(s2.internal_storage_size(), 0);
        BOOST_CHECK_EQUAL(s2.empty(), false);
        BOOST_CHECK_EQUAL((std::distance(s2.begin(), s2.end())), 5);
    }

    {
        auto s1 = utility::dynamic_string_view{"hello"s};
        auto s2 = utility::dynamic_string_view{};
        s2 = s1;

        BOOST_CHECK_EQUAL(s1.size(), 5);
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 5);
        BOOST_CHECK_EQUAL(s1.empty(), false);
        BOOST_CHECK_EQUAL((std::distance(s1.begin(), s1.end())), 5);

        BOOST_CHECK_EQUAL(s2.size(), 5);
        BOOST_CHECK_EQUAL(s2.internal_storage_size(), 5);
        BOOST_CHECK_EQUAL(s2.empty(), false);
        BOOST_CHECK_EQUAL((std::distance(s2.begin(), s2.end())), 5);
    }
}

BOOST_AUTO_TEST_CASE(convert_to_internal_storage)
{
    auto s1 = utility::dynamic_string_view{"hello"};
    BOOST_CHECK_EQUAL(s1.internal_storage_size(), 0);

    s1.convert_to_internal_storage();
    BOOST_CHECK_EQUAL(s1.internal_storage_size(), 5);

    s1.convert_to_internal_storage();
    BOOST_CHECK_EQUAL(s1.internal_storage_size(), 5);
}

BOOST_AUTO_TEST_CASE(string_view_conversion_operator)
{
    const auto s1 = utility::dynamic_string_view{"hello"};

    // Explicit
    {
        const auto sv = static_cast<std::string_view>(s1);
        BOOST_CHECK_EQUAL(sv.size(), 5);
    }

    // Implicit
    {
        auto f = [](std::string_view sv) { BOOST_CHECK_EQUAL(sv.size(), 5); };
        f(s1);
    }
}

BOOST_AUTO_TEST_CASE(equality_operator)
{
    BOOST_CHECK(utility::dynamic_string_view{"hello"} == utility::dynamic_string_view{"hello"});
    BOOST_CHECK(utility::dynamic_string_view{"hello"} != utility::dynamic_string_view{"world"});
    BOOST_CHECK(utility::dynamic_string_view{} == utility::dynamic_string_view{});

    BOOST_CHECK(utility::dynamic_string_view{"hello"s} == utility::dynamic_string_view{"hello"});
    BOOST_CHECK(utility::dynamic_string_view{"hello"s} != utility::dynamic_string_view{"world"});
    BOOST_CHECK(utility::dynamic_string_view{"hello"} == utility::dynamic_string_view{"hello"s});
    BOOST_CHECK(utility::dynamic_string_view{"hello"} != utility::dynamic_string_view{"world"s});
    BOOST_CHECK(utility::dynamic_string_view{"hello"s} == utility::dynamic_string_view{"hello"s});
    BOOST_CHECK(utility::dynamic_string_view{"hello"s} != utility::dynamic_string_view{"world"s});
}

BOOST_AUTO_TEST_CASE(iterators)
{
    const auto s1 = utility::dynamic_string_view{"hello"};
    const auto s2 = std::string(s1.begin(), s1.end());

    BOOST_CHECK_EQUAL(std::string_view{s1}, "hello");
    BOOST_CHECK_EQUAL(s2, "hello");
}

BOOST_AUTO_TEST_CASE(inplace_concatenation_operator)
{
    {
        auto s1 = utility::dynamic_string_view{"hello"};
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 0);

        s1 += " world";
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 11);

        BOOST_CHECK_EQUAL(std::string_view{s1}, "hello world");
    }

    {
        auto s1 = utility::dynamic_string_view{"hello"s};
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 5);

        s1 += " world";
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 11);

        BOOST_CHECK_EQUAL(std::string_view{s1}, "hello world");
    }
}

BOOST_AUTO_TEST_CASE(pre_concatenation_operator)
{
    {
        auto s1 = utility::dynamic_string_view{"hello"};
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 0);

        auto s2 = utility::dynamic_string_view{" world"};
        BOOST_CHECK_EQUAL(s2.internal_storage_size(), 0);

        const auto s3 = s1 + s2;
        BOOST_CHECK_EQUAL(std::string_view{s3}, "hello world");
        BOOST_CHECK_EQUAL(s3.internal_storage_size(), 11);
    }

    {
        auto s1 = utility::dynamic_string_view{"hello"s};
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 5);

        auto s2 = utility::dynamic_string_view{" world"s};
        BOOST_CHECK_EQUAL(s2.internal_storage_size(), 6);

        const auto s3 = s1 + s2;
        BOOST_CHECK_EQUAL(std::string_view{s3}, "hello world");
        BOOST_CHECK_EQUAL(s3.internal_storage_size(), 11);
    }

    {
        const auto s1 =
            utility::dynamic_string_view{"hello"s} + utility::dynamic_string_view{" world"s};
        BOOST_CHECK_EQUAL(std::string_view{s1}, "hello world");
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 11);
    }

    {
        const auto s1 = utility::dynamic_string_view{"hello"} + " world";
        BOOST_CHECK_EQUAL(std::string_view{s1}, "hello world");
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 11);
    }

    {
        const auto s1 = utility::dynamic_string_view{"hello"} + " world"sv;
        BOOST_CHECK_EQUAL(std::string_view{s1}, "hello world");
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 11);
    }

    {
        const auto s1 = utility::dynamic_string_view{"hello"} + " world"s;
        BOOST_CHECK_EQUAL(std::string_view{s1}, "hello world");
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 11);
    }
}

BOOST_AUTO_TEST_CASE(post_concatenation_operator)
{
    {
        const auto s1 = "hello" + utility::dynamic_string_view{" world"};
        BOOST_CHECK_EQUAL(std::string_view{s1}, "hello world");
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 11);
    }

    {
        const auto s1 = "hello" + utility::dynamic_string_view{" world"sv};
        BOOST_CHECK_EQUAL(std::string_view{s1}, "hello world");
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 11);
    }

    {
        const auto s1 = "hello" + utility::dynamic_string_view{" world"s};
        BOOST_CHECK_EQUAL(std::string_view{s1}, "hello world");
        BOOST_CHECK_EQUAL(s1.internal_storage_size(), 11);
    }
}

BOOST_AUTO_TEST_SUITE_END()
