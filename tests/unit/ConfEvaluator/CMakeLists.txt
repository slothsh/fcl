# Test Data
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Tests
add_executable(ConfEvaluator.test ConfEvaluator.test.cpp)
add_dependencies(ConfEvaluator.test precompile_std)
target_compile_options(ConfEvaluator.test PRIVATE -fmodule-file=std=${STD_PCM})
target_link_options(ConfEvaluator.test PRIVATE -stdlib=libc++)
target_link_libraries(ConfEvaluator.test PRIVATE Catch2::Catch2WithMain LibTrader)

catch_discover_tests(ConfEvaluator.test)
