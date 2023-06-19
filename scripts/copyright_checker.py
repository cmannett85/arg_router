#!/usr/bin/env python3

# Copyright (C) 2022-2023 by Camden Mannett.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

import sys
import glob
import os
import re
import datetime
import argparse
import fnmatch

LANGUAGE_COMMENT_ENDS = [('*.sh', '###'),
                         ('*.cpp', '//'),
                         ('*.hpp', '//'),
                         ('*.py', '#'),
                         ('*.cmake', '###'),
                         ('*CMakeLists.txt', '###'),
                         ('*.doxy', '//'),
                         ('*pre-commit', '#')]
SKIP_PATHS = ['external' + os.sep,
              'test' + os.sep + 'death_test',
              'build' + os.sep,         # VSCode
              'out' + os.sep,           # Visual Studio
              'install' + os.sep,       # CI
              'package_build' + os.sep,  # CI
              'download' + os.sep]      # CI
THIS_YEAR = datetime.datetime.now().year

SECOND_COPYRIGHT_LINE = ' Distributed under the Boost Software License, Version 1.0.\n'
THIRD_COPYRIGHT_LINE = ' (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)\n'


def skip_file(file):
    for skip_path in SKIP_PATHS:
        if skip_path in file:
            return True

    for lang in LANGUAGE_COMMENT_ENDS:
        suffix = lang[0]
        if suffix.startswith('*'):
            suffix = suffix[1:]

        if file.endswith(suffix):
            return False

    return True


def find_re_in_lines(lines, prog):
    for line in lines:
        m = prog.search(line)
        if m:
            return m

    return None


def find_copyright(line_prefix, lines):
    prog = re.compile(
        r'Copyright \(C\) (\d{4}|\d{4}-\d{4}) by Camden Mannett\.')
    for i in range(len(lines)):
        m = prog.search(lines[i])
        if m and ((i+2) < len(lines)) and (lines[i+1] == (line_prefix + SECOND_COPYRIGHT_LINE)) and \
                (lines[i+2] == (line_prefix + THIRD_COPYRIGHT_LINE)):
            return m

    return None


def build_copyright(line_prefix):
    return line_prefix + ' Copyright (C) ' + str(THIS_YEAR) + \
        ' by Camden Mannett.\n' + \
        line_prefix + SECOND_COPYRIGHT_LINE + \
        line_prefix + THIRD_COPYRIGHT_LINE


def presence_checker(args):
    root_dir = args.dir

    print('Checking copyrights, starting at ' + root_dir + '...')

    missing = []

    for t in LANGUAGE_COMMENT_ENDS:
        for file in glob.iglob(os.path.join(root_dir, '**/' + t[0]), recursive=True):
            if skip_file(file):
                continue

            data = None
            with open(file, 'r', encoding='utf-8') as f:
                data = f.readlines()

            if not find_copyright(t[1], data):
                missing.append(file)

    if len(missing) != 0:
        print('Failed to find copyright notice in:')
        for file in missing:
            print('\t' + file)
        sys.exit(1)


def date_checker(args):
    for file in args.files:
        if not os.path.exists(file):
            print('Skipping non-existent file: ' + file)
            continue

        # Find the matching line prefix
        prefix = None
        for t in LANGUAGE_COMMENT_ENDS:
            if fnmatch.fnmatch(file, t[0]):
                prefix = t[1]
                break

        if (prefix == None) or skip_file(file):
            continue

        with open(file, 'r', encoding="utf-8") as f:
            data = f.readlines()
            m = find_copyright(prefix, data)
            if not m:
                # Shouldn't get here as it _should_ have already been checked as a part of the
                # build process, but you never know...
                print('Failed to find copyright notice in: ' + file)
                sys.exit(1)

            latest_file_year = int(max(m.group(1).split('-')))
            if latest_file_year != THIS_YEAR:
                print('Incorrect copyright year in this file ' +
                      '(' + str(latest_file_year) + '): ' + file)
                sys.exit(1)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Checks for the presence of copyright notices in the ' +
                    'given directory.')
    subparsers = parser.add_subparsers()

    presence_parser = subparsers.add_parser(
        'presence',
        help='Check for the presence for, or generate, copyright notices')
    presence_parser.add_argument('dir', help='Directory to recurse through and check',
                                 default='.')
    presence_parser.set_defaults(func=presence_checker)

    date_parser = subparsers.add_parser(
        'date',
        help='Checks the date in a copyright notice is correct for a list of files')
    date_parser.add_argument('files', help='List of files to check', nargs='*')
    date_parser.set_defaults(func=date_checker)

    args = parser.parse_args()
    args.func(args)
