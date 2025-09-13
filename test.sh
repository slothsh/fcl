#!/bin/sh

function main() {
    set -xe
    ./build.sh
    ctest --output-on-failure --test-dir ./build/tests
}

main
