/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/utility/utf8/code_point.hpp"

#include <array>

namespace arg_router::utility::utf8
{
/** Zero-width code points i.e. those that occupy 0 terminal columns when rendered.
 *
 * Each entry is an inclusive range of code points.
 *
 * This table is generated using scripts/unicode_table_generators.py from
 * http://www.unicode.org/Public/UNIDATA/extracted/DerivedGeneralCategory.txt v14.0.0.
 */
constexpr auto zero_width_table = std::array<code_point::range, 341>{{
    {0x300, 0x36F},     {0x483, 0x487},     {0x488, 0x489},     {0x591, 0x5BD},
    {0x5BF, 0x5BF},     {0x5C1, 0x5C2},     {0x5C4, 0x5C5},     {0x5C7, 0x5C7},
    {0x610, 0x61A},     {0x64B, 0x65F},     {0x670, 0x670},     {0x6D6, 0x6DC},
    {0x6DF, 0x6E4},     {0x6E7, 0x6E8},     {0x6EA, 0x6ED},     {0x711, 0x711},
    {0x730, 0x74A},     {0x7A6, 0x7B0},     {0x7EB, 0x7F3},     {0x7FD, 0x7FD},
    {0x816, 0x819},     {0x81B, 0x823},     {0x825, 0x827},     {0x829, 0x82D},
    {0x859, 0x85B},     {0x898, 0x89F},     {0x8CA, 0x8E1},     {0x8E3, 0x902},
    {0x93A, 0x93A},     {0x93C, 0x93C},     {0x941, 0x948},     {0x94D, 0x94D},
    {0x951, 0x957},     {0x962, 0x963},     {0x981, 0x981},     {0x9BC, 0x9BC},
    {0x9C1, 0x9C4},     {0x9CD, 0x9CD},     {0x9E2, 0x9E3},     {0x9FE, 0x9FE},
    {0xA01, 0xA02},     {0xA3C, 0xA3C},     {0xA41, 0xA42},     {0xA47, 0xA48},
    {0xA4B, 0xA4D},     {0xA51, 0xA51},     {0xA70, 0xA71},     {0xA75, 0xA75},
    {0xA81, 0xA82},     {0xABC, 0xABC},     {0xAC1, 0xAC5},     {0xAC7, 0xAC8},
    {0xACD, 0xACD},     {0xAE2, 0xAE3},     {0xAFA, 0xAFF},     {0xB01, 0xB01},
    {0xB3C, 0xB3C},     {0xB3F, 0xB3F},     {0xB41, 0xB44},     {0xB4D, 0xB4D},
    {0xB55, 0xB56},     {0xB62, 0xB63},     {0xB82, 0xB82},     {0xBC0, 0xBC0},
    {0xBCD, 0xBCD},     {0xC00, 0xC00},     {0xC04, 0xC04},     {0xC3C, 0xC3C},
    {0xC3E, 0xC40},     {0xC46, 0xC48},     {0xC4A, 0xC4D},     {0xC55, 0xC56},
    {0xC62, 0xC63},     {0xC81, 0xC81},     {0xCBC, 0xCBC},     {0xCBF, 0xCBF},
    {0xCC6, 0xCC6},     {0xCCC, 0xCCD},     {0xCE2, 0xCE3},     {0xD00, 0xD01},
    {0xD3B, 0xD3C},     {0xD41, 0xD44},     {0xD4D, 0xD4D},     {0xD62, 0xD63},
    {0xD81, 0xD81},     {0xDCA, 0xDCA},     {0xDD2, 0xDD4},     {0xDD6, 0xDD6},
    {0xE31, 0xE31},     {0xE34, 0xE3A},     {0xE47, 0xE4E},     {0xEB1, 0xEB1},
    {0xEB4, 0xEBC},     {0xEC8, 0xECD},     {0xF18, 0xF19},     {0xF35, 0xF35},
    {0xF37, 0xF37},     {0xF39, 0xF39},     {0xF71, 0xF7E},     {0xF80, 0xF84},
    {0xF86, 0xF87},     {0xF8D, 0xF97},     {0xF99, 0xFBC},     {0xFC6, 0xFC6},
    {0x102D, 0x1030},   {0x1032, 0x1037},   {0x1039, 0x103A},   {0x103D, 0x103E},
    {0x1058, 0x1059},   {0x105E, 0x1060},   {0x1071, 0x1074},   {0x1082, 0x1082},
    {0x1085, 0x1086},   {0x108D, 0x108D},   {0x109D, 0x109D},   {0x135D, 0x135F},
    {0x1712, 0x1714},   {0x1732, 0x1733},   {0x1752, 0x1753},   {0x1772, 0x1773},
    {0x17B4, 0x17B5},   {0x17B7, 0x17BD},   {0x17C6, 0x17C6},   {0x17C9, 0x17D3},
    {0x17DD, 0x17DD},   {0x180B, 0x180D},   {0x180F, 0x180F},   {0x1885, 0x1886},
    {0x18A9, 0x18A9},   {0x1920, 0x1922},   {0x1927, 0x1928},   {0x1932, 0x1932},
    {0x1939, 0x193B},   {0x1A17, 0x1A18},   {0x1A1B, 0x1A1B},   {0x1A56, 0x1A56},
    {0x1A58, 0x1A5E},   {0x1A60, 0x1A60},   {0x1A62, 0x1A62},   {0x1A65, 0x1A6C},
    {0x1A73, 0x1A7C},   {0x1A7F, 0x1A7F},   {0x1AB0, 0x1ABD},   {0x1ABE, 0x1ABE},
    {0x1ABF, 0x1ACE},   {0x1B00, 0x1B03},   {0x1B34, 0x1B34},   {0x1B36, 0x1B3A},
    {0x1B3C, 0x1B3C},   {0x1B42, 0x1B42},   {0x1B6B, 0x1B73},   {0x1B80, 0x1B81},
    {0x1BA2, 0x1BA5},   {0x1BA8, 0x1BA9},   {0x1BAB, 0x1BAD},   {0x1BE6, 0x1BE6},
    {0x1BE8, 0x1BE9},   {0x1BED, 0x1BED},   {0x1BEF, 0x1BF1},   {0x1C2C, 0x1C33},
    {0x1C36, 0x1C37},   {0x1CD0, 0x1CD2},   {0x1CD4, 0x1CE0},   {0x1CE2, 0x1CE8},
    {0x1CED, 0x1CED},   {0x1CF4, 0x1CF4},   {0x1CF8, 0x1CF9},   {0x1DC0, 0x1DFF},
    {0x20D0, 0x20DC},   {0x20DD, 0x20E0},   {0x20E1, 0x20E1},   {0x20E2, 0x20E4},
    {0x20E5, 0x20F0},   {0x2CEF, 0x2CF1},   {0x2D7F, 0x2D7F},   {0x2DE0, 0x2DFF},
    {0x302A, 0x302D},   {0x3099, 0x309A},   {0xA66F, 0xA66F},   {0xA670, 0xA672},
    {0xA674, 0xA67D},   {0xA69E, 0xA69F},   {0xA6F0, 0xA6F1},   {0xA802, 0xA802},
    {0xA806, 0xA806},   {0xA80B, 0xA80B},   {0xA825, 0xA826},   {0xA82C, 0xA82C},
    {0xA8C4, 0xA8C5},   {0xA8E0, 0xA8F1},   {0xA8FF, 0xA8FF},   {0xA926, 0xA92D},
    {0xA947, 0xA951},   {0xA980, 0xA982},   {0xA9B3, 0xA9B3},   {0xA9B6, 0xA9B9},
    {0xA9BC, 0xA9BD},   {0xA9E5, 0xA9E5},   {0xAA29, 0xAA2E},   {0xAA31, 0xAA32},
    {0xAA35, 0xAA36},   {0xAA43, 0xAA43},   {0xAA4C, 0xAA4C},   {0xAA7C, 0xAA7C},
    {0xAAB0, 0xAAB0},   {0xAAB2, 0xAAB4},   {0xAAB7, 0xAAB8},   {0xAABE, 0xAABF},
    {0xAAC1, 0xAAC1},   {0xAAEC, 0xAAED},   {0xAAF6, 0xAAF6},   {0xABE5, 0xABE5},
    {0xABE8, 0xABE8},   {0xABED, 0xABED},   {0xFB1E, 0xFB1E},   {0xFE00, 0xFE0F},
    {0xFE20, 0xFE2F},   {0x101FD, 0x101FD}, {0x102E0, 0x102E0}, {0x10376, 0x1037A},
    {0x10A01, 0x10A03}, {0x10A05, 0x10A06}, {0x10A0C, 0x10A0F}, {0x10A38, 0x10A3A},
    {0x10A3F, 0x10A3F}, {0x10AE5, 0x10AE6}, {0x10D24, 0x10D27}, {0x10EAB, 0x10EAC},
    {0x10F46, 0x10F50}, {0x10F82, 0x10F85}, {0x11001, 0x11001}, {0x11038, 0x11046},
    {0x11070, 0x11070}, {0x11073, 0x11074}, {0x1107F, 0x11081}, {0x110B3, 0x110B6},
    {0x110B9, 0x110BA}, {0x110C2, 0x110C2}, {0x11100, 0x11102}, {0x11127, 0x1112B},
    {0x1112D, 0x11134}, {0x11173, 0x11173}, {0x11180, 0x11181}, {0x111B6, 0x111BE},
    {0x111C9, 0x111CC}, {0x111CF, 0x111CF}, {0x1122F, 0x11231}, {0x11234, 0x11234},
    {0x11236, 0x11237}, {0x1123E, 0x1123E}, {0x112DF, 0x112DF}, {0x112E3, 0x112EA},
    {0x11300, 0x11301}, {0x1133B, 0x1133C}, {0x11340, 0x11340}, {0x11366, 0x1136C},
    {0x11370, 0x11374}, {0x11438, 0x1143F}, {0x11442, 0x11444}, {0x11446, 0x11446},
    {0x1145E, 0x1145E}, {0x114B3, 0x114B8}, {0x114BA, 0x114BA}, {0x114BF, 0x114C0},
    {0x114C2, 0x114C3}, {0x115B2, 0x115B5}, {0x115BC, 0x115BD}, {0x115BF, 0x115C0},
    {0x115DC, 0x115DD}, {0x11633, 0x1163A}, {0x1163D, 0x1163D}, {0x1163F, 0x11640},
    {0x116AB, 0x116AB}, {0x116AD, 0x116AD}, {0x116B0, 0x116B5}, {0x116B7, 0x116B7},
    {0x1171D, 0x1171F}, {0x11722, 0x11725}, {0x11727, 0x1172B}, {0x1182F, 0x11837},
    {0x11839, 0x1183A}, {0x1193B, 0x1193C}, {0x1193E, 0x1193E}, {0x11943, 0x11943},
    {0x119D4, 0x119D7}, {0x119DA, 0x119DB}, {0x119E0, 0x119E0}, {0x11A01, 0x11A0A},
    {0x11A33, 0x11A38}, {0x11A3B, 0x11A3E}, {0x11A47, 0x11A47}, {0x11A51, 0x11A56},
    {0x11A59, 0x11A5B}, {0x11A8A, 0x11A96}, {0x11A98, 0x11A99}, {0x11C30, 0x11C36},
    {0x11C38, 0x11C3D}, {0x11C3F, 0x11C3F}, {0x11C92, 0x11CA7}, {0x11CAA, 0x11CB0},
    {0x11CB2, 0x11CB3}, {0x11CB5, 0x11CB6}, {0x11D31, 0x11D36}, {0x11D3A, 0x11D3A},
    {0x11D3C, 0x11D3D}, {0x11D3F, 0x11D45}, {0x11D47, 0x11D47}, {0x11D90, 0x11D91},
    {0x11D95, 0x11D95}, {0x11D97, 0x11D97}, {0x11EF3, 0x11EF4}, {0x16AF0, 0x16AF4},
    {0x16B30, 0x16B36}, {0x16F4F, 0x16F4F}, {0x16F8F, 0x16F92}, {0x16FE4, 0x16FE4},
    {0x1BC9D, 0x1BC9E}, {0x1CF00, 0x1CF2D}, {0x1CF30, 0x1CF46}, {0x1D167, 0x1D169},
    {0x1D17B, 0x1D182}, {0x1D185, 0x1D18B}, {0x1D1AA, 0x1D1AD}, {0x1D242, 0x1D244},
    {0x1DA00, 0x1DA36}, {0x1DA3B, 0x1DA6C}, {0x1DA75, 0x1DA75}, {0x1DA84, 0x1DA84},
    {0x1DA9B, 0x1DA9F}, {0x1DAA1, 0x1DAAF}, {0x1E000, 0x1E006}, {0x1E008, 0x1E018},
    {0x1E01B, 0x1E021}, {0x1E023, 0x1E024}, {0x1E026, 0x1E02A}, {0x1E130, 0x1E136},
    {0x1E2AE, 0x1E2AE}, {0x1E2EC, 0x1E2EF}, {0x1E8D0, 0x1E8D6}, {0x1E944, 0x1E94A},
    {0xE0100, 0xE01EF},
}};
}  // namespace arg_router::utility::utf8
