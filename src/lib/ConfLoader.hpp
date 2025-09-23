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
    using TokenKindType = ConfParser::TokenKindType;

    enum class Error {
        FAILED_TO_LEX,
        FAILED_TO_PARSE,
        NULL_AST_POINTER,
        CHILD_NOT_FOUND,
    };

    ConfLoader() = delete;

    explicit ConfLoader(std::string_view config_file_path) noexcept;

    std::expected<void, Error> load();
    std::expected<void, Error> preProcess();

    std::expected<void, Error> visitIncludes(AstType& ast);
    std::expected<void, Error> visitSpliceIncludes(NodeType* parent, NodeType* me, AstType& splice);

    AstType const& ast();

private:
    AstType m_ast;
    PathType m_config_file_path;
};
