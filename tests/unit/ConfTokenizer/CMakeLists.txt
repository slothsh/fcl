# Test Data
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Tests
add_executable(ConfTokenizer.test ConfTokenizer.test.cpp)
add_dependencies(ConfTokenizer.test precompile_std)
target_compile_options(ConfTokenizer.test PRIVATE -fmodule-file=std=${STD_PCM})
target_link_options(ConfTokenizer.test PRIVATE -stdlib=libc++)
target_link_libraries(ConfTokenizer.test PRIVATE Catch2::Catch2WithMain LibTrader)

catch_discover_tests(ConfTokenizer.test)
