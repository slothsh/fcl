# Test Data
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Tests
add_executable(ConfLoader.test ConfLoader.test.cpp)
target_link_libraries(ConfLoader.test PRIVATE LibTrader)
target_link_libraries(ConfLoader.test PRIVATE Catch2::Catch2WithMain)
target_compile_options(ConfLoader.test PRIVATE -include "lib/Core.hpp")

catch_discover_tests(ConfLoader.test)
