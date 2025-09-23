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
        WALRUS,
        SEMI_COLON,
        IDENTIFIER,
        NUMBER_LITERAL,
        STRING_LITERAL,
        PATH_LITERAL,
        OPEN_BRACE,
        CLOSE_BRACE,
        OPEN_DOUBLE_BRACE,
        CLOSE_DOUBLE_BRACE,
        OPEN_QUOTE,
        CLOSE_QUOTE,
        OPEN_DOUBLE_QUOTE,
        CLOSE_DOUBLE_QUOTE,
        TAB_FEED,
        LINE_FEED,
        VERTICAL_FEED,
        SPACE,
        COMMENT,
        KEYWORD_INCLUDE,
        SHELL_EXPRESSION,
    };

    static constexpr std::array KEYWORDS {
        std::pair{ TokenKind::KEYWORD_INCLUDE, Conf::STRING_KEYWORD_INCLUDE },
    };

    // TODO: separate into open/close punctuators
    static constexpr std::array PUNCTUATORS {
        std::pair{ TokenKind::EQUALS, Conf::STRING_EQUALS },
        std::pair{ TokenKind::WALRUS, Conf::STRING_WALRUS },
        std::pair{ TokenKind::SEMI_COLON, Conf::STRING_SEMI_COLON },
        std::pair{ TokenKind::OPEN_BRACE, Conf::STRING_OPEN_BRACE },
        std::pair{ TokenKind::CLOSE_BRACE, Conf::STRING_CLOSE_BRACE },
    };

    static constexpr std::array STATEMENT_TERMINATORS {
        std::pair{ TokenKind::SEMI_COLON, Conf::STRING_SEMI_COLON },
    };

    static constexpr std::array STRING_OPEN_PUNCTUATORS {
        std::pair{ TokenKind::OPEN_QUOTE, Conf::STRING_OPEN_QUOTE },
        std::pair{ TokenKind::OPEN_DOUBLE_QUOTE, Conf::STRING_OPEN_DOUBLE_QUOTE },
    };

    static constexpr std::array STRING_CLOSE_PUNCTUATORS {
        std::pair{ TokenKind::CLOSE_QUOTE, Conf::STRING_CLOSE_QUOTE },
        std::pair{ TokenKind::CLOSE_DOUBLE_QUOTE, Conf::STRING_CLOSE_DOUBLE_QUOTE },
    };

    static constexpr std::array SHELL_EXPRESSION_OPEN_PUNCTUATORS {
        std::pair{ TokenKind::OPEN_DOUBLE_BRACE, Conf::STRING_OPEN_DOUBLE_BRACE },
    };

    static constexpr std::array SHELL_EXPRESSION_CLOSE_PUNCTUATORS {
        std::pair{ TokenKind::CLOSE_DOUBLE_BRACE, Conf::STRING_CLOSE_DOUBLE_BRACE },
    };

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

    using TokenListType = std::vector<Token>;
    using ExpectedType = std::expected<TokenListType, Error>;

    static ExpectedType lexFile(std::string_view input_file_path);
    static ExpectedType lexInputFileStream(std::ifstream& input_file);

    static std::optional<Error> pushToken(Token&& token, TokenListType& ast);
    static std::optional<std::string_view> peekTokenKind(std::ifstream& stream, TokenKind token_kind);
    static std::optional<char> peekEscapedCharacter(std::ifstream& stream);

    static constexpr std::optional<TokenKind> terminatorFor(TokenKind token_kind);
    static constexpr std::optional<std::string_view> tokenKindString(TokenKind token_kind);
    static constexpr bool isSpace(char c);
    static constexpr bool isKeywordStart(char c);
    static constexpr bool isIdentifier(char c);
    static constexpr bool isIdentifierStart(char c);
    static constexpr bool isPunctuatorStart(char c);
    static constexpr bool isStringLiteralStart(char c);
    static constexpr bool isNumberLiteralStart(char c);
    static constexpr bool isNumberLiteral(char c);
    static constexpr bool isCommentStart(char c);
    static constexpr bool isPathLiteralStart(char c);
    static constexpr bool isStatementTerminator(char c);

    static std::optional<Token> eatKeyword(std::ifstream& stream);
    static std::optional<Token> eatIdentifier(std::ifstream& stream);
    static std::optional<Token> eatShellExpression(std::ifstream& stream);
    static std::optional<Token> eatLiteral(std::ifstream& stream);
    static std::optional<Token> eatSpaces(std::ifstream& stream);
    static std::optional<Token> eatPunctuator(std::ifstream& stream);
    static std::optional<Token> eatComment(std::ifstream& stream);

    template<size_t N>
    static constexpr std::optional<std::pair<TokenKind, TokenKind>> peekDelimitersFor(std::ifstream& stream, std::array<std::pair<TokenKind, std::string_view>, N> const& open_delimiters) {
        using enum TokenKind;

        size_t reset = stream.tellg();
        std::array<char, 1024> token_buffer{};

        auto punctuator_kind = UNKNOWN;
        auto terminator_kind = UNKNOWN;
        for (auto const& [kind, chunk] : open_delimiters) {
            stream.read(&token_buffer[0], chunk.size());
            if (std::string_view{token_buffer.data(), chunk.size()} == chunk) {
                punctuator_kind = kind;
                terminator_kind = ConfLexer::terminatorFor(punctuator_kind).value_or(UNKNOWN);
                break;
            } else {
                stream.seekg(-chunk.size(), std::ios::cur);
            }
        }

        if (punctuator_kind == UNKNOWN) {
            stream.seekg(reset);
            return std::nullopt;
        }

        if (terminator_kind == UNKNOWN) {
            stream.seekg(reset);
            return std::nullopt;
        }

        return std::make_pair(punctuator_kind, terminator_kind);
    }
};

