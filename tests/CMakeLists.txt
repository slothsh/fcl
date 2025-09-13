find_package(Catch2 3 REQUIRED)

add_executable(ConfLexer.test unit/ConfLexer.test.cpp)

target_link_libraries(ConfLexer.test PRIVATE LibTrader)
target_link_libraries(ConfLexer.test PRIVATE Catch2::Catch2WithMain)

enable_testing()
include(CTest)
include(Catch)

catch_discover_tests(ConfLexer.test)
