#! /usr/bin/env bash

### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
${SCRIPT_DIR}/base.sh clang++-9 "$@"
