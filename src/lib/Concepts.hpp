#pragma once

#include <concepts>

template<typename T, typename... Ts>
concept AnyOf = (std::same_as<T, Ts> || ...);

template<typename T, typename... Ts>
concept Number = std::floating_point<T> || std::integral<T>;

template<typename T, typename... Ts>
concept NoneOf = !(std::same_as<T, Ts> || ...);

template<typename T>
concept ImplicitConfigValue = std::same_as<ConfigBoolean, std::remove_cvref_t<T>> || Number<T> || std::constructible_from<ConfigString, T>;

template<typename T, typename C>
concept HasRawData = requires (T t) {
    { t.data() } -> std::same_as<C*>;
};

template<typename T>
concept ValueFromConfig = AnyOf<
    T,
    ConfigBoolean,
    ConfigNumber,
    ConfigString
>;
