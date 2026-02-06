# Test Data
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Tests
add_test_target(ConfEvaluator.test ConfEvaluator.test.cpp)
catch_discover_tests(ConfEvaluator.test)
