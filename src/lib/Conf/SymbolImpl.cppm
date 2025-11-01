export module Conf:Evaluator.SymbolImpl;

import :Common;
import :Evaluator.Symbol;
import std;

inline namespace {
    using namespace Conf::Language;
}

constexpr Symbol::Symbol(std::string_view _name, NamespaceType const& namespaces, SymbolConstantness constantness)
    : name{_name}
    , namespaces{namespaces}
    , constantness{constantness}
{}
