/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/utility/utf8/code_point.hpp"

#include <array>

namespace arg_router::utility::utf8
{
/** Whitespace code points.
 *
 * Each entry is an inclusive range of code points.
 *
 * This table is generated using scripts/unicode_table_generators.py from
 * http://www.unicode.org/Public/UNIDATA/PropList.txt v14.0.0.
 */
constexpr auto whitespace_table = std::array<code_point::range, 10>{{
    {0x9, 0xD},
    {0x20, 0x20},
    {0x85, 0x85},
    {0xA0, 0xA0},
    {0x1680, 0x1680},
    {0x2000, 0x200A},
    {0x2028, 0x2029},
    {0x202F, 0x202F},
    {0x205F, 0x205F},
    {0x3000, 0x3000},
}};
}  // namespace arg_router::utility::utf8
