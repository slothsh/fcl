function(find_package_manager_brew COMMAND)
    find_program(BREW_COMMAND brew)
    if(BREW_COMMAND)
        message(STATUS "Package Manager: Homebrew not found")
        set(${COMMAND} ${BREW_COMMAND} PARENT_SCOPE)
    endif()
endfunction()

function(add_test_target TARGET_NAME SOURCE_FILES)
    add_executable(${TARGET_NAME} ${SOURCE_FILES})
    add_dependencies(${TARGET_NAME} ${CXX_TARGET_PRECOMPILED_STD})
    target_compile_options(${TARGET_NAME} PRIVATE -fmodule-file=std=${CXX_STD_PCM})
    target_link_options(${TARGET_NAME} PRIVATE -stdlib=libc++)
    target_link_libraries(${TARGET_NAME} PRIVATE Catch2::Catch2WithMain ${PROJECT_LIBRARY_TARGET})
endfunction()

function(add_executable_target TARGET_NAME)
    cmake_parse_arguments(
        PARSE_ARGV 1
        ARG
        ""
        ""
        "SOURCE_FILES;MODULE_FILES"
    )

    add_executable(${TARGET_NAME} ${ARG_SOURCE_FILES})
    target_sources(
        ${TARGET_NAME}
        PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        FILES
            ${ARG_MODULE_FILES}
    )

    add_dependencies(${TARGET_NAME} ${CXX_TARGET_PRECOMPILED_STD})
    target_compile_options(${TARGET_NAME} PRIVATE -fmodule-file=std=${CXX_STD_PCM})
    target_link_options(${TARGET_NAME} PRIVATE -stdlib=libc++)
endfunction()

function(add_library_target TARGET_NAME)
    cmake_parse_arguments(
        PARSE_ARGV 1
        ARG
        ""
        ""
        "SOURCE_FILES;MODULE_FILES"
    )

    add_library(${TARGET_NAME} ${ARG_SOURCE_FILES})
    target_sources(
        ${TARGET_NAME}
        PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        FILES
            ${ARG_MODULE_FILES}
    )

    add_dependencies(${TARGET_NAME} ${CXX_TARGET_PRECOMPILED_STD})
    target_compile_options(${TARGET_NAME} PRIVATE -fmodule-file=std=${CXX_STD_PCM})
    target_link_options(${TARGET_NAME} PRIVATE -stdlib=libc++)
endfunction()
