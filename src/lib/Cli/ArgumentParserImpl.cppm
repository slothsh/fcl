module;

#include "../Macros.hpp"

export module Cli:ArgumentParserImpl;

import :ArgumentParser;
import std;

inline namespace {
    using ArgumentType = typename ArgumentParser::ArgumentType;
    using CommandLineArgumentsType = typename ArgumentParser::CommandLineArgumentsType;
}

static constexpr std::size_t ASCII_UPPERCASE_OFFSET = 65;
static constexpr std::size_t ASCII_LOWERCASE_OFFSET = 97;
static constexpr std::array<std::pair<char, const char*>, 52> SHORT_FLAG_MAP = {{
    { 'A', "-A" }, { 'B', "-B" }, 
    { 'C', "-C" }, { 'D', "-D" }, 
    { 'E', "-E" }, { 'F', "-F" }, 
    { 'G', "-G" }, { 'H', "-H" }, 
    { 'I', "-I" }, { 'J', "-J" }, 
    { 'K', "-K" }, { 'L', "-L" }, 
    { 'M', "-M" }, { 'N', "-N" }, 
    { 'O', "-O" }, { 'P', "-P" }, 
    { 'Q', "-Q" }, { 'R', "-R" }, 
    { 'S', "-S" }, { 'T', "-T" }, 
    { 'U', "-U" }, { 'V', "-V" }, 
    { 'W', "-W" }, { 'X', "-X" }, 
    { 'Y', "-Y" }, { 'Z', "-Z" }, 

    { 'a', "-a" }, { 'b', "-b" }, 
    { 'c', "-c" }, { 'd', "-d" }, 
    { 'e', "-e" }, { 'f', "-f" }, 
    { 'g', "-g" }, { 'h', "-h" }, 
    { 'i', "-i" }, { 'j', "-j" }, 
    { 'k', "-k" }, { 'l', "-l" }, 
    { 'm', "-m" }, { 'n', "-n" }, 
    { 'o', "-o" }, { 'p', "-p" }, 
    { 'q', "-q" }, { 'r', "-r" }, 
    { 's', "-s" }, { 't', "-t" }, 
    { 'u', "-u" }, { 'v', "-v" }, 
    { 'w', "-w" }, { 'x', "-x" }, 
    { 'y', "-y" }, { 'z', "-z" }, 
}};

static inline bool isSingleShortFlag(std::string_view item) {
    return item.size() == 2 && item.starts_with('-') && std::isalpha(item.at(1));
}

static inline bool isCompoundShortFlag(std::string_view item) {
    return item.size() > 2 && item.starts_with('-') && std::ranges::all_of(
        item | std::views::drop(1),
        [](char c) { return std::isalpha(c); }
    );
}

static inline bool isSwitch(std::string_view item) {
    return item.size() > 2 && item.starts_with("--") && std::ranges::all_of(
        item | std::views::drop(2),
        [](char c) { return std::isalpha(c); }
    );
}

static inline bool isLongFlagExpression(std::string_view item) {
    return item.size() > 3 && item.starts_with("--") && std::ranges::distance(std::views::split(item, '=')) == 2;
}

static inline bool isRestArePositionals(std::span<char*> arguments) {
    return std::ranges::all_of(arguments, [](char* item) {
        return !isSingleShortFlag(item) || !isCompoundShortFlag(item) || !isSwitch(item) || !isLongFlagExpression(item);
    });
}

static std::optional<std::string_view> parseSingleShortFlag(std::string_view item) {
    if (!isSingleShortFlag(item)) {
        return std::nullopt;
    }

    return item;
}

static std::optional<std::vector<std::string_view>> parseCompoundShortFlag(std::string_view item) {
    if (!isCompoundShortFlag(item)) {
        return std::nullopt;
    }

    auto const flag_list = item
        | std::views::drop(1)
        | std::views::transform([](char c) {
            auto const is_lower_case = std::islower(c);
            return std::string_view{
                SHORT_FLAG_MAP.at(
                    c
                    - (!is_lower_case ? ASCII_UPPERCASE_OFFSET : ASCII_LOWERCASE_OFFSET)
                    + (!is_lower_case ? 0 : SHORT_FLAG_MAP.size() / 2)
                ).second
            };
        })
        | std::ranges::to<std::vector>();

    return flag_list;
}

