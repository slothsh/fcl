#pragma once

#include <array>
#include <expected>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class ConfTokenizer {
public:
    using Token = Conf::Language::Token;
    using TokenKind = Conf::Language::TokenKind;

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
    static std::optional<std::pair<TokenKind, std::string>> peekNumberToken(std::ifstream& stream);
    static std::optional<std::pair<TokenKind, std::string>> peekPathToken(std::ifstream& stream);

    static constexpr std::optional<TokenKind> terminatorFor(TokenKind token_kind);
    static constexpr std::optional<std::string_view> tokenKindString(TokenKind token_kind);
    static constexpr bool isSpace(char c);
    static constexpr bool isKeywordStart(char c);
    static constexpr bool isIdentifier(char c);
    static constexpr bool isIdentifierStart(char c);
    static constexpr bool isPunctuatorStart(char c);
    static constexpr bool isStringLiteralStart(char c);
    static constexpr bool isNumberLiteral(char c);
    static constexpr bool isHexadecimalDigit(char c);
    static constexpr bool isDecimalDigit(char c);
    static constexpr bool isOctalDigit(char c);
    static constexpr bool isBinaryDigit(char c);
    static constexpr bool isCommentStart(char c);
    static constexpr bool isPathLiteralStart(char c);
    static constexpr bool isStatementTerminator(char c);
    static constexpr bool isStatementSeparator(char c);

    static std::optional<Token> eatKeyword(std::ifstream& stream);
    static std::optional<Token> eatIdentifier(std::ifstream& stream);
    static std::optional<Token> eatShellLiteral(std::ifstream& stream);
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
                terminator_kind = ConfTokenizer::terminatorFor(punctuator_kind).value_or(UNKNOWN);
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

    template<size_t N>
    static constexpr std::optional<TokenKind> peekTokenFor(std::ifstream& stream, std::array<std::pair<TokenKind, std::string_view>, N> token_list) {
        using enum TokenKind;

        size_t reset = stream.tellg();
        std::array<char, 1024> token_buffer{};

        auto punctuator_kind = UNKNOWN;
        for (auto const& [kind, chunk] : token_list) {
            stream.read(&token_buffer[0], chunk.size());
            if (std::string_view{token_buffer.data(), chunk.size()} == chunk) {
                punctuator_kind = kind;
                break;
            } else {
                stream.seekg(-chunk.size(), std::ios::cur);
            }
        }

        if (punctuator_kind == UNKNOWN) {
            stream.seekg(reset);
            return std::nullopt;
        }

        return punctuator_kind;
    }
};
