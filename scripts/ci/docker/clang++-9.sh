#! /usr/bin/env bash

### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
${SCRIPT_DIR}/base.sh clang++-9 "$@"
