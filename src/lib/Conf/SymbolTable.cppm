export module Conf:Evaluator.SymbolTable;

import :Common;
import :Evaluator.SymbolImpl;
import Containers;
import std;

inline namespace {
    using namespace Conf::Language;
}

export struct SymbolTable {
    enum class Error {
        FAILED_TO_EMPLACE_SYMBOL,
    };

    template<typename... Args>
    std::expected<void, Error> emplaceSymbol(Args&&... args) noexcept {
        this->table.try_emplace(Symbol::toFullyQualifiedName(Symbol { std::forward<Args>(args)... }), std::forward<Args>(args)...);
        return {};
    }

    std::unordered_map<std::string, Symbol> table;
};
