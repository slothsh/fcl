export module DynamicValue:ConfigValue;

import Traits;
import std;

export class ConfigValue {
public:
    template<typename T>
        requires std::same_as<ConfigBoolean, std::remove_cvref<T>>
    ConfigValue(T&& value)
        : m_kind{BOOL}
        , m_value{ .boolean = std::forward<T>(value) }
    {}

    template<Number T>
    ConfigValue(T&& value)
        : m_kind{NUMBER}
        , m_value{ .number = std::forward<T>(value) }
    {}

    template<NoneOf<ConfigBoolean, ConfigNumber> T>
        requires std::constructible_from<ConfigString, T>
    ConfigValue(T&& value)
        : m_kind{STRING}
        , m_value{ .string = std::forward<T>(value) }
    {}

    template<ImplicitConfigValue T>
    static T cast(std::string_view value) {
        if constexpr (std::same_as<ConfigBoolean, T>) {
            return value == "true" ? true : false;
        } else if constexpr (std::same_as<ConfigNumber, T>) {
            return std::stod(value.data());
        } else if constexpr (std::same_as<ConfigString, T>) {
            return value;
        }
    }

    template<ValueFromConfig T>
    inline std::optional<T> as() const noexcept {
        if constexpr (std::same_as<std::remove_cvref<T>, ConfigBoolean>) {
            switch (m_kind) {
                case BOOL: return m_value.boolean;
                default: return std::nullopt;
            }
        } else if constexpr (Number<T>) {
            switch (m_kind) {
                case NUMBER: return m_value.number;
                default: return std::nullopt;
            }
        } else if constexpr (std::constructible_from<ConfigString, T>) {
            switch (m_kind) {
                case STRING: return m_value.string;
                default: return std::nullopt;
            }
        }

        return std::nullopt;
    }

    ConfigValue(ConfigValue const& config_value);
    ConfigValue& operator=(ConfigValue const& config_value);
    ConfigValue(ConfigValue&& config_value) noexcept;
    ConfigValue& operator=(ConfigValue&& config_value) noexcept;

    ~ConfigValue() noexcept;

private:
    enum : int {
        BOOL,
        NUMBER,
        STRING,
    } m_kind;

    union ValueUnion {
        ConfigBoolean boolean;
        ConfigNumber number;
        ConfigString string = "";
        ~ValueUnion() noexcept;
    } m_value;
};
