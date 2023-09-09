// Copyright (C) 2023 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace arg_router
{
namespace utility
{
/** Trivial utility function object that always returns true regardless of input arguments. */
struct always_true {
    /** Always returns true.
     *
     * @tparam T Input arg types, all ignored
     * @param args Args, all ignored
     * @return True
     */
    template <typename... T>
    bool operator()([[maybe_unused]] T&&... args) const noexcept
    {
        return true;
    }
};
}  // namespace utility
}  // namespace arg_router
