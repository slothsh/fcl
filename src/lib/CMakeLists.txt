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
        ConfAnalyzer.cpp
        ConfLoader.cpp
)

target_precompile_headers(
    LibTrader
    PRIVATE
        Macros.hpp
        Types.hpp
        TypeTraits.hpp
        Concepts.hpp
        ConfigValue.hpp
        CommandLineArgumentParser.hpp
        ArenaAllocator.hpp
        ConnectionHandler.hpp
        Scheduler.hpp
        ThreadPool.hpp
        UserConfiguration.hpp
        Conf.hpp
        ConfLexer.hpp
        ConfParser.hpp
        ConfAnalyzer.hpp
        ConfLoader.hpp
)

add_subdirectory(include/)