template <>
struct std::formatter<ConfLexer::TokenKind> : std::formatter<std::string_view> {
    using enum ConfLexer::TokenKind;

    static constexpr std::string_view to_string(ConfLexer::TokenKind kind) {
        switch (kind) {
            case UNKNOWN:            return "UNKNOWN";
            case IDENTIFIER:         return "IDENTIFIER";
            case EQUALS:             return "EQUALS";
            case WALRUS:             return "WALRUS";
            case SEMI_COLON:         return "SEMI_COLON";
            case NUMBER_LITERAL:     return "NUMBER";
            case STRING_LITERAL:     return "STRING_LITERAL";
            case PATH_LITERAL:       return "PATH_LITERAL";
            case COMMENT:            return "COMMENT";
            case OPEN_BRACE:         return "OPEN_BRACE";
            case CLOSE_BRACE:        return "CLOSE_BRACE";
            case OPEN_DOUBLE_BRACE:  return "OPEN_DOUBLE_BRACE";
            case CLOSE_DOUBLE_BRACE: return "CLOSE_DOUBLE_BRACE";
            case OPEN_QUOTE:         return "OPEN_QUOTE";
            case CLOSE_QUOTE:        return "CLOSE_QUOTE";
            case OPEN_DOUBLE_QUOTE:  return "OPEN_DOUBLE_QUOTE";
            case CLOSE_DOUBLE_QUOTE: return "CLOSE_DOUBLE_QUOTE";
            case TAB_FEED:           return "TAB_FEED";
            case LINE_FEED:          return "LINE_FEED";
            case VERTICAL_FEED:      return "VERTICAL_FEED";
            case SPACE:              return "SPACE";
            case KEYWORD_INCLUDE:    return "KEYWORD_INCLUDE";
            case SHELL_EXPRESSION:   return "SHELL_EXPRESSION";
        }
    }

    template <typename FormatContext>
    auto format(ConfLexer::TokenKind kind, FormatContext& ctx) const {
        return std::formatter<std::string_view>::format(to_string(kind), ctx);
    }
};
