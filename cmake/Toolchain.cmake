# Initialization Macros

macro(initialize_toolchain)
    set_global_options()
    prepare_toolchain()
endmacro()

macro(set_global_options)
    # Support c++ modules
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD 26)
    set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
endmacro()

macro(prepare_toolchain)
    if(UNIX AND NOT APPLE)
        message(FATAL_ERROR "todo: not implemented")
    elseif(APPLE)
        message(STATUS "Preparing toolchain for ${CMAKE_SYSTEM_NAME}")

        find_package_manager_brew(BREW_COMMAND)

        if(DEFINED BREW_COMMAND)
            message(STATUS "Found Homebrew package manager: ${BREW_COMMAND}")
        else()
            message(FATAL_ERROR "System package manager not found")
        endif()

        message(STATUS "Searching for Clang LLVM toolchain")

        prepare_darwin_llvm_toolchain(TOOLCHAIN_NAME)

        if(DEFINED TOOLCHAIN_NAME)
            message(STATUS "Toolchain: ${TOOLCHAIN_NAME}")
        else()
            message(FATAL_ERROR "Toolchain not found")
        endif()

        prepare_darwin_llvm_modules_support(MODULES_SUPPORT_SUCCESS)

        if(MODULES_SUPPORT_SUCCESS)
            message(STATUS "C++ modules configured for ${TOOLCHAIN_NAME} toolchain")
        else()
            message(FATAL_ERROR "Could not prepare toolchain")
        endif()
    elseif(WIN32)
        message(FATAL_ERROR "todo: not implemented")
    else()
        message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
    endif()
endmacro()

# Prepare Platform Toolchains

function(prepare_darwin_llvm_toolchain NAME)
    execute_process(
        COMMAND ${BREW_COMMAND} --prefix llvm
        OUTPUT_VARIABLE LLVM_ROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if(LLVM_ROOT)
        set(CMAKE_C_COMPILER "${LLVM_ROOT}/bin/clang" PARENT_SCOPE)
        set(CMAKE_CXX_COMPILER "${LLVM_ROOT}/bin/clang++" PARENT_SCOPE)
        set(LLVM_LIBRARY_DIR "${LLVM_ROOT}/lib" PARENT_SCOPE)

        execute_process(
            COMMAND xcrun --show-sdk-path
            OUTPUT_VARIABLE APPLE_SDK_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        add_compile_options(-isysroot "${APPLE_SDK_PATH}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${LLVM_ROOT}/lib/c++ -Wl,-rpath,${LLVM_ROOT}/lib/c++" PARENT_SCOPE)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L${LLVM_ROOT}/lib/c++ -Wl,-rpath,${LLVM_ROOT}/lib/c++" PARENT_SCOPE)
        set(LLVM_LIBRARY_DIR "${LLVM_ROOT}/lib" PARENT_SCOPE)

        set(${NAME} "LLVM Clang" PARENT_SCOPE)
        set(LLVM_ROOT ${LLVM_ROOT} PARENT_SCOPE)
    endif()
endfunction()

function(prepare_darwin_llvm_modules_support SUCCESS)
    set(${SUCCESS} FALSE PARENT_SCOPE)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-reserved-identifier -Wno-reserved-module-identifier" PARENT_SCOPE)

    add_custom_command(
        OUTPUT std.pcm
        COMMAND ${CMAKE_CXX_COMPILER}
                -std=c++${CMAKE_CXX_STANDARD}
                -stdlib=libc++
                -Wno-reserved-identifier
                -Wno-reserved-module-identifier
                --precompile
                -o std.pcm
                ${LLVM_ROOT}/share/libc++/v1/std.cppm
        DEPENDS ${LLVM_ROOT}/share/libc++/v1/std.cppm
        COMMENT "Precompiling libc++ standard module (std.pcm)"
    )

    add_custom_target(cxx_precompiled_std ALL DEPENDS std.pcm)

    set(CXX_TARGET_PRECOMPILED_STD cxx_precompiled_std PARENT_SCOPE)
    set(CXX_STD_PCM ${CMAKE_BINARY_DIR}/std.pcm PARENT_SCOPE)
    set(${SUCCESS} TRUE PARENT_SCOPE)
endfunction()
