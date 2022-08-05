/* Copyright (C) 2022 by Camden Mannett.  All rights reserved. */

#pragma once

#include "arg_router/utility/utf8/code_point.hpp"

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
    {0x000300, 0x00036F}, {0x000483, 0x000487}, {0x000488, 0x000489}, {0x000591, 0x0005BD},
    {0x0005BF, 0x0005BF}, {0x0005C1, 0x0005C2}, {0x0005C4, 0x0005C5}, {0x0005C7, 0x0005C7},
    {0x000610, 0x00061A}, {0x00064B, 0x00065F}, {0x000670, 0x000670}, {0x0006D6, 0x0006DC},
    {0x0006DF, 0x0006E4}, {0x0006E7, 0x0006E8}, {0x0006EA, 0x0006ED}, {0x000711, 0x000711},
    {0x000730, 0x00074A}, {0x0007A6, 0x0007B0}, {0x0007EB, 0x0007F3}, {0x0007FD, 0x0007FD},
    {0x000816, 0x000819}, {0x00081B, 0x000823}, {0x000825, 0x000827}, {0x000829, 0x00082D},
    {0x000859, 0x00085B}, {0x000898, 0x00089F}, {0x0008CA, 0x0008E1}, {0x0008E3, 0x000902},
    {0x00093A, 0x00093A}, {0x00093C, 0x00093C}, {0x000941, 0x000948}, {0x00094D, 0x00094D},
    {0x000951, 0x000957}, {0x000962, 0x000963}, {0x000981, 0x000981}, {0x0009BC, 0x0009BC},
    {0x0009C1, 0x0009C4}, {0x0009CD, 0x0009CD}, {0x0009E2, 0x0009E3}, {0x0009FE, 0x0009FE},
    {0x000A01, 0x000A02}, {0x000A3C, 0x000A3C}, {0x000A41, 0x000A42}, {0x000A47, 0x000A48},
    {0x000A4B, 0x000A4D}, {0x000A51, 0x000A51}, {0x000A70, 0x000A71}, {0x000A75, 0x000A75},
    {0x000A81, 0x000A82}, {0x000ABC, 0x000ABC}, {0x000AC1, 0x000AC5}, {0x000AC7, 0x000AC8},
    {0x000ACD, 0x000ACD}, {0x000AE2, 0x000AE3}, {0x000AFA, 0x000AFF}, {0x000B01, 0x000B01},
    {0x000B3C, 0x000B3C}, {0x000B3F, 0x000B3F}, {0x000B41, 0x000B44}, {0x000B4D, 0x000B4D},
    {0x000B55, 0x000B56}, {0x000B62, 0x000B63}, {0x000B82, 0x000B82}, {0x000BC0, 0x000BC0},
    {0x000BCD, 0x000BCD}, {0x000C00, 0x000C00}, {0x000C04, 0x000C04}, {0x000C3C, 0x000C3C},
    {0x000C3E, 0x000C40}, {0x000C46, 0x000C48}, {0x000C4A, 0x000C4D}, {0x000C55, 0x000C56},
    {0x000C62, 0x000C63}, {0x000C81, 0x000C81}, {0x000CBC, 0x000CBC}, {0x000CBF, 0x000CBF},
    {0x000CC6, 0x000CC6}, {0x000CCC, 0x000CCD}, {0x000CE2, 0x000CE3}, {0x000D00, 0x000D01},
    {0x000D3B, 0x000D3C}, {0x000D41, 0x000D44}, {0x000D4D, 0x000D4D}, {0x000D62, 0x000D63},
    {0x000D81, 0x000D81}, {0x000DCA, 0x000DCA}, {0x000DD2, 0x000DD4}, {0x000DD6, 0x000DD6},
    {0x000E31, 0x000E31}, {0x000E34, 0x000E3A}, {0x000E47, 0x000E4E}, {0x000EB1, 0x000EB1},
    {0x000EB4, 0x000EBC}, {0x000EC8, 0x000ECD}, {0x000F18, 0x000F19}, {0x000F35, 0x000F35},
    {0x000F37, 0x000F37}, {0x000F39, 0x000F39}, {0x000F71, 0x000F7E}, {0x000F80, 0x000F84},
    {0x000F86, 0x000F87}, {0x000F8D, 0x000F97}, {0x000F99, 0x000FBC}, {0x000FC6, 0x000FC6},
    {0x00102D, 0x001030}, {0x001032, 0x001037}, {0x001039, 0x00103A}, {0x00103D, 0x00103E},
    {0x001058, 0x001059}, {0x00105E, 0x001060}, {0x001071, 0x001074}, {0x001082, 0x001082},
    {0x001085, 0x001086}, {0x00108D, 0x00108D}, {0x00109D, 0x00109D}, {0x00135D, 0x00135F},
    {0x001712, 0x001714}, {0x001732, 0x001733}, {0x001752, 0x001753}, {0x001772, 0x001773},
    {0x0017B4, 0x0017B5}, {0x0017B7, 0x0017BD}, {0x0017C6, 0x0017C6}, {0x0017C9, 0x0017D3},
    {0x0017DD, 0x0017DD}, {0x00180B, 0x00180D}, {0x00180F, 0x00180F}, {0x001885, 0x001886},
    {0x0018A9, 0x0018A9}, {0x001920, 0x001922}, {0x001927, 0x001928}, {0x001932, 0x001932},
    {0x001939, 0x00193B}, {0x001A17, 0x001A18}, {0x001A1B, 0x001A1B}, {0x001A56, 0x001A56},
    {0x001A58, 0x001A5E}, {0x001A60, 0x001A60}, {0x001A62, 0x001A62}, {0x001A65, 0x001A6C},
    {0x001A73, 0x001A7C}, {0x001A7F, 0x001A7F}, {0x001AB0, 0x001ABD}, {0x001ABE, 0x001ABE},
    {0x001ABF, 0x001ACE}, {0x001B00, 0x001B03}, {0x001B34, 0x001B34}, {0x001B36, 0x001B3A},
    {0x001B3C, 0x001B3C}, {0x001B42, 0x001B42}, {0x001B6B, 0x001B73}, {0x001B80, 0x001B81},
    {0x001BA2, 0x001BA5}, {0x001BA8, 0x001BA9}, {0x001BAB, 0x001BAD}, {0x001BE6, 0x001BE6},
    {0x001BE8, 0x001BE9}, {0x001BED, 0x001BED}, {0x001BEF, 0x001BF1}, {0x001C2C, 0x001C33},
    {0x001C36, 0x001C37}, {0x001CD0, 0x001CD2}, {0x001CD4, 0x001CE0}, {0x001CE2, 0x001CE8},
    {0x001CED, 0x001CED}, {0x001CF4, 0x001CF4}, {0x001CF8, 0x001CF9}, {0x001DC0, 0x001DFF},
    {0x0020D0, 0x0020DC}, {0x0020DD, 0x0020E0}, {0x0020E1, 0x0020E1}, {0x0020E2, 0x0020E4},
    {0x0020E5, 0x0020F0}, {0x002CEF, 0x002CF1}, {0x002D7F, 0x002D7F}, {0x002DE0, 0x002DFF},
    {0x00302A, 0x00302D}, {0x003099, 0x00309A}, {0x00A66F, 0x00A66F}, {0x00A670, 0x00A672},
    {0x00A674, 0x00A67D}, {0x00A69E, 0x00A69F}, {0x00A6F0, 0x00A6F1}, {0x00A802, 0x00A802},
    {0x00A806, 0x00A806}, {0x00A80B, 0x00A80B}, {0x00A825, 0x00A826}, {0x00A82C, 0x00A82C},
    {0x00A8C4, 0x00A8C5}, {0x00A8E0, 0x00A8F1}, {0x00A8FF, 0x00A8FF}, {0x00A926, 0x00A92D},
    {0x00A947, 0x00A951}, {0x00A980, 0x00A982}, {0x00A9B3, 0x00A9B3}, {0x00A9B6, 0x00A9B9},
    {0x00A9BC, 0x00A9BD}, {0x00A9E5, 0x00A9E5}, {0x00AA29, 0x00AA2E}, {0x00AA31, 0x00AA32},
    {0x00AA35, 0x00AA36}, {0x00AA43, 0x00AA43}, {0x00AA4C, 0x00AA4C}, {0x00AA7C, 0x00AA7C},
    {0x00AAB0, 0x00AAB0}, {0x00AAB2, 0x00AAB4}, {0x00AAB7, 0x00AAB8}, {0x00AABE, 0x00AABF},
    {0x00AAC1, 0x00AAC1}, {0x00AAEC, 0x00AAED}, {0x00AAF6, 0x00AAF6}, {0x00ABE5, 0x00ABE5},
    {0x00ABE8, 0x00ABE8}, {0x00ABED, 0x00ABED}, {0x00FB1E, 0x00FB1E}, {0x00FE00, 0x00FE0F},
    {0x00FE20, 0x00FE2F}, {0x0101FD, 0x0101FD}, {0x0102E0, 0x0102E0}, {0x010376, 0x01037A},
    {0x010A01, 0x010A03}, {0x010A05, 0x010A06}, {0x010A0C, 0x010A0F}, {0x010A38, 0x010A3A},
    {0x010A3F, 0x010A3F}, {0x010AE5, 0x010AE6}, {0x010D24, 0x010D27}, {0x010EAB, 0x010EAC},
    {0x010F46, 0x010F50}, {0x010F82, 0x010F85}, {0x011001, 0x011001}, {0x011038, 0x011046},
    {0x011070, 0x011070}, {0x011073, 0x011074}, {0x01107F, 0x011081}, {0x0110B3, 0x0110B6},
    {0x0110B9, 0x0110BA}, {0x0110C2, 0x0110C2}, {0x011100, 0x011102}, {0x011127, 0x01112B},
    {0x01112D, 0x011134}, {0x011173, 0x011173}, {0x011180, 0x011181}, {0x0111B6, 0x0111BE},
    {0x0111C9, 0x0111CC}, {0x0111CF, 0x0111CF}, {0x01122F, 0x011231}, {0x011234, 0x011234},
    {0x011236, 0x011237}, {0x01123E, 0x01123E}, {0x0112DF, 0x0112DF}, {0x0112E3, 0x0112EA},
    {0x011300, 0x011301}, {0x01133B, 0x01133C}, {0x011340, 0x011340}, {0x011366, 0x01136C},
    {0x011370, 0x011374}, {0x011438, 0x01143F}, {0x011442, 0x011444}, {0x011446, 0x011446},
    {0x01145E, 0x01145E}, {0x0114B3, 0x0114B8}, {0x0114BA, 0x0114BA}, {0x0114BF, 0x0114C0},
    {0x0114C2, 0x0114C3}, {0x0115B2, 0x0115B5}, {0x0115BC, 0x0115BD}, {0x0115BF, 0x0115C0},
    {0x0115DC, 0x0115DD}, {0x011633, 0x01163A}, {0x01163D, 0x01163D}, {0x01163F, 0x011640},
    {0x0116AB, 0x0116AB}, {0x0116AD, 0x0116AD}, {0x0116B0, 0x0116B5}, {0x0116B7, 0x0116B7},
    {0x01171D, 0x01171F}, {0x011722, 0x011725}, {0x011727, 0x01172B}, {0x01182F, 0x011837},
    {0x011839, 0x01183A}, {0x01193B, 0x01193C}, {0x01193E, 0x01193E}, {0x011943, 0x011943},
    {0x0119D4, 0x0119D7}, {0x0119DA, 0x0119DB}, {0x0119E0, 0x0119E0}, {0x011A01, 0x011A0A},
    {0x011A33, 0x011A38}, {0x011A3B, 0x011A3E}, {0x011A47, 0x011A47}, {0x011A51, 0x011A56},
    {0x011A59, 0x011A5B}, {0x011A8A, 0x011A96}, {0x011A98, 0x011A99}, {0x011C30, 0x011C36},
    {0x011C38, 0x011C3D}, {0x011C3F, 0x011C3F}, {0x011C92, 0x011CA7}, {0x011CAA, 0x011CB0},
    {0x011CB2, 0x011CB3}, {0x011CB5, 0x011CB6}, {0x011D31, 0x011D36}, {0x011D3A, 0x011D3A},
    {0x011D3C, 0x011D3D}, {0x011D3F, 0x011D45}, {0x011D47, 0x011D47}, {0x011D90, 0x011D91},
    {0x011D95, 0x011D95}, {0x011D97, 0x011D97}, {0x011EF3, 0x011EF4}, {0x016AF0, 0x016AF4},
    {0x016B30, 0x016B36}, {0x016F4F, 0x016F4F}, {0x016F8F, 0x016F92}, {0x016FE4, 0x016FE4},
    {0x01BC9D, 0x01BC9E}, {0x01CF00, 0x01CF2D}, {0x01CF30, 0x01CF46}, {0x01D167, 0x01D169},
    {0x01D17B, 0x01D182}, {0x01D185, 0x01D18B}, {0x01D1AA, 0x01D1AD}, {0x01D242, 0x01D244},
    {0x01DA00, 0x01DA36}, {0x01DA3B, 0x01DA6C}, {0x01DA75, 0x01DA75}, {0x01DA84, 0x01DA84},
    {0x01DA9B, 0x01DA9F}, {0x01DAA1, 0x01DAAF}, {0x01E000, 0x01E006}, {0x01E008, 0x01E018},
    {0x01E01B, 0x01E021}, {0x01E023, 0x01E024}, {0x01E026, 0x01E02A}, {0x01E130, 0x01E136},
    {0x01E2AE, 0x01E2AE}, {0x01E2EC, 0x01E2EF}, {0x01E8D0, 0x01E8D6}, {0x01E944, 0x01E94A},
    {0x0E0100, 0x0E01EF},
}};
}  // namespace arg_router::utility::utf8
