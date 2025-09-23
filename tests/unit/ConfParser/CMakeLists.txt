# Test Data
file(GLOB TEST_DATA "${CMAKE_CURRENT_SOURCE_DIR}/data/*.conf")

foreach(file IN LISTS TEST_DATA)
    get_filename_component(basename "${file}" NAME)
    configure_file(${file} ${CMAKE_CURRENT_BINARY_DIR}/data/${basename} COPYONLY)
endforeach()

# Tests
add_executable(ConfParser.test ConfParser.test.cpp)
target_link_libraries(ConfParser.test PRIVATE LibTrader)
target_link_libraries(ConfParser.test PRIVATE Catch2::Catch2WithMain)
target_compile_options(ConfParser.test PRIVATE -include "lib/Core.hpp")

catch_discover_tests(ConfParser.test)
