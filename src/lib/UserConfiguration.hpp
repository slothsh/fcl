#pragma once

#include "Macros.hpp"
#include <unordered_map>
#include <string>
#include <string_view>

class UserConfiguration {
public:
    using MapType = std::unordered_map<std::string, ConfigValue>;

    UserConfiguration() = default;

    UserConfiguration(UserConfiguration const& user_config) = delete;
    UserConfiguration& operator=(UserConfiguration const& user_config) = delete;

    UserConfiguration(UserConfiguration&& user_config) = default;
    UserConfiguration& operator=(UserConfiguration&& user_config) = default;

    ~UserConfiguration() noexcept = default;

    template<ImplicitConfigValue T>
    void insert(std::string_view key, T&& value) noexcept {
        m_entries.insert({ key.data(), std::forward<T>(value) });
    }

    template<ImplicitConfigValue T>
    T get(std::string_view key) const noexcept {
        if (!m_entries.contains(key.data())) {
            TODO("not implemented");
        }

        std::optional<T> value_result = m_entries.at(key.data()).as<T>();

        if (!value_result) {
            TODO("not implemented");
        }

        return value_result.value();
    }

private:
    MapType m_entries;
};
