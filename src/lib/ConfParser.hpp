#pragma once

#include <expected>
#include <memory>
#include <optional>
#include <variant>

class ConfParser {
public:
    using TokenType = typename ConfLexer::Token;
    using TokenKindType = typename ConfLexer::TokenKind;
    using TokenListType = typename ConfLexer::TokenListType;

    struct RootBlock;
    struct NamedBlock;
    struct KeywordBinOp;
    struct VariableAssignmentExpression;
    struct ConstantAssignmentExpression;
    struct StringExpression;
    struct NumberExpression;
    struct PathExpression;
    struct ShellExpression;

    using Node = std::variant<
        RootBlock,
        NamedBlock,
        KeywordBinOp,
        VariableAssignmentExpression,
        ConstantAssignmentExpression,
        StringExpression,
        NumberExpression,
        PathExpression,
        ShellExpression
    >;

    using NodePtr = std::unique_ptr<Node>;

    enum class NodeKind {
        ROOT_BLOCK,
        NAMED_BLOCK,
        KEYWORD_BIN_OP,
        VARIABLE_ASSIGNMENT_EXPRESSION,
        CONSTANT_ASSIGNMENT_EXPRESSION,
        STRING_EXPRESSION,
        NUMBER_EXPRESSION,
        PATH_EXPRESSION,
        SHELL_EXPRESSION,
    };

    struct RootBlock {
        NodeKind kind;
        std::vector<NodePtr> nodes;
        Node* me;
    };

    struct NamedBlock {
        NodeKind kind;
        TokenType name;
        std::vector<NodePtr> nodes;
        Node* me;
        Node* parent;
    };

    struct KeywordBinOp {
        NodeKind kind;
        TokenType keyword;
        TokenType expression;
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
        TokenType token;
        Node* me;
        Node* parent;
    };

    struct PathExpression {
        NodeKind kind;
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
    };

    static constexpr std::array EXPRESSION_TOKEN_KINDS {
        TokenKindType::STRING_LITERAL,
        TokenKindType::NUMBER_LITERAL,
        TokenKindType::PATH_LITERAL,
        TokenKindType::SHELL_LITERAL,
    };

    static std::optional<NodePtr> parseTokenList(TokenListType const& token_list);
    static bool isExpressionToken(TokenKindType token_kind);

    explicit ConfParser(TokenListType const& token_list);

    std::expected<NodePtr, Error> parse(NodePtr& parent);

    std::optional<NodePtr> takeNamedBlock(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeKeywordBinOp(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeVariableAssignmentExpression(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeConstantAssignmentExpression(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeStringExpression(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeNumberExpression(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takePathExpression(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeShellExpression(TokenType const& token, NodePtr& parent);

private:
    size_t m_cursor;
    TokenListType const& m_token_list;
    NodePtr m_root;
};


template <>
struct std::formatter<ConfParser::NodeKind> : std::formatter<std::string_view> {
    using enum ConfParser::NodeKind;

    static constexpr std::string_view to_string(ConfParser::NodeKind kind) {
        switch (kind) {
            case ROOT_BLOCK:                     return "ROOT_BLOCK";
            case NAMED_BLOCK:                    return "NAMED_BLOCK";
            case KEYWORD_BIN_OP:                 return "KEYWORD_BIN_OP";
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
