find_package(Catch2 3 REQUIRED)

enable_testing()
include(CTest)
include(Catch)

add_subdirectory(unit/)
