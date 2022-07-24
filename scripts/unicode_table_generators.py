#!/usr/bin/env python3

# Copyright (C) 2022 by Camden Mannett.  All rights reserved.

from io import StringIO
from enum import Enum, unique
import requests
import re

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

    def add_to_code_point(self, cp):
        abbrv_value = self.value[0] & 0xF
        return (cp & 0x1FFFFF) | (abbrv_value << CODE_POINT_DATA_OFFSET)


def download(url):
    return requests.get(url).text


def extract_version(content):
    line_end = content.find('\n')
    if line_end == -1:
        raise RuntimeError('Cannot find first line')

    line = content[0:line_end]
    rc = re.compile(r'\d+\.\d+\.\d+')
    version = rc.search(line)
    if version is None:
        raise RuntimeError('Cannot find version')

    print('\tVersion: {}'.format(version.group(0)))


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


def extract_code_points(abbrvs, content):
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
            result.append([start, start])
        else:
            start = int(line[0:range_split], 16)
            end = int(line[range_split+len(RANGE_DIVIDER):], 16)
            result.append([start, end])

    return result


def sort_code_points(cps):
    return sorted(cps, key=lambda cp_range: cp_range[0] & 0x1FFFFF)


def print_code_points(cps):
    for cp_range in cps:
        print('    {{0x{:X}, 0x{:X}}},'.format(cp_range[0], cp_range[1]))


def run(url, abbrvs, varname):
    print('\tURL: {}'.format(url))
    contents = download(url)
    extract_version(contents)
    cps = extract_code_points(abbrvs, contents)
    cps = sort_code_points(cps)
    print('constexpr auto {} = std::array<code_point::range, {}>{{{{'.format(
        varname, len(cps)))
    print_code_points(cps)
    print('}};')


def run_grapheme_cluster_break_property():
    # First grab all the break properties
    url = 'https://www.unicode.org/Public/UCD/latest/ucd/auxiliary/' \
          'GraphemeBreakProperty.txt'
    print('\tURL: {}'.format(url))
    contents = download(url)
    extract_version(contents)

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
        if abbrv_value is None:
            raise RuntimeError('Unknown abbreviation: {}'.format(abbrv))

        # The code point could be singular or a range.  Each code point has
        # the 21 bits of value data followed by 4 bits for the abbreviation
        # type
        line = line[0:sc_pos].strip()
        range_split = line.find(RANGE_DIVIDER)
        if range_split == -1:
            start = abbrv_value.add_to_code_point(int(line, 16))
            result.append([start, start])
        else:
            start = abbrv_value.add_to_code_point(int(line[0:range_split], 16))
            end = abbrv_value.add_to_code_point(
                int(line[range_split+len(RANGE_DIVIDER):], 16))
            result.append([start, end])

    # Unfortuantely this doesn't include extended pictographic, so get that
    # separately
    url = 'https://www.unicode.org/Public/14.0.0/ucd/emoji/emoji-data.txt'
    print('\tURL: {}'.format(url))
    contents = download(url)
    extract_version(contents)

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
            start = abbrv_value.add_to_code_point(int(line, 16))
            result.append([start, start])
        else:
            start = abbrv_value.add_to_code_point(int(line[0:range_split], 16))
            end = abbrv_value.add_to_code_point(
                int(line[range_split+len(RANGE_DIVIDER):], 16))
            result.append([start, end])

    cps = sort_code_points(result)
    print('constexpr auto grapheme_cluster_break_table = '
          'std::array<code_point::range, {}>{{{{'.format(len(cps)))
    print_code_points(cps)
    print('}};')


if __name__ == '__main__':
    print('Whitespace:')
    run('http://www.unicode.org/Public/UNIDATA/PropList.txt',
        ['White_Space'],
        'whitespace_table')

    print('\nDouble width:')
    run('http://www.unicode.org/Public/UNIDATA/EastAsianWidth.txt',
        ['W', 'F'],
        'full_width_table')

    print('\nZero width:')
    run('http://www.unicode.org/Public/UNIDATA/extracted/'
        'DerivedGeneralCategory.txt',
        ['Mn', 'Me'],
        'zero_width_table')

    # The grapheme cluster break property table is done separately as this
    # table is a different structure to the others
    print('\nGrapheme cluster break property table')
    run_grapheme_cluster_break_property()
