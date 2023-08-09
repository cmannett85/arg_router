#!/usr/bin/env bash

### Copyright (C) 2023 by Camden Mannett.
### Distributed under the Boost Software License, Version 1.0.
### (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

# If a REF hash is provided, check that it is valid
ref_hash=""
if [ ! -z "$1" ]; then
    if [[ ! "$1" =~ ^[0-9a-f]{40}$ ]]; then
        echo "Invalid REF hash"
        exit 1
    fi
    ref_hash="$1"
fi

root_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/../

# Use the version in the Doxyfile
version=`sed -n -e 's/^PROJECT_NUMBER         = \(.*\)$/\1/p' ${root_dir}/docs/Doxyfile`

# Create a new branch at vcpkg HEAD.  Wipe out any previous changes
cd ${root_dir}/external/vcpkg
git checkout master
git reset --hard origin/master
git fetch
git checkout -b vcpkg_${version}

# Update the versions
sed -ri "s/^  \"version\": \"[0-9]+\.[0-9]+\.[0-9]+\",$/  \"version\": \"${version}\",/" ports/arg-router/vcpkg.json
./vcpkg format-manifest ports/arg-router/vcpkg.json
git add ports/arg-router/vcpkg.json
git commit -m "[arg-router] Update to v${version}"
./vcpkg x-add-version arg-router

# Manually build the test package, this will fail due to an incorrect SHA but we need it to for
# vcpkg to give us the correct one
cd ${root_dir}/ci/vcpkg_test_project
mkdir build
cd ./build
set +e
sha_output=`cmake -G "Ninja" -DCMAKE_CXX_STANDARD=20 -B . -S .. 2>&1`
set -e

# Update the SHA value in the portfile, and the ref if this run is for a test build
cd ${root_dir}/external/vcpkg
new_sha=`echo "$sha_output" | sed -n -e 's/^Actual hash: \(.*\)$/\1/p'`
sed -ri "s/^    SHA512 [0-9a-f]{128}$/    SHA512 ${new_sha}/" ports/arg-router/portfile.cmake

# If this run is for a test build, then update the REF entry to the specified hash
if [ ! -z "$ref_hash" ]; then
    sed -ri "s/^    REF v.*$/    REF ${ref_hash}/" ports/arg-router/portfile.cmake
fi

# Re-run the build to confirm
cd ${root_dir}/ci/vcpkg_test_project/build
cmake -G "Ninja" -DCMAKE_CXX_STANDARD=20 -B . -S ..
cmake --build .
cd ..
rm -rf ./build
