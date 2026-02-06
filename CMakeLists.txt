cmake_minimum_required(VERSION 4.0.3...4.1.1)

include(cmake/Utilities.cmake)
include(cmake/Toolchain.cmake)

initialize_toolchain()

project(fcl VERSION 0.0.0 LANGUAGES CXX)

add_subdirectory(src/)
add_subdirectory(tests/)
