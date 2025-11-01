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
    std::expected<void, Error> emplaceSymbol(Node* node, Args... args) noexcept {
        this->table.try_emplace(node, std::forward<Args>(args)...);
        return {};
    }

    std::unordered_map<Node*, Symbol> table;
};
