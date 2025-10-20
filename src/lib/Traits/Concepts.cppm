export module Traits:Concepts;

import std;
import :Types;

export template<typename T, typename... Ts>
concept AnyOf = (std::same_as<T, Ts> || ...);

export template<typename T, typename... Ts>
concept Number = std::floating_point<T> || std::integral<T>;

export template<typename T, typename... Ts>
concept NoneOf = !(std::same_as<T, Ts> || ...);

export template<typename T>
concept ImplicitConfigValue = std::same_as<ConfigBoolean, std::remove_cvref_t<T>> || Number<T> || std::constructible_from<ConfigString, T>;

export template<typename T, typename C>
concept HasRawData = requires (T t) {
    { t.data() } -> std::same_as<C*>;
};

export template<typename T>
concept ValueFromConfig = AnyOf<
    T,
    ConfigBoolean,
    ConfigNumber,
    ConfigString
>;

export template<typename F, typename... Args>
concept InvocableConstReferenceReturn = std::invocable<F, Args...> && requires (F f, Args... args) {
    { f(args...) } -> std::same_as<std::invoke_result_t<F, Args...> const&>;
};

export template<typename T>
concept IsFunctionArgument = requires (T t, typename T::InnerType& inner) {
    typename T::KindType;
    typename T::InnerType;
    typename T::ReturnType;
    typename T::VariantType;
    { T::Index } -> std::same_as<std::size_t const&>;
    { T::kinds_size } -> std::same_as<std::size_t const&>;
    { T::kinds } -> std::same_as<std::array<typename T::KindType, 32> const&>;
    { T::unwrap(inner) } -> std::same_as<typename T::ReturnType const&>;
};

export template<typename T, typename P>
concept HasFunctionTraits = requires(T t) {
    { T::arity } -> std::same_as<std::size_t const&>;
    { T::parameters } -> std::same_as<P const&>;
};

export template<typename T, typename R>
concept IsSubscriptable = requires (T t) {
    { t[0] } -> std::same_as<R&>;
    { t.at(0) } -> std::same_as<R&>;
};
