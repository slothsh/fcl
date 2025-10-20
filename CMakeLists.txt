cmake_minimum_required(VERSION 4.0.3...4.1.1)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_COMPILER /opt/homebrew/opt/llvm/bin/clang)
set(CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm/bin/clang++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(trader VERSION 0.8.0 LANGUAGES CXX)

# Add LLVM libc++ & disable module-specific warnings 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-reserved-identifier -Wno-reserved-module-identifier")

# Build standard library module
add_custom_command(
    OUTPUT std.pcm
    COMMAND ${CMAKE_CXX_COMPILER}
            -std=c++23
            -stdlib=libc++
            -Wno-reserved-identifier
            -Wno-reserved-module-identifier
            --precompile
            -o std.pcm
            /opt/homebrew/opt/llvm/share/libc++/v1/std.cppm
    DEPENDS /opt/homebrew/opt/llvm/share/libc++/v1/std.cppm
    COMMENT "Precompiling libc++ standard module (std.pcm)"
)


add_custom_target(precompile_std ALL DEPENDS std.pcm)

execute_process(
    COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE APPLE_SDK_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_compile_options(-isysroot "${APPLE_SDK_PATH}")

set(STD_PCM ${CMAKE_BINARY_DIR}/std.pcm)

add_subdirectory(src/)
add_subdirectory(tests/)
