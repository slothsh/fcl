# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(
    TRAITS_MODULE_SRC_FILES
        Traits/Types.cppm
        Traits/Concepts.cppm
        Traits/TypeTraits.cppm
        Traits/Traits.cppm
)

set(
    MEMORY_MODULE_SRC_FILES
        Memory/MemoryTreeImpl.cppm
        Memory/MemoryTree.cppm
        Memory/Arena.cppm
        Memory/ArenaAllocator.cppm
        Memory/Memory.cppm
)

set(
    THREADING_MODULE_SRC_FILES
        Threading/ThreadPoolImpl.cppm
        Threading/ThreadPool.cppm
        Threading/Threading.cppm
)

set(
    NETWORKING_MODULE_SRC_FILES
        Networking/Async/ConnectionHandlerImpl.cppm
        Networking/Async/ConnectionHandler.cppm
        Networking/Async/SchedulerImpl.cppm
        Networking/Async/Scheduler.cppm
        Networking/Async/Async.cppm
        Networking/Networking.cppm
)

set(
    CLI_MODULE_SRC_FILES
        Cli/ArgumentParserImpl.cppm
        Cli/ArgumentParser.cppm
        Cli/Cli.cppm
)

set(
    DYNAMICVALUE_MODULE_SRC_FILES
        DynamicValue/ConfigValueImpl.cppm
        DynamicValue/ConfigValue.cppm
        DynamicValue/UserConfigurationImpl.cppm
        DynamicValue/UserConfiguration.cppm
        DynamicValue/DynamicValue.cppm
)

set(
    CONF_MODULE_SRC_FILES
        Conf/ConfCommon.cppm
        Conf/ConfTokenizerImpl.cppm
        Conf/ConfTokenizer.cppm
        Conf/ConfParserImpl.cppm
        Conf/ConfParser.cppm
        Conf/ConfAnalyzerImpl.cppm
        Conf/ConfAnalyzer.cppm
        Conf/ConfEvaluatorImpl.cppm
        Conf/ConfEvaluator.cppm
        Conf/Conf.cppm
)

add_library(LibTrader)

target_sources(
    LibTrader
    PUBLIC
    FILE_SET cxx_modules
    TYPE CXX_MODULES
    FILES
        ${TRAITS_MODULE_SRC_FILES}
        ${MEMORY_MODULE_SRC_FILES}
        ${THREADING_MODULE_SRC_FILES}
        ${NETWORKING_MODULE_SRC_FILES}
        ${CLI_MODULE_SRC_FILES}
        ${DYNAMICVALUE_MODULE_SRC_FILES}
        ${CONF_MODULE_SRC_FILES}
)

add_dependencies(LibTrader precompile_std)
target_compile_options(LibTrader PRIVATE -fmodule-file=std=${STD_PCM})
target_link_options(LibTrader PRIVATE -stdlib=libc++)
