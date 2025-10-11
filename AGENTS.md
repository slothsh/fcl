# Agent Instructions for Trader Codebase

## Build & Test Commands
- **Build**: `./build.sh` (uses CMake with 8 parallel jobs, generates compile_commands.json)
- **Test all**: `./test.sh` (builds then runs ctest with output on failure)
- **Single test**: `./build/tests/unit/<TestName>/<TestName>.test` (e.g., `./build/tests/unit/ConfTokenizer/ConfTokenizer.test`)
- **Manual test**: `ctest --output-on-failure --test-dir ./build/tests -R <TestName>`

## Language & Standards
- **C++23** (CMAKE_CXX_STANDARD 23, required)
- Use modern C++ features: `std::expected`, `std::optional`, concepts, ranges, formatters

## Code Style
- **Headers**: `#pragma once` for include guards
- **Imports**: Group by standard library, then local; alphabetical within groups
- **Naming**: PascalCase classes/enums, camelCase methods/variables, SCREAMING_SNAKE_CASE enum values/constants
- **Types**: Use `using` aliases for complex types at class or namespace scope (e.g., `using TokenListType = std::vector<Token>`)
- **Templates**: Prefer concepts (e.g., `std::invocable<IdType>`) over SFINAE
- **Error handling**: Use `std::expected<T, Error>` for fallible operations, `std::optional<T>` for nullable values
- **Enums**: Use `enum class` with explicit underlying type when needed
- **Constants**: `static constexpr` for compile-time constants, `static` for runtime-initialized globals
- **Formatting**: 4-space indent, braces on same line for functions/classes, opening brace on new line discouraged
- **Output**: Use `std::println` from `<print>` for console output
