#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>

namespace Conf {

using NumberType = double;

// Binary Operator Strings
inline constexpr std::string_view STRING_EQUALS              = "=";
inline constexpr std::string_view STRING_WALRUS              = ":=";

// Symmetric Delimiter Strings
inline constexpr std::string_view STRING_OPEN_BRACE          = "{";
inline constexpr std::string_view STRING_CLOSE_BRACE         = "}";
inline constexpr std::string_view STRING_OPEN_DOUBLE_BRACE   = "{{";
inline constexpr std::string_view STRING_CLOSE_DOUBLE_BRACE  = "}}";
inline constexpr std::string_view STRING_OPEN_QUOTE          = "'";
inline constexpr std::string_view STRING_CLOSE_QUOTE         = "'";
inline constexpr std::string_view STRING_OPEN_DOUBLE_QUOTE   = "\"";
inline constexpr std::string_view STRING_CLOSE_DOUBLE_QUOTE  = "\"";

// White Space Strings
inline constexpr std::string_view STRING_TAB_FEED            = "\t";
inline constexpr std::string_view STRING_LINE_FEED           = "\r\n";
inline constexpr std::string_view STRING_VERTICAL_FEED       = "\v";
inline constexpr std::string_view STRING_SPACE               = " ";

// Single Character Strings
inline constexpr std::string_view STRING_SEMI_COLON          = ";";
inline constexpr std::string_view STRING_COMMA               = ",";

// Single Delimiter Strings
inline constexpr std::string_view STRING_COMMENT_SINGLE_LINE = "#";
inline constexpr std::string_view STRING_ESCAPE_SEQUENCE     = "\\";

// Keyword Strings
inline constexpr std::string_view STRING_KEYWORD_INCLUDE     = "include";

} // END OF NAMESPACE `Conf`

namespace Conf::Number {

template<std::floating_point T, HasRawData<std::string::value_type> S = std::string>
std::optional<T> fromHexadecimalString(S const& number_string) noexcept {
    auto const digit_value = [](char c) -> int64_t {
        if ('0' <= c && c <= '9') return c - '0';
        if ('A' <= c && c <= 'F') return 10 + (c - 'A');
        if ('a' <= c && c <= 'f') return 10 + (c - 'a');
        return -1;
    };

    auto const group_value = [](auto group) -> int64_t {
        return std::pow(16, std::get<0>(group)) * std::get<1>(group);
    };

    auto const digits = number_string | std::views::transform(digit_value) | std::views::reverse;

    if (std::ranges::any_of(digits, [](int digit){ return digit == -1; })) {
        return std::nullopt;
    }

    int64_t value = std::ranges::fold_left(
        std::views::zip(std::ranges::iota_view(0), digits) | std::views::transform(group_value),
        0LL,
        std::plus{}
    );

    return static_cast<T>(value);
}

template<std::floating_point T, typename S = std::string>
std::optional<T> fromDecimalString(S const& number_string) noexcept {
    auto const digit_value = [](char c) -> int64_t {
        if ('0' <= c && c <= '9') return c - '0';
        if (c == '.') return 0;
        return -1;
    };

    auto const group_value = [](auto group) -> T {
        return std::pow(10, std::get<0>(group)) * std::get<1>(group);
    };

    auto const digits = number_string | std::views::transform(digit_value);

    if (std::ranges::any_of(digits, [](int digit){ return digit == -1; })) {
        return std::nullopt;
    }

    auto const decimal_position = std::ranges::find(number_string, '.');
    auto const decimal_offset = decimal_position != number_string.end()
        ? std::ranges::distance(number_string.begin(), decimal_position)
        : 0;
    auto const off_by_one = decimal_position != number_string.end() ? 1 : 0;

    auto const whole_part = digits | std::views::take(digits.size() - decimal_offset - off_by_one) | std::views::reverse;
    auto const fractional_part = digits | std::views::drop(digits.size() - decimal_offset) | std::views::take(decimal_offset) | std::views::reverse;

    T const whole_value = std::ranges::fold_left(
        std::views::zip(std::ranges::iota_view(0), whole_part) | std::views::transform(group_value),
        0LL,
        std::plus{}
    );

    T const fractional_value = std::ranges::fold_left(
        std::views::zip(std::ranges::iota_view(-decimal_offset), fractional_part) | std::views::transform(group_value),
        0LL,
        std::plus{}
    );

    return whole_value + fractional_value;
}

template<std::floating_point T, typename S = std::string>
std::optional<T> fromOctalString(S const& number_string) noexcept {
    auto const digit_value = [](char c) -> int64_t {
        if ('0' <= c && c <= '7') return c - '0';
        return -1;
    };

    auto const group_value = [](auto group) -> int64_t {
        return std::pow(8, std::get<0>(group)) * std::get<1>(group);
    };

    auto const digits = number_string | std::views::transform(digit_value) | std::views::reverse;

    if (std::ranges::any_of(digits, [](int digit){ return digit == -1; })) {
        return std::nullopt;
    }

    int64_t value = std::ranges::fold_left(
        std::views::zip(std::ranges::iota_view(0), digits) | std::views::transform(group_value),
        0LL,
        std::plus{}
    );

    return static_cast<T>(value);
}


template<std::floating_point T, typename S = std::string>
std::optional<T> fromBinaryString(S const& number_string) noexcept {
    auto const digit_value = [](char c) -> int64_t {
        if ('0' <= c && c <= '1') return c - '0';
        return -1;
    };

    auto const group_value = [](auto group) -> int64_t {
        return std::pow(2, std::get<0>(group)) * std::get<1>(group);
    };

    auto const digits = number_string | std::views::transform(digit_value) | std::views::reverse;

    if (std::ranges::any_of(digits, [](int digit){ return digit == -1; })) {
        return std::nullopt;
    }

    int64_t value = std::ranges::fold_left(
        std::views::zip(std::ranges::iota_view(0), digits) | std::views::transform(group_value),
        0LL,
        std::plus{}
    );

    return static_cast<T>(value);
}

} // END OF NAMESPACE `Conf::Number`
