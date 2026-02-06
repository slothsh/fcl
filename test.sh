#!/bin/sh -xeu

./build.sh
ctest --output-on-failure --test-dir ./build/tests
