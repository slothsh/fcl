#pragma once

#include <cstdio>
#include <print>

#define TODO(message) do {                                               \
    std::println(stderr, "{}:{} TODO: {}", __FILE__, __LINE__, message); \
    std::exit(1);                                                        \
} while (false)

#define WARN(message) do {                                               \
    std::println(stderr, "{}:{} WARNING {}", __FILE__, __LINE__, message); \
} while (false)
