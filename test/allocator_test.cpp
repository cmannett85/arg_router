// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#undef AR_ALLOCATOR
#define AR_ALLOCATOR arg_router::test::tracking_allocator

#include <memory>

namespace arg_router::test
{
std::size_t allocated_bytes = 0;

template <typename T>
class tracking_allocator
{
public:
    using value_type = T;
    using pointer = T*;
    using size_type = std::size_t;

    tracking_allocator() = default;

    template <class U>
    tracking_allocator([[maybe_unused]] const tracking_allocator<U>& other) noexcept
    {
    }

    pointer allocate(size_type n)
    {
        auto p = alloc_.allocate(n);
        allocated_bytes += n * sizeof(T);

        return p;
    }

    void deallocate(T* p, size_type n)
    {
        allocated_bytes -= n * sizeof(T);
        alloc_.deallocate(p, n);
    }

    bool operator==(const tracking_allocator& other) const noexcept
    {
        return alloc_ == other.alloc_;
    }

    bool operator!=(const tracking_allocator& other) const noexcept
    {
        return alloc_ != other.alloc_;
    }

private:
    std::allocator<T> alloc_;
};
}  // namespace arg_router::test

#include "arg_router/basic_types.hpp"

#include "test_helpers.hpp"

using namespace arg_router;

BOOST_AUTO_TEST_SUITE(allocator_suite)

// The allocation strategy is implementation defined, so just test that _something_ was allocated
// via the custom allocator

BOOST_AUTO_TEST_CASE(string_test)
{
    arg_router::test::allocated_bytes = 0;
    auto s = string("Try to exceed the Small String Optimisation limit!");
    BOOST_CHECK_GT(arg_router::test::allocated_bytes, 0u);
}

BOOST_AUTO_TEST_CASE(ostringstream_test)
{
    arg_router::test::allocated_bytes = 0;
    auto ss = ostringstream{};
    ss << "Try to exceed the Small String Optimisation limit!";
    BOOST_CHECK_GT(arg_router::test::allocated_bytes, 0u);
}

BOOST_AUTO_TEST_CASE(vector_test)
{
    arg_router::test::allocated_bytes = 0;
    auto s = vector<char>('a', 42);
    BOOST_CHECK_GT(arg_router::test::allocated_bytes, 0u);
}

BOOST_AUTO_TEST_SUITE_END()
