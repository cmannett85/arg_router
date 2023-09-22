#!/usr/bin/env python3

# Copyright (C) 2022-2023 by Camden Mannett.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

from io import StringIO
from enum import Enum, unique
import re
import requests
import argparse

RANGE_DIVIDER = '..'
COMMENT = '#'
CODE_POINT_DATA_OFFSET = 21


@unique
class GraphemeClusterBreak(Enum):
    # ANY = 0, not used in the script but is in the C++
    CR = (1, 'CR')
    LF = (2, 'LF')
    CONTROL = (3, 'Control')
    EXTEND = (4, 'Extend')
    ZWJ = (5, 'ZWJ')
    REGIONAL_INDICATOR = (6, 'Regional_Indicator')
    PREPEND = (7, 'Prepend')
    SPACING_MARK = (8, 'SpacingMark')
    L = (9, 'L')
    V = (10, 'V')
    T = (11, 'T')
    LV = (12, 'LV')
    LVT = (13, 'LVT')
    EXTENDED_PICTOGRAPHIC = (14, 'Extended_Pictographic')


@unique
class LineBreak(Enum):
    # Any = 0, not used in the script but is in the C++
    # v14.0.0
    AL = 1
    BA = 2
    BB = 3
    B2 = 4
    BK = 5
    CB = 6
    CL = 7
    CM = 8
    CP = 9
    CR = 10
    EB = 11
    EM = 12
    EX = 13
    GL = 14
    H2 = 15
    H3 = 16
    HY = 17
    ID = 18
    HL = 19
    IN = 20
    IS = 21
    JL = 22
    JT = 23
    JV = 24
    LF = 25
    NL = 26
    NS = 27
    NU = 28
    OP = 29
    PO = 30
    PR = 31
    QU = 32
    RI = 33
    SP = 34
    SY = 35
    WJ = 36
    ZW = 37
    ZWJ = 38
    # v15.1.0
    AK = 39
    AP = 40
    AS = 41
    VI = 42
    VF = 43


def download(url):
    return requests.get(url).text


def get_abbreviation(line):
    # Skip empty or any comment lines
    if (len(line) == 0) or (line[0] == COMMENT):
        return None

    sc_pos = line.find(';')
    if sc_pos == -1:
        raise RuntimeError('Cannot find abbreviation divider')
    comment_pos = line.find(COMMENT, sc_pos)
    if comment_pos == -1:
        comment_pos = len(line)

    return (sc_pos, line[sc_pos+1:comment_pos].strip())


def create_range(line):
    range_split = line.find(RANGE_DIVIDER)
    if range_split == -1:
        start = int(line, 16)
        return [start, start]
    else:
        start = int(line[0:range_split], 16)
        end = int(line[range_split+len(RANGE_DIVIDER):], 16)
        return [start, end]


def limit_to_ascii(start, end, ascii_only):
    if not ascii_only:
        return [start, end]

    if end <= 0x7f:
        return [start, end]
    elif start > 0x7f:
        return []
    else:
        return [start, 0x7f]


def extract_code_points(abbrvs, content, ascii_only):
    # For each line split take the section between the semicolon and comment
    # start, and remove trailing whitespace.  This will give just the type
    # abbreviation
    result = []
    for line in StringIO(content):
        line = line.strip()
        abbrv = get_abbreviation(line)
        if (abbrv is None) or (abbrv[1] not in abbrvs):
            continue

        # The code point could be singular or a range
        sc_pos = abbrv[0]
        line = line[0:sc_pos].strip()
        range_split = line.find(RANGE_DIVIDER)
        if range_split == -1:
            start = int(line, 16)
            result.append(limit_to_ascii(start, start, ascii_only))
        else:
            start = int(line[0:range_split], 16)
            end = int(line[range_split+len(RANGE_DIVIDER):], 16)
            result.append(limit_to_ascii(start, end, ascii_only))

    return result


def sort_code_points(cps):
    return sorted([rng for rng in cps if len(rng) > 1], key=lambda cp_range: cp_range[0])


def print_code_points(cps):
    for cp_range in cps:
        if len(cp_range) == 3:
            print('    {{0x{:06X}, 0x{:06X}, 0x{:06X}}},'.format(cp_range[0],
                                                                 cp_range[1],
                                                                 cp_range[2]))
        else:
            print('    {{0x{:06X}, 0x{:06X}}},'.format(
                cp_range[0], cp_range[1]))


def run(url, abbrvs, varname, ascii_only):
    print('\tURL: {}'.format(url))
    contents = download(url)
    cps = extract_code_points(abbrvs, contents, ascii_only)
    cps = sort_code_points(cps)
    print('constexpr auto {} = std::array<code_point::range, {}>{{{{'.format(
        varname, len(cps)))
    print_code_points(cps)
    print('}};')


