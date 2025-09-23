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
    struct AssignmentExpression;
    struct ShellAssignmentExpression;

    using Node = std::variant<
        RootBlock,
        NamedBlock,
        KeywordBinOp,
        AssignmentExpression,
        ShellAssignmentExpression
    >;

    using NodePtr = std::unique_ptr<Node>;

    enum class NodeKind {
        ROOT_BLOCK,
        NAMED_BLOCK,
        KEYWORD_BIN_OP,
        ASSIGNMENT_EXPRESSION,
        SHELL_ASSIGNMENT_EXPRESSION,
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

    struct AssignmentExpression {
        NodeKind kind;
        TokenType name;
        TokenType expression;
        Node* me;
        Node* parent;
    };

    struct ShellAssignmentExpression {
        NodeKind kind;
        TokenType name;
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
    };

    static std::optional<NodePtr> parseTokenList(TokenListType const& token_list);
    static bool isExpressionToken(TokenKindType token_kind);

    explicit ConfParser(TokenListType const& token_list);

    std::expected<NodePtr, Error> parse(NodePtr& parent);

    std::optional<NodePtr> takeNamedBlock(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeKeywordBinOp(TokenType const& token, NodePtr& parent);
    std::optional<NodePtr> takeAssignmentExpression(TokenType const& token, NodePtr& parent);
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
            case ROOT_BLOCK:                  return "ROOT_BLOCK";
            case NAMED_BLOCK:                 return "NAMED_BLOCK";
            case KEYWORD_BIN_OP:              return "KEYWORD_BIN_OP";
            case ASSIGNMENT_EXPRESSION:       return "ASSIGNMENT_EXPRESSION";
            case SHELL_ASSIGNMENT_EXPRESSION: return "SHELL_ASSIGNMENT_EXPRESSION";
        }
    }

    template <typename FormatContext>
    auto format(ConfParser::NodeKind kind, FormatContext& ctx) const {
        return std::formatter<std::string_view>::format(to_string(kind), ctx);
    }
};
