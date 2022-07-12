#!/usr/bin/env python3

### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

from io import StringIO
import requests, re

RANGE_DIVIDER = '..'
COMMENT = '#'

def download(url):
    return requests.get(url).text


def extract_version(content):
    line_end = content.find('\n')
    if line_end == -1:
        raise RuntimeError('Cannot find first line')

    line = content[0:line_end]
    rc = re.compile(r'\d+\.\d+\.\d+')
    version = rc.search(line)
    if version == None:
        raise RuntimeError('Cannot find version')

    print('\tVersion: {}'.format(version.group(0)))

def extract_code_points(abbrvs, content):
    # For each line split take the section between the semicolon and comment start, and remove
    # trailing whitespace.  This will give just the type abbreviation
    result = []
    for line in StringIO(content):
        line = line.strip()

        # Skip any comment lines
        if (len(line) == 0) or (line[0] == COMMENT):
            continue

        sc_pos = line.find(';')
        if sc_pos == -1:
            raise RuntimeError('Cannot find abbreviation divider')
        comment_pos = line.find(COMMENT, sc_pos)
        if comment_pos == -1:
            comment_pos = len(line)

        abbrv = line[sc_pos+1:comment_pos].strip()
        if abbrv not in abbrvs:
            continue

        # The code point could be singular or a range
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
    return sorted(cps, key=lambda cp_range: cp_range[0])


def print_code_points(cps):
    for cp_range in cps:
        print('    {{0x{:X}, 0x{:X}}},'.format(cp_range[0], cp_range[1]))


def run(url, abbrvs, varname):
    print('\tURL: {}'.format(url))
    contents = download(url)
    extract_version(contents)
    cps = extract_code_points(abbrvs, contents)
    cps = sort_code_points(cps)
    print('constexpr auto {} = std::array<code_point_range, {}>{{{{'.format(varname, len(cps)))
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
    run('http://www.unicode.org/Public/UNIDATA/extracted/DerivedGeneralCategory.txt',
        ['Mn', 'Me'],
        'zero_width_table')
