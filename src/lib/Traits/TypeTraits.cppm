export module Traits:TypeTraits;

import std;
import :Concepts;

export template <typename T>
struct FunctionTraits;

export template <typename Return, typename... Args>
struct FunctionTraits<Return(Args...) noexcept> {
    using ReturnType = Return;
    using ArgumentTypes = std::tuple<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
};

export template <typename Return, typename... Args>
struct FunctionTraits<Return(Args...)> {
    using ReturnType = Return;
    using ArgumentTypes = std::tuple<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
};

export template<std::size_t I, typename Kind, typename Variant, typename Inner, InvocableConstReferenceReturn<std::remove_cvref_t<Inner>&> auto F, Kind... Kinds>
struct FunctionArgument {
    using KindType = std::remove_cvref_t<Kind>;
    using InnerType = std::remove_cvref_t<Inner>;
    using ReturnType = std::invoke_result_t<decltype(F), Inner&>;
    using VariantType = std::remove_cvref_t<Variant>;
    static constexpr std::size_t Index = I;
    static ReturnType const& unwrap(InnerType& inner) { return F(inner); }
    static constexpr std::size_t kinds_size = sizeof...(Kinds);
    static constexpr std::array<KindType, 32> kinds { Kinds... };
};

export template<typename Kind, typename Variant, IsFunctionArgument... U>
struct FunctionSchemaTraits {
    using Parameters = std::array<std::array<Kind, 32>, 128>;
    using Unwrappers = std::tuple<U...>;
    using VariantType = Variant;
    static constexpr std::size_t arity = sizeof...(U);
    static constexpr Parameters parameters { U::kinds... };
};
