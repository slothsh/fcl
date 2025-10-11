#pragma once

template <typename T>
struct FunctionTraits;

template <typename Return, typename... Args>
struct FunctionTraits<Return(Args...) noexcept> {
    using return_type = Return;
    using argument_types = std::tuple<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
};

template <typename Return, typename... Args>
struct FunctionTraits<Return(Args...)> {
    using return_type = Return;
    using argument_types = std::tuple<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
};
