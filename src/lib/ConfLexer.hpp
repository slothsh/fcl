#pragma once

#include <array>
#include <expected>
#include <fstream>
#include <optional>
#include <string>
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

    static std::optional<Token> eatIdentifier(std::ifstream& stream);
    static std::optional<Token> eatLiteral(std::ifstream& stream);
    static std::optional<Token> eatSpaces(std::ifstream& stream);
    static std::optional<Token> eatPunctuator(std::ifstream& stream);
    static std::optional<Token> eatComment(std::ifstream& stream);
};
