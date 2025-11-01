# Tests
add_executable(StaticVector.test StaticVector.test.cpp)
add_dependencies(StaticVector.test precompile_std)
target_compile_options(StaticVector.test PRIVATE -fmodule-file=std=${STD_PCM})
target_link_options(StaticVector.test PRIVATE -stdlib=libc++)
target_link_libraries(StaticVector.test PRIVATE Catch2::Catch2WithMain LibTrader)

catch_discover_tests(StaticVector.test)
