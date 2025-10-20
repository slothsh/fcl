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

set(STD_PCM ${CMAKE_BINARY_DIR}/src/std.pcm)

add_subdirectory(lib/)
# add_subdirectory(services/)
