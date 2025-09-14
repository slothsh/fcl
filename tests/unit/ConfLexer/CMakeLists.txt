# Test Data
file(GLOB TEST_DATA "${CMAKE_CURRENT_SOURCE_DIR}/data/*.conf")

foreach(file IN LISTS TEST_DATA)
    get_filename_component(basename "${file}" NAME)
    configure_file(${file} ${CMAKE_CURRENT_BINARY_DIR}/data/${basename} COPYONLY)
endforeach()

# Tests
add_executable(ConfLexer.test ConfLexer.test.cpp)
target_link_libraries(ConfLexer.test PRIVATE LibTrader)
target_link_libraries(ConfLexer.test PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(ConfLexer.test)
