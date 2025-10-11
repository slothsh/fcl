#pragma once

#include <expected>

class ConfEvaluator {
public:
    using NodePtr = Conf::Language::NodePtr;
    using Node = Conf::Language::Node;
    using PathType = Conf::Language::PathType;

    enum class Error {
        FAILED_TO_ANALYZE,
        FAILED_TO_TOKENIZE,
        FAILED_TO_PARSE,
        FAILED_TO_RESOLVE_INCLUDE_PATH,
        NULL_AST_POINTER,
        NULL_SELF_POINTER,
        CHILD_NOT_FOUND,
    };

    ConfEvaluator() = delete;

    explicit ConfEvaluator(std::string_view config_file_path) noexcept;

    std::expected<void, Error> load();
    std::expected<void, Error> analyzeAst() const;
    std::expected<void, Error> preProcess();

    std::expected<void, Error> visitIncludes(NodePtr& ast);
    std::expected<void, Error> visitSpliceIncludes(Node* parent, Node* me, NodePtr& splice);
    std::expected<Node*, Error> findNearestRootAncestor(Node* me);

    NodePtr const& ast() const;

private:
    NodePtr m_ast;
    PathType m_config_file_path;
};