static std::optional<std::string_view> parseSwitch(std::string_view item) {
    if (!isSwitch(item)) {
        return std::nullopt;
    }

    return item;
}

static std::optional<std::pair<std::string_view, std::string_view>> parseLongFlagExpression(std::string_view item) {
    if (!isLongFlagExpression(item)) {
        return std::nullopt;
    }

    auto parts = item | std::views::split('=');
    auto left = std::string_view{parts.front()};
    auto right = std::string_view{(parts | std::views::drop(1)).front()};

    return std::make_pair(left, right);
}

static std::optional<std::vector<std::string_view>> parseRestAsPositionals(std::span<char*> arguments) {
    if (!isRestArePositionals(arguments)) {
        return std::nullopt;
    }

    return arguments
        | std::views::transform([](char* item) {
            return std::string_view{item};
        })
        | std::ranges::to<std::vector>();
}

static std::optional<std::string_view> parseNextAsValue(std::size_t current_index, std::span<char*> arguments) {
    if (current_index + 1 >= arguments.size()) {
        return std::nullopt;
    }

    auto const next_item = arguments[current_index + 1];

    if (isSingleShortFlag(next_item) || isCompoundShortFlag(next_item) || isSwitch(next_item) || isLongFlagExpression(next_item)) {
        return std::nullopt;
    }

    return next_item;
}

CommandLineArgumentsType ArgumentParser::parseArguments(int argc, char** argv) {
    CommandLineArgumentsType parsed_arguments;

    auto const input_arguments = std::span(argv, argc);

    for (std::size_t i = 0; i < static_cast<std::size_t>(argc); ++i) {
        std::string_view const item = input_arguments[i];

        if (i == 0) {
            WARN("not implemented: program name: {}", item);
        } else if (auto const parsed = parseSingleShortFlag(item); parsed) {
            auto next_value = parseNextAsValue(i, input_arguments);

            if (next_value) {
                parsed_arguments.insert({
                    parsed->data(),
                    FlagArgument{ .flag = parsed->data(), .value = next_value->data() }
                });
                ++i;
            } else {
                parsed_arguments.insert({
                    parsed.value().data(),
                    SwitchArgument{ .value = parsed->data() }
                });
            }
        } else if (auto const parsed = parseCompoundShortFlag(item); parsed) {
            for (std::size_t parsed_index = 0; auto const& flag : parsed.value()) {
                auto next_value = parseNextAsValue(i, input_arguments);

                if (parsed_index < parsed->size() - 1) {
                    parsed_arguments.insert({
                        flag.data(),
                        SwitchArgument{ .value = flag.data() }
                    });
                } else {
                    auto next_value = parseNextAsValue(i, input_arguments);

                    if (next_value) {
                        parsed_arguments.insert({
                            flag.data(),
                            FlagArgument{ .flag = flag.data(), .value = next_value->data() }
                        });
                        ++i;
                    } else {
                        parsed_arguments.insert({
                            flag.data(),
                            SwitchArgument{ .value = flag.data() }
                        });
                    }
                }

                ++parsed_index;
            }
        } else if (auto const parsed = parseSwitch(item); parsed) {
            parsed_arguments.insert({
                parsed->data(),
                SwitchArgument{ .value = parsed->data() }
            });
        } else if (auto const parsed = parseLongFlagExpression(item); parsed) {
            parsed_arguments.insert({
                {parsed->first.data(), parsed->first.size()},
                FlagArgument{ .flag = {parsed->first.data(), parsed->first.size()}, .value = parsed->second.data() }
            });
        } else if (auto const parsed = parseRestAsPositionals(input_arguments | std::views::drop(i)); parsed) {
            for (auto const& item : parsed.value()) {
                parsed_arguments.insert({
                    {item.data(), item.size()},
                    PositionalArgument{ .value = {item.data(), item.size()} }
                });
            }
        } else {
            TODO("not implemented: unknown argument: {}", item);
        }
    }

    return parsed_arguments;
}
