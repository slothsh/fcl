#include <memory>
#include <variant>

namespace detail {
    using FlagArgument = typename CommandLineArgumentParser::FlagArgument;
    using SwitchArgument = typename CommandLineArgumentParser::SwitchArgument;
    using PositionalArgument = typename CommandLineArgumentParser::PositionalArgument;
    using ConfigMap = typename UserConfiguration::MapType;
}

static std::unique_ptr<UserConfiguration> initializeUserArguments(int argc, char** argv) {
    auto arguments = CommandLineArgumentParser::parseArguments(argc, argv);
    UserConfiguration user_config{};

    static auto visitor = Visitors {
        [&user_config](detail::FlagArgument const& argument) {
            if (argument.flag == "--max-processes") {
                user_config.insert("max-processes", ConfigValue::cast<ConfigNumber>(argument.value));
            }
        },

        [](detail::SwitchArgument const& argument) {},

        [](detail::PositionalArgument const& argument) {},
    };

    for (auto const& item : arguments) {
        std::visit(visitor, item.second);
    }

    return std::make_unique<UserConfiguration>(std::move(user_config));
}

static std::unique_ptr<StrategyService> initializeStrategyService() {
    return std::make_unique<StrategyService>();
}

void ServiceState::initialize(int argc, char** argv) {
    if (USER_CONFIG) {
        TODO("not implemented");
    }

    if (STRATEGY_SERVICE) {
        TODO("not implemented");
    }

    USER_CONFIG = initializeUserArguments(argc, argv);
    STRATEGY_SERVICE = initializeStrategyService();

    std::println("value: {}", USER_CONFIG->get<ConfigNumber>("max-processes"));
}
