#! /usr/bin/env bash

### Copyright (C) 2022 by Camden Mannett.  All rights reserved.

# This will be called from within the docker container if the compiler is invoked by cmake and is
# set to one of our compiler scripts.  In that scenario we just invoke directly
if command -v docker &> /dev/null
then
    SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
    REPO_PATH=${SCRIPT_DIR}/../../..
    REPO_PARENT_PATH=${REPO_PATH}/..

    docker run --rm \
        -v ${REPO_PARENT_PATH}:${REPO_PARENT_PATH} \
        -v ${HOME}/.cache/vcpkg/archives:${HOME}/.cache/vcpkg/archives \
        -v ${HOME}/.cache/ccache:${HOME}/.cache/ccache \
        -v /etc/passwd:/etc/passwd:ro \
        -v /etc/group:/etc/group:ro \
        -u $(id -u):$(id -g) \
        -w ${REPO_PATH} \
        arg_router "$@"
else
    $@
fi

