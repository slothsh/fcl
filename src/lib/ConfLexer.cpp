#include <algorithm>
#include <cctype>
#include <cstddef>
#include <expected>
#include <fstream>
#include <ios>
#include <optional>
#include <string>
#include <string_view>

namespace detail {
    using Error = typename ConfLexer::Error;
    using ExpectedType = typename ConfLexer::ExpectedType;
    using AstType = typename ConfLexer::AstType;
    using Context = typename ConfLexer::Context;
    using TokenKind = typename ConfLexer::TokenKind;
    using Token = typename ConfLexer::Token;
}

#define PUSH_TOKEN(ast, token, context)                  \
[&ast, &token]() {                                       \
    ConfLexer::pushToken(std::move(token.value()), ast); \
    return context;                                      \
}()

detail::ExpectedType ConfLexer::lexFile(std::string_view input_file_path) {
    using enum detail::Error;
    using enum detail::Context;
    using enum detail::TokenKind;

    std::ifstream input_file(input_file_path.data());
    if (!input_file) {
        return std::unexpected(FAILED_TO_OPEN_FILE);
    };

    input_file >> std::noskipws;

    return ConfLexer::lexAst(input_file);
}

detail::ExpectedType ConfLexer::lexAst(std::ifstream& input_file) {
    using enum detail::Context;
    using enum detail::TokenKind;

    auto context = NONE;
    detail::AstType ast{};

    while (!input_file.eof()) {
        if (auto token = ConfLexer::eatSpaces(input_file)) {
            context = NONE;
        } else if (auto token = ConfLexer::eatPunctuator(input_file)) {
            context = PUSH_TOKEN(ast, token, NONE);
        } else if (auto token = ConfLexer::eatKeyword(input_file)) {
            context = PUSH_TOKEN(ast, token, NONE);
        } else if (auto token = ConfLexer::eatIdentifier(input_file)) {
            context = PUSH_TOKEN(ast, token, NONE);
        } else if (auto token = ConfLexer::eatLiteral(input_file)) {
            context = PUSH_TOKEN(ast, token, NONE);
        } else if (auto token = ConfLexer::eatComment(input_file)) {
            context = PUSH_TOKEN(ast, token, NONE);
        } else {
            WARN("unknown token: {}", static_cast<char>(input_file.peek()));
            token = Token {
                .data = std::string{static_cast<char>(input_file.peek()), 1},
                .kind = UNKNOWN,
                .position = static_cast<size_t>(input_file.tellg()),
                .length = 1,
            };
            context = PUSH_TOKEN(ast, token, NONE);
            input_file.seekg(1, std::ios::cur);
        }
    }

    return ast;
}

std::optional<detail::Error> ConfLexer::pushToken(detail::Token&& token, detail::AstType& ast) {
    ast.push_back(std::move(token));
    return std::nullopt;
}

constexpr bool ConfLexer::isSpace(char c) {
    return std::isspace(static_cast<unsigned char>(c));
}

constexpr bool ConfLexer::isKeywordStart(char c) {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

constexpr bool ConfLexer::isIdentifierStart(char c) {
    return c == '_'
        || ('A' <= c && c <= 'Z')
        || ('a' <= c && c <= 'z');
}

constexpr bool ConfLexer::isPunctuatorStart(char c) {
    return std::ranges::find_if(
        ConfLexer::PUNCTUATORS,
        [c](std::pair<TokenKind, const char*> const& pair) {
            return std::string_view{pair.second}.front() == c;
        }
    ) != ConfLexer::PUNCTUATORS.end();
}

constexpr bool ConfLexer::isIdentifier(char c) {
    return ConfLexer::isIdentifierStart(c)
        || ('0' <= c && c <= '9')
        || c == '-';
}

constexpr bool ConfLexer::isStringLiteralStart(char c) {
    return c == '\'' || c == '"';
}

constexpr bool ConfLexer::isNumberLiteralStart(char c) {
    return ('0' <= c && c <= '9');
}

constexpr bool ConfLexer::isCommentStart(char c) {
    return c == '#';
}

constexpr bool ConfLexer::isPathLiteralStart(char c) {
    return c == '.' || c == '/';
}

std::optional<detail::Token> ConfLexer::eatKeyword(std::ifstream& stream) {
    using enum TokenKind;

    if (!ConfLexer::isKeywordStart(stream.peek())) {
        return std::nullopt;
    }

    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};

    size_t reset = stream.tellg();

    while (!ConfLexer::isSpace(stream.peek())) {
        stream.read(&token_buffer[cursor++], 1);
    }

    auto const keyword = std::ranges::find_if(
        ConfLexer::KEYWORDS,
        [keyword_chunk = std::string_view{token_buffer.data(), cursor}]
        (std::pair<TokenKind, const char*> const& pair) {
            return pair.second == keyword_chunk;
        }
    );

    if (keyword == ConfLexer::KEYWORDS.end()) {
        stream.seekg(reset);
        return std::nullopt;
    }

    return Token {
        .data = std::string{token_buffer.data(), cursor},
        .kind = keyword->first,
        .position = static_cast<size_t>(stream.tellg()),
        .length = cursor,
    };
}

