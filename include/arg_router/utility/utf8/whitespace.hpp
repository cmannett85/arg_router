// Copyright (C) 2022 by Camden Mannett.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "arg_router/utility/utf8/code_point.hpp"

namespace arg_router::utility::utf8
{
/** Whitespace code points.
 *
 * Each entry is an inclusive range of code points.
 *
 * This table is generated using scripts/unicode_table_generators.py from
 * http://www.unicode.org/Public/UNIDATA/PropList.txt v14.0.0.
 */
constexpr auto whitespace_table = std::array<code_point::range, 11>{{
    {0x000009, 0x00000D},
    {0x000020, 0x000020},
    {0x000085, 0x000085},
    {0x0000A0, 0x0000A0},
    {0x001680, 0x001680},
    {0x002000, 0x00200A},
    {0x002028, 0x002028},
    {0x002029, 0x002029},
    {0x00202F, 0x00202F},
    {0x00205F, 0x00205F},
    {0x003000, 0x003000},
}};
}  // namespace arg_router::utility::utf8
