#pragma once

#include <string>
#include <unordered_map>
#include <variant>

class CommandLineArgumentParser {
public:
    enum ArgumentKind : int {
        FLAG,
        SWITCH,
        POSITIONAL,
    };

    struct FlagArgument {
        std::string flag;
        std::string value;
    };

    struct SwitchArgument {
        std::string value;
    };

    struct PositionalArgument {
        std::string value;
    };

    using ArgumentType = std::variant<FlagArgument, SwitchArgument, PositionalArgument>;
    using CommandLineArgumentsType = std::unordered_map<std::string, ArgumentType>;

    static CommandLineArgumentsType parseArguments(int argc, char** argv);
};
