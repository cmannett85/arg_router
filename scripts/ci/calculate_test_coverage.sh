#!/usr/bin/env bash

### Copyright (C) 2022 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

# Run from inside the scripts/ci folder.
# - First argument is the top-level build directory and is required
# - Second argument is optional, and defined the gcov tool (defaults to gcov-11)
if [ -z "$1" ]; then
    echo "Must pass build directory"
    exit 1
fi

BUILD_DIR=$1
DEFAULT_TOOL_NAME="llvm-gcov-14"
TOOL=${2:-"${BUILD_DIR}/${DEFAULT_TOOL_NAME}"}
SKIP_UPDATE="${SKIP_COVERAGE_UPDATE:-1}"

SRC_PATH=${PWD}
OLD_COVERAGE="$(cat ./old_coverage)"

# llvm-cov needs a gcov arg to be used with lcov, but lcov doesn't accept it, so we need to create
# a script that emulates it
cd ${BUILD_DIR}
if [[ "${TOOL}" == *"${DEFAULT_TOOL_NAME}"* ]]; then
    rm ${TOOL}
    SCRIPT_TEXT="#!/usr/bin/env bash\nexec llvm-cov-14 gcov \"\$@\""
    echo -e ${SCRIPT_TEXT} >> ${TOOL}
    chmod a+x ${TOOL}
fi

lcov -d ./test/CMakeFiles/arg_router_test_coverage.dir    \
     -c -o temp.info --gcov-tool ${TOOL}
lcov --remove temp.info "/usr/include/*" \
     --remove temp.info "*/vcpkg_installed/*" \
     --remove temp.info "*/arg_router/test/*" \
     --output-file arg_router.info

NEW_COVERAGE="$(lcov --summary arg_router.info | awk 'NR==3 {print $2+0}')"
DIFF=$(echo "$NEW_COVERAGE - $OLD_COVERAGE" | bc);
echo "New coverage: ${NEW_COVERAGE}%, previous coverage: ${OLD_COVERAGE}%, diff: ${DIFF}"
if (( $(echo "${DIFF} < -1.0" | bc -l) )); then
    echo "Coverage drop too great (>1%)"
    exit 1
fi

if [ "${SKIP_UPDATE}" = "0" ]; then
    # Write the new value back to the cache
    echo ${NEW_COVERAGE} > ${SRC_PATH}/old_coverage
fi
