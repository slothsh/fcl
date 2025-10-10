# Test Data
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Tests
add_executable(ConfLexer.test ConfLexer.test.cpp)
target_link_libraries(ConfLexer.test PRIVATE LibTrader)
target_link_libraries(ConfLexer.test PRIVATE Catch2::Catch2WithMain)
target_compile_options(ConfLexer.test PRIVATE -include "lib/Core.hpp")

catch_discover_tests(ConfLexer.test)