def run_grapheme_cluster_break(version, ascii_only):
    # First grab all the break properties
    url = f'https://www.unicode.org/Public/{version}/ucd/auxiliary/GraphemeBreakProperty.txt'
    print('\tURL: {}'.format(url))
    contents = download(url)

    result = []
    for line in StringIO(contents):
        line = line.strip()

        abbrv = get_abbreviation(line)
        if abbrv is None:
            continue
        sc_pos = abbrv[0]
        abbrv = abbrv[1]

        abbrv_value = None
        for member in list(GraphemeClusterBreak):
            if member.value[1] == abbrv:
                abbrv_value = member
                break
        if abbrv_value is None:
            raise RuntimeError('Unknown abbreviation: {}'.format(abbrv))

        # The code point could be singular or a range.  Each code point has
        # the 21 bits of value data followed by 4 bits for the abbreviation
        # type
        line = line[0:sc_pos].strip()
        range_split = line.find(RANGE_DIVIDER)
        if range_split == -1:
            start = int(line, 16)
            result.append(limit_to_ascii(start, start, ascii_only) + [abbrv_value.value[0]])
        else:
            start = int(line[0:range_split], 16)
            end = int(line[range_split+len(RANGE_DIVIDER):], 16)
            result.append(limit_to_ascii(start, end, ascii_only) + [abbrv_value.value[0]])

    # Unfortuantely this doesn't include extended pictographic, so get that
    # separately
    url = f'https://www.unicode.org/Public/{version}/ucd/emoji/emoji-data.txt'
    print('\tURL: {}'.format(url))
    contents = download(url)

    for line in StringIO(contents):
        line = line.strip()

        abbrv = get_abbreviation(line)
        if abbrv is None:
            continue
        sc_pos = abbrv[0]
        abbrv = abbrv[1]

        if abbrv != GraphemeClusterBreak.EXTENDED_PICTOGRAPHIC.value[1]:
            continue
        abbrv_value = GraphemeClusterBreak.EXTENDED_PICTOGRAPHIC

        # The code point could be singular or a range.  Each code point has
        # the 21 bits of value data followed by 4 bits for the abbreviation
        # type
        line = line[0:sc_pos].strip()
        range_split = line.find(RANGE_DIVIDER)
        if range_split == -1:
            start = int(line, 16)
            result.append(limit_to_ascii(start, start, ascii_only) + [abbrv_value.value[0]])
        else:
            start = int(line[0:range_split], 16)
            end = int(line[range_split+len(RANGE_DIVIDER):], 16)
            result.append(limit_to_ascii(start, end, ascii_only) + [abbrv_value.value[0]])

    cps = sort_code_points(result)
    print('constexpr auto grapheme_cluster_break_table = '
          'std::array<code_point::range, {}>{{{{'.format(len(cps)))
    print_code_points(cps)
    print('}};')


def line_break(version, ascii_only):
    # First grab all the break properties
    url = f'https://www.unicode.org/Public/{version}/ucd/LineBreak.txt'
    print('\tURL: {}'.format(url))
    contents = download(url)

    result = []
    for line in StringIO(contents):
        line = line.strip()

        abbrv = get_abbreviation(line)
        if abbrv is None:
            continue
        sc_pos = abbrv[0]
        abbrv = abbrv[1]

        # Some values need resolving manually because they map to different classes
        abbrv_value = None
        if abbrv == 'AI' or abbrv == 'SG' or abbrv == 'XX':
            abbrv_value = LineBreak.AL
        elif abbrv == 'CJ':
            abbrv_value = LineBreak.NS
        elif abbrv == 'SA':
            # Extract the general category
            c_pos = line.find(COMMENT, sc_pos)
            if c_pos == -1:
                raise RuntimeError(
                    'Cannot find comment divider, to extract general category')

            general_category = line[c_pos+2:c_pos+4]
            if general_category == 'Mn' or general_category == 'Mc':
                abbrv_value = LineBreak.CM
            else:
                abbrv_value = LineBreak.AL
        else:
            for member in list(LineBreak):
                if member.name == abbrv:
                    abbrv_value = member
                    break
            if abbrv_value is None:
                raise RuntimeError('Unknown abbreviation: {}'.format(abbrv))

        # The code point could be singular or a range.  Each code point has
        # the 21 bits of value data followed by 6 bits for the abbreviation
        # type
        line = line[0:sc_pos].strip()
        range_split = line.find(RANGE_DIVIDER)
        if range_split == -1:
            start = int(line, 16)
            result.append(limit_to_ascii(start, start, ascii_only) + [abbrv_value.value])
        else:
            start = int(line[0:range_split], 16)
            end = int(line[range_split+len(RANGE_DIVIDER):], 16)
            result.append(limit_to_ascii(start, end, ascii_only) + [abbrv_value.value])

    cps = sort_code_points(result)
    print('constexpr auto line_break_table = std::array<code_point::range, {}>{{{{'.format(len(cps)))
    print_code_points(cps)
    print('}};')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generates the Unicode tables for arg_router')
    parser.add_argument('--unicode-version', default='latest')
    parser.add_argument('--ascii-only', action='store_true')
    args = parser.parse_args()

    version = args.unicode_version
    if version == 'latest':
        version = 'UCD/latest'

    print('Whitespace:')
    run(f'http://www.unicode.org/Public/{version}/ucd/PropList.txt',
        ['White_Space'],
        'whitespace_table',
        args.ascii_only)

    print('\nDouble width:')
    run(f'http://www.unicode.org/Public/{version}/ucd/EastAsianWidth.txt',
        ['W', 'F'],
        'double_width_table',
        args.ascii_only)

    print('\nZero width:')
    run(f'http://www.unicode.org/Public/{version}/ucd/extracted/DerivedGeneralCategory.txt',
        ['Mn', 'Me'],
        'zero_width_table',
        args.ascii_only)

    # The following property tables are done separately as they are a different structure to the
    # others
    print('\nGrapheme cluster break table')
    run_grapheme_cluster_break(version, args.ascii_only)

    print('\nLine break table')
    line_break(version, args.ascii_only)
