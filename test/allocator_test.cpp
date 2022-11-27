// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#undef AR_ALLOCATOR
#define AR_ALLOCATOR tracking_allocator

#include <memory>

struct allocator_fixture {
    static std::size_t allocated_bytes;
    static std::size_t current_bytes;

    allocator_fixture()
    {
        allocated_bytes = 0;
        current_bytes = 0;
    }
};

std::size_t allocator_fixture::allocated_bytes = 0;
std::size_t allocator_fixture::current_bytes = 0;

template <typename T>
class tracking_allocator
{
public:
    using value_type = T;
    using pointer = T*;
    using size_type = std::size_t;

    tracking_allocator() noexcept = default;

    tracking_allocator(const tracking_allocator& other) noexcept = default;

    template <class U>
    tracking_allocator([[maybe_unused]] const tracking_allocator<U>& other) noexcept
    {
    }

    pointer allocate(size_type n)
    {
        auto p = std::allocator<T>{}.allocate(n);
        allocator_fixture::allocated_bytes += n * sizeof(T);
        allocator_fixture::current_bytes += n * sizeof(T);

        return p;
    }

    void deallocate(T* p, size_type n)
    {
        allocator_fixture::current_bytes -= n * sizeof(T);
        std::allocator<T>{}.deallocate(p, n);
    }

    bool operator==([[maybe_unused]] const tracking_allocator& other) const noexcept
    {
        return true;
    }

    bool operator!=(const tracking_allocator& other) const noexcept { return !(*this == other); }
};

#include "arg_router/basic_types.hpp"
#include "arg_router/flag.hpp"
#include "arg_router/mode.hpp"
#include "arg_router/policy/description.hpp"
#include "arg_router/policy/long_name.hpp"
#include "arg_router/policy/router.hpp"
#include "arg_router/policy/validator.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(allocator_suite)

BOOST_FIXTURE_TEST_CASE(string_test, allocator_fixture)
{
    {
        auto s = string("Try to exceed the Small String Optimisation limit!");
    }
    BOOST_CHECK_EQUAL(allocator_fixture::current_bytes, 0u);
    BOOST_CHECK_GE(allocator_fixture::allocated_bytes, 51u);
}

BOOST_FIXTURE_TEST_CASE(ostringstream_test, allocator_fixture)
{
    {
        auto ss = ostringstream{};
        ss << "Try to exceed the Small String Optimisation limit!";
    }
    BOOST_CHECK_EQUAL(allocator_fixture::current_bytes, 0u);
    BOOST_CHECK_GT(allocator_fixture::allocated_bytes, 51u);
}

BOOST_FIXTURE_TEST_CASE(vector_test, allocator_fixture)
{
    {
        auto s = vector<char>('a', 42);
    }
    BOOST_CHECK_EQUAL(allocator_fixture::current_bytes, 0u);
    BOOST_CHECK_GT(allocator_fixture::allocated_bytes, 42u);
}

BOOST_FIXTURE_TEST_CASE(root_test, allocator_fixture)
{
    {
        auto router_hit = std::optional<bool>{};
        const auto r = root(mode(flag(policy::long_name<AR_STRING("hello")>,
                                      policy::description<AR_STRING("Hello description")>),
                                 policy::router{[&](auto hello) { router_hit = hello; }}),
                            policy::validation::default_validator);

        {
            auto args = std::vector{"foo", "--hello"};
            r.parse(args.size(), const_cast<char**>(args.data()));
            BOOST_CHECK(router_hit);
            BOOST_CHECK_GE(allocator_fixture::allocated_bytes, 160u);
            BOOST_CHECK_EQUAL(allocator_fixture::current_bytes, 0u);
        }

        allocator_fixture::allocated_bytes = 0;
        router_hit.reset();

        try {
            auto args = std::vector{"foo", "--goodbye"};
            r.parse(args.size(), const_cast<char**>(args.data()));
            BOOST_CHECK(false);
        } catch (parse_exception& e) {
            BOOST_CHECK(!router_hit);
            BOOST_CHECK_GE(allocator_fixture::allocated_bytes, 139u);
            BOOST_CHECK_GE(allocator_fixture::current_bytes, 33u);
        }
    }
    BOOST_CHECK_EQUAL(allocator_fixture::current_bytes, 0u);
}

BOOST_AUTO_TEST_SUITE_END()

#undef AR_ALLOCATOR
#define AR_ALLOCATOR std::allocator
