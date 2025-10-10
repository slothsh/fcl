#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <variant>

class ConfParser {
public:
    using TokenType = typename ConfLexer::Token;
    using TokenKindType = typename ConfLexer::TokenKind;
    using TokenListType = typename ConfLexer::TokenListType;
    using NumberType = typename Conf::NumberType;
    using PathType = std::filesystem::path;

    struct FilePathRootBlock;
    struct FilePathSubRootBlock;
    struct NamedBlock;
    struct KeywordStatement;
    struct VariableAssignmentExpression;
    struct ConstantAssignmentExpression;
    struct StringExpression;
    struct NumberExpression;
    struct PathExpression;
    struct ShellExpression;

    using Node = std::variant<
        FilePathRootBlock,
        FilePathSubRootBlock,
        NamedBlock,
        KeywordStatement,
        VariableAssignmentExpression,
        ConstantAssignmentExpression,
        StringExpression,
        NumberExpression,
        PathExpression,
        ShellExpression
    >;

    using NodePtr = std::unique_ptr<Node>;

    enum class NodeKind {
        FILE_PATH_ROOT_BLOCK,
        FILE_PATH_SUB_ROOT_BLOCK,
        NAMED_BLOCK,
        KEYWORD_STATEMENT,
        VARIABLE_ASSIGNMENT_EXPRESSION,
        CONSTANT_ASSIGNMENT_EXPRESSION,
        STRING_EXPRESSION,
        NUMBER_EXPRESSION,
        PATH_EXPRESSION,
        SHELL_EXPRESSION,
    };

    struct FilePathRootBlock {
        NodeKind kind;
        std::vector<NodePtr> nodes;
        PathType file_path;
        Node* me;
    };

    struct FilePathSubRootBlock {
        NodeKind kind;
        std::vector<NodePtr> nodes;
        PathType file_path;
        Node* me;
        Node* parent;
    };

    struct NamedBlock {
        NodeKind kind;
        TokenType name;
        std::vector<NodePtr> nodes;
        Node* me;
        Node* parent;
    };

    struct KeywordStatement {
        NodeKind kind;
        TokenType keyword;
        std::vector<NodePtr> arguments;
        Node* me;
        Node* parent;
    };

    struct VariableAssignmentExpression {
        NodeKind kind;
        TokenType name;
        NodePtr expression;
        Node* me;
        Node* parent;
    };

    struct ConstantAssignmentExpression {
        NodeKind kind;
        TokenType name;
        NodePtr expression;
        Node* me;
        Node* parent;
    };

    struct StringExpression {
        NodeKind kind;
        TokenType token;
        Node* me;
        Node* parent;
    };

    struct NumberExpression {
        NodeKind kind;
        NumberType value;
        TokenType token;
        Node* me;
        Node* parent;
    };

    struct PathExpression {
        NodeKind kind;
        PathType path;
        TokenType token;
        Node* me;
        Node* parent;
    };

    struct ShellExpression {
        NodeKind kind;
        TokenType command;
        Node* me;
        Node* parent;
    };

    enum class Error {
        TODO,
        NUMBER_CONVERSION_ERROR,
    };

    static std::optional<NodePtr> parseTokenListWithFilePathRoot(TokenListType const& token_list, std::string_view file_path);
    static std::optional<NodePtr> parseTokenListWithFilePathSubRoot(TokenListType const& token_list, std::string_view file_path);
    static std::expected<NumberType, Error> convertTokenToNumber(TokenType const& token) noexcept;

    explicit ConfParser(TokenListType const& token_list);

    std::expected<NodePtr, Error> parse(NodePtr& parent);

    std::optional<NodePtr> takeNamedBlock(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeKeywordStatement(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeVariableAssignmentExpression(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeConstantAssignmentExpression(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeStringExpression(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeNumberExpression(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takePathExpression(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeShellExpression(TokenType const& token, NodePtr& parent);

private:
    size_t m_cursor;
    TokenListType const& m_token_list;
};


template <>
struct std::formatter<ConfParser::NodeKind> : std::formatter<std::string_view> {
    using enum ConfParser::NodeKind;
static constexpr std::string_view to_string(ConfParser::NodeKind kind) {
        switch (kind) {
            case FILE_PATH_ROOT_BLOCK:           return "FILE_PATH_ROOT_BLOCK";
            case FILE_PATH_SUB_ROOT_BLOCK:       return "FILE_PATH_SUB_ROOT_BLOCK";
            case NAMED_BLOCK:                    return "NAMED_BLOCK";
            case KEYWORD_STATEMENT:              return "KEYWORD_STATEMENT";
            case VARIABLE_ASSIGNMENT_EXPRESSION: return "VARIABLE_ASSIGNMENT_EXPRESSION";
            case CONSTANT_ASSIGNMENT_EXPRESSION: return "CONSTANT_ASSIGNMENT_EXPRESSION";
            case STRING_EXPRESSION:              return "STRING_EXPRESSION";
            case NUMBER_EXPRESSION:              return "NUMBER_EXPRESSION";
            case PATH_EXPRESSION:                return "PATH_EXPRESSION";
            case SHELL_EXPRESSION:               return "SHELL_EXPRESSION";
        }
    }

    template <typename FormatContext>
    auto format(ConfParser::NodeKind kind, FormatContext& ctx) const {
        return std::formatter<std::string_view>::format(to_string(kind), ctx);
    }
};
