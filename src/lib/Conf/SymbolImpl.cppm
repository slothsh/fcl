export module Conf:Evaluator.SymbolImpl;

import :Common;
import :Evaluator.Symbol;
import std;

inline namespace {
    using namespace Conf::Language;
}

constexpr Symbol::Symbol(std::string_view name, NamespaceType const& namespaces, Node* node, SymbolConstantness constantness)
    : name{name}
    , namespaces{namespaces}
    , node{node}
    , constantness{constantness}
{}


std::string Symbol::toFullyQualifiedName(Symbol&& symbol) {
    auto size = std::ranges::fold_left(
        symbol.namespaces | std::views::transform([](std::string_view ns) {
            return ns.size();
        }),
        0z,
        [](std::size_t acc, std::size_t n) {
            return acc + n;
        }
    );


    std::string fully_qualified_name(size + symbol.namespaces.size() * (sizeof(STRING_PERIOD) - 1) + symbol.name.size(), '\0');
    for (auto const& ns : symbol.namespaces) {
        std::ranges::copy(ns, std::back_inserter(fully_qualified_name));
        std::ranges::copy(STRING_PERIOD, std::back_inserter(fully_qualified_name));
    }

    std::ranges::copy(symbol.name, std::back_inserter(fully_qualified_name));

    return fully_qualified_name;
}
