include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(
    LibTrader
        ConfigValue.cpp
        ConnectionHandler.cpp
        Scheduler.cpp
        ThreadPool.cpp
        CommandLineArgumentParser.cpp
        UserConfiguration.cpp
        ConfLexer.cpp
        ConfParser.cpp
        ConfInterpreter.cpp
)

target_precompile_headers(
    LibTrader
    PRIVATE
        Macros.hpp
        Types.hpp
        Concepts.hpp
        ConfigValue.hpp
        CommandLineArgumentParser.hpp
        ArenaAllocator.hpp
        ConnectionHandler.hpp
        Scheduler.hpp
        ThreadPool.hpp
        UserConfiguration.hpp
        ConfLexer.hpp
        ConfParser.hpp
        ConfInterpreter.hpp
)

add_subdirectory(include/)
