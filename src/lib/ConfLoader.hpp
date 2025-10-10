#pragma once

#include <expected>
#include <filesystem>

class ConfLoader {
public:
    using LexerType = ConfLexer;
    using ParserType = ConfParser;
    using AstType = typename ParserType::NodePtr;
    using NodeType = typename ParserType::Node;
    using NodeKindType = typename ParserType::NodeKind;
    using PathType = std::filesystem::path;
    using KeywordStatementType = typename ParserType::KeywordStatement;
    using TokenKindType = ConfParser::TokenKindType;

    enum class Error {
        FAILED_TO_LEX,
        FAILED_TO_PARSE,
        FAILED_TO_RESOLVE_INCLUDE_PATH,
        NULL_AST_POINTER,
        NULL_SELF_POINTER,
        CHILD_NOT_FOUND,
    };

    ConfLoader() = delete;

    explicit ConfLoader(std::string_view config_file_path) noexcept;

    std::expected<void, Error> load();
    std::expected<void, Error> preProcess();

    std::expected<void, Error> visitIncludes(AstType& ast);
    std::expected<void, Error> visitSpliceIncludes(NodeType* parent, NodeType* me, AstType& splice);
    std::expected<NodeType*, Error> findNearestRootAncestor(NodeType* me);

    AstType const& ast() const;

private:
    AstType m_ast;
    PathType m_config_file_path;
};
