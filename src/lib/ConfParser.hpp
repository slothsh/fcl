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

    struct NamedBlock;
    struct KeywordBinOp;
    struct NamedDeclaration;

    using Node = std::variant<
        NamedBlock,
        KeywordBinOp,
        NamedDeclaration
    >;
    using NodePtr = std::unique_ptr<Node>;

    enum class NodeKind {
        NAMED_BLOCK,
        KEYWORD_BIN_OP,
        NAMED_DECLARATION,
    };

    struct NamedBlock {
        NodeKind kind;
        TokenType name;
        std::vector<NodePtr> nodes;
        // TODO: file location info
    };

    struct KeywordBinOp {
        NodeKind kind;
        TokenType keyword;
        TokenType expression;
        // TODO: file location info
    };

    struct NamedDeclaration {
        NodeKind kind;
        TokenType name;
        TokenType expression;
    };
    
    enum class Error {
        TODO,
    };

    static constexpr std::array EXPRESSION_TOKEN_KINDS {
        TokenKindType::STRING_LITERAL,
        TokenKindType::NUMBER_LITERAL,
        TokenKindType::PATH_LITERAL,
    };

    static std::optional<NodePtr> parse(TokenListType const& token_list);
    static bool isExpressionToken(TokenKindType token_kind);

    explicit ConfParser(TokenListType const& token_list);

    std::expected<NodePtr, Error> parse();

    std::optional<NodePtr> takeNamedBlock(TokenType const& token);
    std::optional<NodePtr> takeKeywordBinOp(TokenType const& token);
    std::optional<NodePtr> takeNamedDeclaration(TokenType const& token);

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
            case NAMED_BLOCK:       return "NAMED_BLOCK";
            case KEYWORD_BIN_OP:    return "KEYWORD_BIN_OP";
            case NAMED_DECLARATION: return "NAMED_DECLARATION";
        }
    }

    template <typename FormatContext>
    auto format(ConfParser::NodeKind kind, FormatContext& ctx) const {
        return std::formatter<std::string_view>::format(to_string(kind), ctx);
    }
};
