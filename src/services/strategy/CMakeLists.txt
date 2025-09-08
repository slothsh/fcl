find_package(msgpack-cxx REQUIRED)

add_executable(
    strategy-service
        Main.cpp
        ServiceState.cpp
        StrategyService.cpp
)

set_target_properties(
    strategy-service
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_precompile_headers(
    strategy-service
    PRIVATE
        StrategyService.hpp
        Globals.hpp
        ServiceState.hpp
)

target_compile_options(strategy-service PRIVATE -include "lib/Core.hpp")
target_link_libraries(strategy-service PRIVATE LibTrader)
target_link_libraries(strategy-service PRIVATE msgpack-cxx)
