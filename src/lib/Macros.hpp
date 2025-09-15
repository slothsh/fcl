#pragma once

#include <cstdio>
#include <print>

#define TODO(message) do {                                               \
    std::println(stderr, "{}:{} TODO: {}", __FILE__, __LINE__, message); \
    std::exit(1);                                                        \
} while (false)

#define WARN(message, ...) do {                                                      \
    std::println(stderr, "{}:{} WARNING "#message, __FILE__, __LINE__, __VA_ARGS__); \
} while (false)

#define INFO(message, ...) do {                                           \
    std::println("{}:{} INFO "#message, __FILE__, __LINE__, __VA_ARGS__); \
} while (false)

#define NOOP_VISITOR [](auto&&) {}
