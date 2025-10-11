# Test Data
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Tests
add_executable(ConfEvaluator.test ConfEvaluator.test.cpp)
target_link_libraries(ConfEvaluator.test PRIVATE LibTrader)
target_link_libraries(ConfEvaluator.test PRIVATE Catch2::Catch2WithMain)
target_compile_options(ConfEvaluator.test PRIVATE -include "lib/Core.hpp")

catch_discover_tests(ConfEvaluator.test)
