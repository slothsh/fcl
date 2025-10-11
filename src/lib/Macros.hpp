#pragma once

#include <cstdio>
#include <print>

#define TODO(message, ...) do {                                                                    \
    std::println(stderr, "[TODO] |{}:{}| " message, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
    std::exit(1);                                                                                  \
} while (false)

#define WARN(message, ...) do {                                                                       \
    std::println(stderr, "[WARNING] |{}:{}| " message, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
} while (false)

#define INFO(message, ...) do {                                                            \
    std::println("[INFO] |{}:{}| " message, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
} while (false)
