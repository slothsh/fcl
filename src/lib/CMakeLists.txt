# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CONF_MODULE_SRC_FILES
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

add_library(
    LibTrader
        # ConfigValue.cpp
        # ConnectionHandler.cpp
        # Scheduler.cpp
        # ThreadPool.cpp
        # CommandLineArgumentParser.cpp
        # UserConfiguration.cpp
        # ConfTokenizer.cpp
        # ConfParser.cpp
        # ConfAnalyzer.cpp
        # ConfEvaluator.cpp
)

target_sources(LibTrader
    PUBLIC
    FILE_SET cxx_modules
    TYPE CXX_MODULES
    FILES
        Types.cppm
        Concepts.cppm
        TypeTraits.cppm
        ${CONF_MODULE_SRC_FILES}
)

add_dependencies(LibTrader precompile_std)
target_compile_options(LibTrader PRIVATE -fmodule-file=std=${STD_PCM})
target_link_options(LibTrader PRIVATE -stdlib=libc++)

# target_precompile_headers(
#     LibTrader
#     PRIVATE
#         Macros.hpp
#         Types.hpp
#         Concepts.hpp
#         TypeTraits.hpp
#         # ConfigValue.hpp
#         # CommandLineArgumentParser.hpp
#         # ArenaAllocator.hpp
#         # ConnectionHandler.hpp
#         # Scheduler.hpp
#         # ThreadPool.hpp
#         # UserConfiguration.hpp
#         Conf.hpp
#         ConfTokenizer.hpp
#         ConfParser.hpp
#         ConfAnalyzer.hpp
#         ConfEvaluator.hpp
# )

add_subdirectory(include/)
