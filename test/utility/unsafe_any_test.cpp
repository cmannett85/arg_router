/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#include "arg_router/utility/unsafe_any.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(utility_suite)

BOOST_AUTO_TEST_SUITE(unsafe_any_suite)

BOOST_AUTO_TEST_CASE(internal_storage_test)
{
    auto f = [](auto value) {
        using value_type = std::decay_t<decltype(value)>;

        {
            const auto any = utility::unsafe_any{value};
            BOOST_CHECK(any.has_value());

            decltype(auto) any_value = any.get<value_type>();
            if constexpr (sizeof(value_type) > sizeof(std::size_t)) {
                static_assert(std::is_same_v<decltype(any_value), const decltype(value)&>,
                              "Type mismatch");
            } else {
                static_assert(std::is_same_v<decltype(any_value), decltype(value)>,
                              "Type mismatch");
            }
            BOOST_CHECK_EQUAL(any_value, value);
        }

        {
            auto any = utility::unsafe_any{value};
            BOOST_CHECK(any.has_value());

            decltype(auto) any_value = any.get<value_type>();
            static_assert(
                std::is_same_v<decltype(any_value), std::add_lvalue_reference_t<decltype(value)>>,
                "Type mismatch");
            BOOST_CHECK_EQUAL(any_value, value);
        }
    };

    test::data_set(
        f,
        std::tuple{std::tuple{42}, std::tuple{3.14}, std::tuple{42u}, std::tuple{std::size_t{42}}});
}

BOOST_AUTO_TEST_CASE(pod_external_storage_test)
{
    using value_type = std::array<char, 32>;
    auto value = value_type{};

    {
        const auto any = utility::unsafe_any{value};
        BOOST_CHECK(any.has_value());

        decltype(auto) any_value = any.get<value_type>();
        static_assert(std::is_same_v<decltype(any_value), const value_type&>, "Type mismatch");
        BOOST_CHECK(any_value == value);
    }

    {
        auto any = utility::unsafe_any{value};
        BOOST_CHECK(any.has_value());

        decltype(auto) any_value = any.get<value_type>();
        static_assert(std::is_same_v<decltype(any_value), value_type&>, "Type mismatch");
        BOOST_CHECK(any_value == value);
    }
}

BOOST_AUTO_TEST_CASE(non_pod_external_storage_test)
{
    auto value = std::string(128, 'a');  // Big enough to exceed SSO

    {
        const auto any = utility::unsafe_any{value};
        BOOST_CHECK(any.has_value());

        decltype(auto) any_value = any.get<std::string>();
        static_assert(std::is_same_v<decltype(any_value), const std::string&>, "Type mismatch");
        BOOST_CHECK_EQUAL(any_value, value);
    }

    {
        auto any = utility::unsafe_any{value};
        BOOST_CHECK(any.has_value());

        decltype(auto) any_value = any.get<std::string>();
        static_assert(std::is_same_v<decltype(any_value), std::string&>, "Type mismatch");
        BOOST_CHECK_EQUAL(any_value, value);
    }
}

BOOST_AUTO_TEST_CASE(default_construction_test)
{
    auto any = utility::unsafe_any{};
    BOOST_CHECK(!any.has_value());

    any = 42;
    BOOST_CHECK(any.has_value());

    decltype(auto) any_value = any.get<int>();
    static_assert(std::is_same_v<decltype(any_value), int&>, "Type mismatch");
    BOOST_CHECK_EQUAL(any_value, 42);
}

BOOST_AUTO_TEST_CASE(copy_construction_test)
{
    const auto any1 = utility::unsafe_any{42};
    const auto any2 = utility::unsafe_any{"hello"};

    const auto any3 = any1;
    BOOST_REQUIRE(any3.has_value());
    decltype(auto) any3_value = any3.get<int>();
    static_assert(std::is_same_v<decltype(any3_value), int>, "Type mismatch");

    const auto any4 = any2;
    BOOST_REQUIRE(any4.has_value());
    decltype(auto) any4_value = any4.get<const char*>();
    static_assert(std::is_same_v<decltype(any4_value), const char*>, "Type mismatch");
}

BOOST_AUTO_TEST_CASE(move_construction_test)
{
    auto any1 = utility::unsafe_any{42};
    BOOST_CHECK(any1.has_value());

    auto any2 = std::move(any1);
    BOOST_CHECK(!any1.has_value());
    BOOST_CHECK(any2.has_value());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
