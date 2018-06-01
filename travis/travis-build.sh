#! /bin/bash

# Copyright 2018 TheAssassin
# Licensed under the terms of the MIT license.

set -x
set -e

# use RAM disk if possible
if [ -d /dev/shm ] && [ "$CI" != "" ]; then
    TEMP_BASE=/dev/shm
else
    TEMP_BASE=/tmp
fi

BUILD_DIR=$(mktemp -d -p "$TEMP_BASE" AppImageUpdate-build-XXXXXX)

cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}

trap cleanup EXIT

# store repo root as variable
REPO_ROOT=$(readlink -f $(dirname $(dirname $0)))
OLD_CWD=$(readlink -f .)

pushd "$BUILD_DIR"

cmake "$REPO_ROOT"

make -j$(nproc)

# move AppImages to old cwd
mv *gs-plugin-appimage*.so* "$OLD_CWD"/

popd
