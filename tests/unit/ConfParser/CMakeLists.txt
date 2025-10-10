# Test Data
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Tests
add_executable(ConfParser.test ConfParser.test.cpp)
target_link_libraries(ConfParser.test PRIVATE LibTrader)
target_link_libraries(ConfParser.test PRIVATE Catch2::Catch2WithMain)
target_compile_options(ConfParser.test PRIVATE -include "lib/Core.hpp")

catch_discover_tests(ConfParser.test)
