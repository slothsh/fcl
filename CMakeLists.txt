cmake_minimum_required(VERSION 4.0.3...4.1.1)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_COMPILER /opt/homebrew/opt/llvm/bin/clang)
set(CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm/bin/clang++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(trader VERSION 0.8.0 LANGUAGES CXX)

add_subdirectory(src/)
# add_subdirectory(tests/)
