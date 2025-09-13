#pragma once

#include <array>
#include <expected>
#include <format>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class ConfLexer {
public:
    enum class TokenKind {
        UNKNOWN,
        EQUALS,
        IDENTIFIER,
        NUMBER_LITERAL,
        STRING_LITERAL,
        PATH_LITERAL,
        OPEN_BRACE,
        CLOSE_BRACE,
        TAB_FEED,
        LINE_FEED,
        VERTICAL_FEED,
        SPACE,
        COMMENT,
    };

    static constexpr std::array PUNCTUATORS { '=', '{', '}' };

    struct Token {
        std::string data;
        TokenKind kind;
        size_t position;
        size_t length;
    };

    enum class Context {
        NONE,
        SYMBOL,
        LITERAL,
        PUNCTUATOR,
    };

    enum class Error {
        FAILED_TO_OPEN_FILE,
        BUFFER_OUT_OF_MEMORY,
    };

    using AstType = std::vector<Token>;
    using ExpectedType = std::expected<AstType, Error>;

    static ExpectedType lexFile(std::string_view input_file_path);
    static ExpectedType lexAst(std::ifstream& input_file);

    static std::optional<Error> pushToken(Token&& token, AstType& ast);

    static constexpr bool isSpace(char c);
    static constexpr bool isIdentifier(char c);
    static constexpr bool isIdentifierStart(char c);
    static constexpr bool isStringLiteralStart(char c);
    static constexpr bool isNumberLiteralStart(char c);
    static constexpr bool isNumberLiteral(char c);
    static constexpr bool isCommentStart(char c);
    static constexpr bool isPathLiteralStart(char c);

    static std::optional<Token> eatIdentifier(std::ifstream& stream);
    static std::optional<Token> eatLiteral(std::ifstream& stream);
    static std::optional<Token> eatSpaces(std::ifstream& stream);
    static std::optional<Token> eatPunctuator(std::ifstream& stream);
    static std::optional<Token> eatComment(std::ifstream& stream);
};

template <>
struct std::formatter<ConfLexer::TokenKind> : std::formatter<std::string_view> {
    using enum ConfLexer::TokenKind;

    static constexpr std::string_view to_string(ConfLexer::TokenKind kind) {
        switch (kind) {
            case UNKNOWN:         return "UNKNOWN";
            case IDENTIFIER:      return "IDENTIFIER";
            case EQUALS:          return "EQUALS";
            case NUMBER_LITERAL:  return "NUMBER";
            case STRING_LITERAL:  return "STRING_LITERAL";
            case PATH_LITERAL:    return "PATH_LITERAL";
            case COMMENT:         return "COMMENT";
            case OPEN_BRACE:      return "OPEN_BRACE";
            case CLOSE_BRACE:     return "CLOSE_BRACE";
            case TAB_FEED:        return "TAB_FEED";
            case LINE_FEED:       return "LINE_FEED";
            case VERTICAL_FEED:   return "VERTICAL_FEED";
            case SPACE:           return "SPACE";
        }
    }

    template <typename FormatContext>
    auto format(ConfLexer::TokenKind kind, FormatContext& ctx) const {
        return std::formatter<std::string_view>::format(to_string(kind), ctx);
    }
};
