#!/bin/sh -xeu

cmake -B ./build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE
cmake --build ./build/ -j8