std::optional<detail::Token> ConfLexer::eatIdentifier(std::ifstream& stream) {
    using enum TokenKind;

    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};

    if (!ConfLexer::isIdentifierStart(stream.peek())) {
        return std::nullopt;
    }

    stream.read(&token_buffer[cursor++], 1);

    while (ConfLexer::isIdentifier(stream.peek())) {
        stream.read(&token_buffer[cursor++], 1);
    }

    return Token {
        .data = std::string{token_buffer.data(), cursor},
        .kind = IDENTIFIER,
        .position = static_cast<size_t>(stream.tellg()),
        .length = cursor,
    };
}

std::optional<detail::Token> ConfLexer::eatLiteral(std::ifstream& stream) {
    using enum TokenKind;

    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};

    auto token_kind = UNKNOWN;

    if (ConfLexer::isStringLiteralStart(stream.peek())) {
        token_kind = STRING_LITERAL;
    } else if (ConfLexer::isPathLiteralStart(stream.peek())) {
        token_kind = PATH_LITERAL;
    } else if (ConfLexer::isNumberLiteralStart(stream.peek())) {
        token_kind = NUMBER_LITERAL;
    } else {
        return std::nullopt;
    }

    stream.read(&token_buffer[cursor++], 1);

    switch (token_kind) {
        case STRING_LITERAL: {
            while (!ConfLexer::isStringLiteralStart(stream.peek())) {
                stream.read(&token_buffer[cursor++], 1);
            }

            stream.read(&token_buffer[cursor++], 1);
        } break;

        case PATH_LITERAL: {
            while (!ConfLexer::isSpace(stream.peek())) {
                stream.read(&token_buffer[cursor++], 1);
            }
        } break;

        case NUMBER_LITERAL: {
            while (!ConfLexer::isSpace(stream.peek())) {
                stream.read(&token_buffer[cursor++], 1);
            }
        } break;

        default: {
            return std::nullopt;
        }
    }

    return Token {
        .data = std::string{token_buffer.data(), cursor},
        .kind = token_kind,
        .position = static_cast<size_t>(stream.tellg()),
        .length = cursor,
    };
}

std::optional<detail::Token> ConfLexer::eatSpaces(std::ifstream& stream) {
    using enum TokenKind;

    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};
    auto token_kind = UNKNOWN;

    switch (stream.peek()) {
        case ' ': {
            stream.read(&token_buffer[cursor++], 1);
            token_kind = SPACE;
            while (stream.peek() == ' ') {
                stream.read(&token_buffer[cursor++], 1);
            }
        } break;

        case '\t': {
            stream.read(&token_buffer[cursor++], 1);
            token_kind = TAB_FEED;
            while (stream.peek() == '\t') {
                stream.read(&token_buffer[cursor++], 1);
            }
        } break;

        case '\v': {
            stream.read(&token_buffer[cursor++], 1);
            token_kind = VERTICAL_FEED;
            while (stream.peek() == '\v') {
                stream.read(&token_buffer[cursor++], 1);
            }
        } break;

        case '\n': {
            stream.read(&token_buffer[cursor++], 1);
            token_kind = LINE_FEED;
            while (stream.peek() == '\n') {
                stream.read(&token_buffer[cursor++], 1);
            }
        } break;

        default: {
            return std::nullopt;
        };
    }

    return Token {
        .data = std::string{token_buffer.data(), cursor},
        .kind = token_kind,
        .position = static_cast<size_t>(stream.tellg()),
        .length = cursor,
    };
}

std::optional<detail::Token> ConfLexer::eatPunctuator(std::ifstream& stream) {
    using enum TokenKind;

    if (!ConfLexer::isPunctuatorStart(stream.peek())) {
        return std::nullopt;
    }

    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};

    size_t reset = stream.tellg();

    while (!ConfLexer::isSpace(stream.peek())) {
        stream.read(&token_buffer[cursor++], 1);
    }

    auto const punctuator = std::ranges::find_if(
        ConfLexer::PUNCTUATORS,
        [punctuator_chunk = std::string_view{token_buffer.data(), cursor}]
        (std::pair<TokenKind, const char*> const& pair) {
            return pair.second == punctuator_chunk;
        }
    );

    if (punctuator == ConfLexer::PUNCTUATORS.end()) {
        stream.seekg(reset);
        return std::nullopt;
    }

    return Token {
        .data = std::string{token_buffer.data(), cursor},
        .kind = punctuator->first,
        .position = static_cast<size_t>(stream.tellg()),
        .length = cursor,
    };
}

std::optional<detail::Token> ConfLexer::eatComment(std::ifstream& stream) {
    using enum TokenKind;

    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};

    if (!ConfLexer::isCommentStart(stream.peek())) {
        return std::nullopt;
    }

    stream.read(&token_buffer[cursor++], 1);

    while (stream.peek() != '\n') {
        stream.read(&token_buffer[cursor++], 1);
    }

    return Token {
        .data = std::string{token_buffer.data(), cursor},
        .kind = COMMENT,
        .position = static_cast<size_t>(stream.tellg()),
        .length = cursor,
    };
}

#undef PUSH_BUFFER
