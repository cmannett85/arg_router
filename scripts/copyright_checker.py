#!/usr/bin/env python3

### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

import sys, glob, os, re, datetime, argparse

SKIP_PATHS = ['vcpkg/',
              'test/death_test/main.cpp',
              'docs/',
              'build/',
              '.github/',
              '.clang-format',
              '.gitignore']
THIS_YEAR = datetime.datetime.now().year

def skip_file(file):
    skip = False
    for skip_path in SKIP_PATHS:
        if skip_path in file:
            skip = True
            break

    return skip


def find_re_in_lines(lines, prog):
    for line in lines:
        m = prog.search(line)
        if m:
            return m

    return None


def find_copyright(lines):
    prog = re.compile(r'Copyright \(C\) (\d{4}|\d{4}-\d{4}) by Camden Mannett\.  All rights reserved\.')
    return find_re_in_lines(lines, prog)


def find_shebang(lines):
    prog = re.compile(r'^#!\/')
    return find_re_in_lines(lines, prog)


def build_copyright():
    return ' Copyright (C) ' + str(THIS_YEAR) + \
           ' by Camden Mannett.  All rights reserved. '


def presence_checker(args):
    LANGUAGE_COMMENT_ENDS = [('*.sh', '###', '\n'),
                             ('*.cpp', '/*', '*/\n'),
                             ('*.hpp', '/*', '*/\n'),
                             ('*.py', '###', '\n'),
                             ('*.cmake', '###', '\n'),
                             ('CMakeLists.txt', '###', '\n')]

    root_dir = args.dir
    generate = args.generate

    print('Checking copyrights, starting at ' + root_dir + '...')

    missing = []

    for t in LANGUAGE_COMMENT_ENDS:
        for file in glob.iglob(os.path.join(root_dir, '**/' + t[0]), recursive=True):
            if skip_file(file):
                continue

            data = None
            with open(file, 'r') as f:
                data = f.readlines()

            if not find_copyright(data):
                if generate:
                    if find_shebang(data):
                        data.insert(1, '\n' + t[1] + build_copyright() + t[2])
                    else:
                        data.insert(0, t[1] + build_copyright() + t[2] + '\n')

                    with open(file, 'w') as f:
                        f.writelines(data)
                else:
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

        if skip_file(file):
            continue

        with open(file, 'r') as f:
                data = f.readlines()
                m = find_copyright(data)
                if not m:
                    # Shouldn't get here as it _should_ have already been
                    # checked as a part of the build process, but you never
                    # know...
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
                    'given directory, and can generate them too.')
    subparsers = parser.add_subparsers()

    presence_parser = subparsers.add_parser(
        'presence',
        help='Check for the presence for, or generate, copyright notices')
    presence_parser.add_argument('dir', help='Directory to recurse through and check',
                        default='.')
    presence_parser.add_argument('-g', '--generate',
                        help='Generate copyright notices if necessary',
                        action='store_true')
    presence_parser.set_defaults(func=presence_checker)

    date_parser = subparsers.add_parser(
        'date',
        help='Checks the date in a copyright notice is correct for a list of files')
    date_parser.add_argument('files', help='List of files to check', nargs='*')
    date_parser.set_defaults(func=date_checker)

    args = parser.parse_args()
    args.func(args)
