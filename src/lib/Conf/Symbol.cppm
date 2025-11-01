export module Conf:Evaluator.Symbol;

import :Common;
import std;

inline namespace {
    using namespace Conf::Language;
}

export struct Symbol {
    std::string_view name;
    NamespaceType namespaces;
    SymbolConstantness constantness;
    // TODO: aliases

    explicit constexpr Symbol(std::string_view _name, NamespaceType const& namespaces, SymbolConstantness constantness);
};
