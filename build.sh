#!/bin/sh

function main() {
    set -xe
    cmake -B ./build -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE
    cmake --build ./build/ -j8
}

main
