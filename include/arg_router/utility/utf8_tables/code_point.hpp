/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include <cstdint>

namespace arg_router
{
namespace utility
{
namespace utf8
{
/** Code point type. */
using code_point = std::uint32_t;

/** Defines an @em inclusive contiguous range of code points. */
struct code_point_range {
    code_point first;  ///< First code point in range
    code_point last;   ///< Last code point in range

    /** Less than operator.
     *
     * @param other Instance to compare against
     * @return True if this is less than @a other
     */
    constexpr bool operator<(code_point_range other) const noexcept
    {
        if (first == other.first) {
            return last < other.last;
        }
        return first < other.first;
    }

    /** Less than operator for a single code point.
     *
     * @param cp Instance to compare against
     * @return True if the start of this range is less than @a cp
     */
    constexpr bool operator<(code_point cp) const noexcept { return first < cp; }
};
}  // namespace utf8
}  // namespace utility
}  // namespace arg_router
