#!/usr/bin/env bash

### Copyright (C) 2022-2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

ESCAPE=0
if [ "$1" == "-e" ]; then
    ESCAPE=1
fi

# Creates a shields.io badge URL for the unit test coverage
COVERAGE="$(cat ./ci/old_coverage)"
COLOUR="brightgreen"
if (( $(echo "$COVERAGE < 50" | bc -l) )); then
    COLOUR="red"
elif (( $(echo "$COVERAGE < 60" | bc -l) )); then
    COLOUR="orange"
elif (( $(echo "$COVERAGE < 70" | bc -l) )); then
    COLOUR="yellow"
elif (( $(echo "$COVERAGE < 80" | bc -l) )); then
    COLOUR="yellowgreen"
elif (( $(echo "$COVERAGE < 90" | bc -l) )); then
    COLOUR="green"
fi

if [ "$ESCAPE" == "0" ]; then
    echo "https://img.shields.io/badge/Unit_Test_Coverage-${COVERAGE}%25-${COLOUR}"
else
    echo "https:\/\/img\.shields\.io\/badge\/Unit_Test_Coverage-${COVERAGE}%25-${COLOUR}"
fi
