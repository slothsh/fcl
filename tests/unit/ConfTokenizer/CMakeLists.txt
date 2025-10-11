# Test Data
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Tests
add_executable(ConfTokenizer.test ConfTokenizer.test.cpp)
target_link_libraries(ConfTokenizer.test PRIVATE LibTrader)
target_link_libraries(ConfTokenizer.test PRIVATE Catch2::Catch2WithMain)
target_compile_options(ConfTokenizer.test PRIVATE -include "lib/Core.hpp")

catch_discover_tests(ConfTokenizer.test)
