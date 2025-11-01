export module Conf:Evaluator.SymbolTableImpl;

import :Evaluator.SymbolImpl;
import :Evaluator.SymbolTable;
import std;

inline namespace {
    using namespace Conf::Language;
    using enum SymbolTable::Error;
}
