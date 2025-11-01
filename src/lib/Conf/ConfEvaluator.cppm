module;

#include "../Macros.hpp"

export module Conf:Evaluator;

import :Common;
import :Evaluator.SymbolTable;
import Containers;
import std;

inline namespace {
    using namespace Conf::Language;
}

export class ConfEvaluator {
public:
    enum class Error {
        FAILED_TO_ANALYZE,
        FAILED_TO_TOKENIZE,
        FAILED_TO_PARSE,
        FAILED_TO_RESOLVE_INCLUDE_PATH,
        FAILED_TO_BUILD_SYMBOL_TABLE,
        NULL_AST_POINTER,
        NULL_SELF_POINTER,
        CHILD_NOT_FOUND,
    };

    ConfEvaluator() = delete;

    explicit ConfEvaluator(std::string_view config_file_path) noexcept;

    std::expected<void, Error> load();
    std::expected<void, Error> analyzeAst() const;
    std::expected<void, Error> preProcess();
    std::expected<void, Error> evaluate(NodePtr& ast) const;

    std::expected<void, Error> visitSymbolTable(NodePtr& ast, StaticVector<std::string_view, SYMBOL_NAMESPACES_SIZE>& namespace_buffer);
    std::expected<void, Error> visitIncludes(NodePtr& ast);
    std::expected<void, Error> visitSpliceIncludes(Node* parent, Node* me, NodePtr& splice);
    std::expected<Node*, Error> findNearestRootAncestor(Node* me);

    NodePtr const& ast() const;

    SymbolTable m_symbol_table;
private:
    NodePtr m_ast;
    PathType m_config_file_path;
};
