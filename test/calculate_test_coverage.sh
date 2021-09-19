#!/usr/bin/env bash

# Run from inside the test folder.
# - First argument is the top-level build directory and is required
# - Second argument is optional, and defined the gcov tool (defaults to gcov-11)
if [ -z "$1" ]; then
    echo "Must pass build directory"
    exit 1
fi

BUILD_DIR=$1
GCOV=${2:-gcov-11}
SKIP_UPDATE="${SKIP_COVERAGE_UPDATE:-1}"

SRC_PATH=${PWD}
OLD_COVERAGE="$(cat ./old_coverage)"

cd ${BUILD_DIR}
lcov -d ./CMakeFiles/arg_router_coverage.dir/src           \
     -d ./test/CMakeFiles/arg_router_test_coverage.dir    \
     -c -o temp.info --rc geninfo_gcov_tool=${GCOV}
lcov --remove temp.info "/usr/include/*" \
     --remove temp.info "${BUILD_DIR}/vcpkg_installed/*" \
     --remove temp.info "${SRC_PATH}/*" \
     --output-file arg_router.info

NEW_COVERAGE="$(lcov --summary arg_router.info | awk 'NR==3 {print $2+0}')"
DIFF=$(echo "$NEW_COVERAGE - $OLD_COVERAGE" | bc);
echo "New coverage: ${NEW_COVERAGE}%, previous coverage: ${OLD_COVERAGE}%, diff: ${DIFF}"
if (( $(echo "${DIFF} < -1.0" | bc -l) )); then
    echo "\tCoverage drop too great (>1%)"
    exit 1
fi

if [ "${SKIP_UPDATE}" = "0" ]; then
    # Write the new value back to the cache
    echo ${NEW_COVERAGE} > ${SRC_PATH}/old_coverage
fi
