file(GLOB_RECURSE TRAITS_MODULE_SRC_FILES Traits/*.cppm)
file(GLOB_RECURSE CONTAINERS_MODULE_SRC_FILES Containers/*.cppm)
file(GLOB_RECURSE CONF_MODULE_SRC_FILES Conf/*.cppm)

add_library(fcl)

target_sources(
    fcl
    PUBLIC
    FILE_SET cxx_modules
    TYPE CXX_MODULES
    FILES
        ${TRAITS_MODULE_SRC_FILES}
        ${CONTAINERS_MODULE_SRC_FILES}
        ${CONF_MODULE_SRC_FILES}
)

add_dependencies(fcl ${CXX_TARGET_PRECOMPILED_STD})
target_compile_options(fcl PRIVATE -fmodule-file=std=${CXX_STD_PCM})
target_link_options(fcl PRIVATE -stdlib=libc++)
