export module Conf:Evaluator.Symbol;

import :Common;
import std;

inline namespace {
    using namespace Conf::Language;
}

export struct Symbol {
    Symbol() = delete;
    explicit constexpr Symbol(std::string_view name, NamespaceType const& namespaces, Node* node, SymbolConstantness constantness);

    static std::string toFullyQualifiedName(Symbol&& symbol);

    std::string_view name;
    NamespaceType namespaces;
    Node* node;
    SymbolConstantness constantness;
};
