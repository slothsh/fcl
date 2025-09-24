#include <algorithm>
#include <cctype>
#include <cstddef>
#include <expected>
#include <fstream>
#include <ios>
#include <optional>
#include <ranges>
#include <ranges>
#include <string>
#include <string_view>

namespace detail {
    using Error = typename ConfLexer::Error;
    using ExpectedType = typename ConfLexer::ExpectedType;
    using TokenListType = typename ConfLexer::TokenListType;
    using Context = typename ConfLexer::Context;
    using TokenKind = typename ConfLexer::TokenKind;
    using Token = typename ConfLexer::Token;

    template<typename S>
    constexpr auto makeStreamIncrementor(S& stream) {
        return [&stream](std::string_view str) {
            stream.seekg(str.size(), std::ios::cur);
            return std::make_optional(str);
        };
    }
}

#define PUSH_TOKEN(token_list, token, context)                  \
[&token_list, &token]() {                                       \
    ConfLexer::pushToken(std::move(token.value()), token_list); \
    return context;                                             \
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

    return ConfLexer::lexInputFileStream(input_file);
}

detail::ExpectedType ConfLexer::lexInputFileStream(std::ifstream& input_file) {
    using enum detail::Context;
    using enum detail::TokenKind;

    auto context = NONE;
    detail::TokenListType token_list{};

    while (!input_file.eof()) {
        if (auto token = ConfLexer::eatSpaces(input_file)) {
            context = NONE;
        } else if (auto token = ConfLexer::eatKeyword(input_file)) {
            context = PUSH_TOKEN(token_list, token, NONE);
        } else if (auto token = ConfLexer::eatIdentifier(input_file)) {
            context = PUSH_TOKEN(token_list, token, NONE);
        } else if (auto token = ConfLexer::eatShellLiteral(input_file)) {
            context = PUSH_TOKEN(token_list, token, NONE);
        } else if (auto token = ConfLexer::eatLiteral(input_file)) {
            context = PUSH_TOKEN(token_list, token, NONE);
        } else if (auto token = ConfLexer::eatComment(input_file)) {
            context = PUSH_TOKEN(token_list, token, NONE);
        } else if (auto token = ConfLexer::eatPunctuator(input_file)) {
            context = PUSH_TOKEN(token_list, token, NONE);
        } else {
            WARN("unknown token: {}", static_cast<char>(input_file.peek()));
            token = Token {
                .data = std::string{static_cast<char>(input_file.peek()), 1},
                .kind = UNKNOWN,
                .position = static_cast<size_t>(input_file.tellg()),
                .length = 1,
            };
            context = PUSH_TOKEN(token_list, token, NONE);
            input_file.seekg(1, std::ios::cur);
        }
    }

    return token_list;
}

std::optional<detail::Error> ConfLexer::pushToken(detail::Token&& token, detail::TokenListType& token_list) {
    token_list.push_back(std::move(token));
    return std::nullopt;
}

constexpr std::optional<detail::TokenKind> ConfLexer::terminatorFor(detail::TokenKind token_kind) {
    using enum TokenKind;

    switch (token_kind) {
        case OPEN_BRACE:        return CLOSE_BRACE;
        case OPEN_DOUBLE_BRACE: return CLOSE_DOUBLE_BRACE;
        case OPEN_QUOTE:        return CLOSE_QUOTE;
        case OPEN_DOUBLE_QUOTE: return CLOSE_DOUBLE_QUOTE;
        default: return std::nullopt;
    }
}


std::optional<std::string_view> ConfLexer::peekTokenKind(std::ifstream& stream, detail::TokenKind token_kind) {
    size_t reset = stream.tellg();

    std::array<char, 32> token_buffer{};

    auto token_kind_string = ConfLexer::tokenKindString(token_kind);
    if (!token_kind_string) {
        return std::nullopt;
    }

    stream.read(&token_buffer[0], token_kind_string->size());
    stream.seekg(reset);

    if (std::string{token_buffer.data(), token_kind_string->size()} == token_kind_string) {
        return token_kind_string;
    }

    return std::nullopt;
}

std::optional<char> ConfLexer::peekEscapedCharacter(std::ifstream& stream) {
    size_t reset = stream.tellg();

    std::array<char, 2> token_buffer{};

    char c = stream.peek();
    if (std::string_view{&c, 1} == Conf::STRING_ESCAPE_SEQUENCE) {
        stream.seekg(1, std::ios::cur);
        c = stream.peek();
        stream.seekg(1, std::ios::cur);
        return c;
    }

    return std::nullopt;
}

constexpr std::optional<std::string_view> ConfLexer::tokenKindString(TokenKind token_kind) {
    using namespace Conf;
    using enum TokenKind;

    switch (token_kind) {
        // Defined tokens
        case EQUALS:             return STRING_EQUALS;
        case WALRUS:             return STRING_WALRUS;
        case SEMI_COLON:         return STRING_SEMI_COLON;
        case COMMA:              return STRING_COMMA;
        case OPEN_BRACE:         return STRING_OPEN_BRACE;
        case CLOSE_BRACE:        return STRING_CLOSE_BRACE;
        case OPEN_DOUBLE_BRACE:  return STRING_OPEN_DOUBLE_BRACE;
        case CLOSE_DOUBLE_BRACE: return STRING_CLOSE_DOUBLE_BRACE;
        case OPEN_DOUBLE_QUOTE:  return STRING_OPEN_DOUBLE_QUOTE;
        case CLOSE_DOUBLE_QUOTE: return STRING_CLOSE_DOUBLE_QUOTE;
        case OPEN_QUOTE:         return STRING_OPEN_QUOTE;
        case CLOSE_QUOTE:        return STRING_CLOSE_QUOTE;
        case TAB_FEED:           return STRING_TAB_FEED;
        case LINE_FEED:          return STRING_LINE_FEED;
        case VERTICAL_FEED:      return STRING_VERTICAL_FEED;
        case SPACE:              return STRING_SPACE;
        case KEYWORD_INCLUDE:    return STRING_KEYWORD_INCLUDE;

        // Dynamic tokens
        case IDENTIFIER:
        case COMMENT:
        case NUMBER_LITERAL:
        case STRING_LITERAL:
        case PATH_LITERAL:
        case SHELL_LITERAL:
        case UNKNOWN:            return std::nullopt;
    }
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
        [c](std::pair<TokenKind, std::string_view> const& pair) {
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
    auto const char_string = std::string_view{&c, 1};
    return char_string == Conf::STRING_OPEN_QUOTE || char_string == Conf::STRING_OPEN_DOUBLE_QUOTE;
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

constexpr bool ConfLexer::isStatementTerminator(char c) {
    return std::ranges::find_if(
        ConfLexer::STATEMENT_TERMINATORS,
        [c](std::pair<TokenKind, std::string_view> const& pair) {
            return std::string_view{pair.second}.front() == c;
        }
    ) != ConfLexer::STATEMENT_TERMINATORS.end();
}

constexpr bool ConfLexer::isStatementSeparator(char c) {
    return std::ranges::find_if(
        ConfLexer::STATEMENT_SEPARATORS,
        [c](std::pair<TokenKind, std::string_view> const& pair) {
            return std::string_view{pair.second}.front() == c;
        }
    ) != ConfLexer::STATEMENT_SEPARATORS.end();
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
        (std::pair<TokenKind, std::string_view> const& pair) {
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

std::optional<detail::Token> ConfLexer::eatShellLiteral(std::ifstream& stream) {
    using enum TokenKind;

    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};
    size_t reset = stream.tellg();

    auto const delimiters = ConfLexer::peekDelimitersFor(stream, ConfLexer::SHELL_LITERAL_OPEN_PUNCTUATORS);
    if (!delimiters) {
        return std::nullopt;
    }

    auto const& [_, terminator_kind] = delimiters.value();
    auto const increment_stream = detail::makeStreamIncrementor(stream);

    while (!ConfLexer::peekTokenKind(stream, terminator_kind).and_then(increment_stream)) {
        if (auto escaped_token = ConfLexer::peekEscapedCharacter(stream)) {
            std::ranges::copy(std::string_view{&escaped_token.value(), 1}, token_buffer.begin() + cursor);
            cursor += sizeof(char);
            continue;
        }

        stream.read(&token_buffer[cursor++], 1);
    }

    auto data = std::string_view{token_buffer.data(), cursor}
        | std::views::drop_while(ConfLexer::isSpace)
        | std::views::reverse
        | std::views::drop_while(ConfLexer::isSpace)
        | std::views::reverse
        | std::ranges::to<std::string>();

    return Token {
        .data = std::move(data),
        .kind = SHELL_LITERAL,
        .position = static_cast<size_t>(stream.tellg()),
        .length = cursor,
    };
}

std::optional<detail::Token> ConfLexer::eatLiteral(std::ifstream& stream) {
    using enum TokenKind;

    size_t reset = stream.tellg();
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

    switch (token_kind) {
        case STRING_LITERAL: {
            auto const delimiters = ConfLexer::peekDelimitersFor(stream, ConfLexer::STRING_LITERAL_OPEN_PUNCTUATORS);
            if (!delimiters) {
                return std::nullopt;
            }

            auto const& [_, terminator_kind] = delimiters.value();
            auto const increment_stream = detail::makeStreamIncrementor(stream);

            bool escape_next = false;
            while (!ConfLexer::peekTokenKind(stream, terminator_kind).and_then(increment_stream)) {
                if (auto escaped_token = ConfLexer::peekEscapedCharacter(stream)) {
                    std::ranges::copy(std::string_view{&escaped_token.value(), 1}, token_buffer.begin() + cursor);
                    cursor += sizeof(char);
                    continue;
                }

                stream.read(&token_buffer[cursor++], 1);
            }
        } break;

        case PATH_LITERAL: {
            stream.read(&token_buffer[cursor++], 1);
            auto const valid_next_token = [](char c) {
                return !ConfLexer::isSpace(c)
                    && !ConfLexer::isStatementTerminator(c)
                    && !ConfLexer::isStatementSeparator(c);
            };

            while (valid_next_token(stream.peek())) {
                if (auto escaped_token = ConfLexer::peekEscapedCharacter(stream)) {
                    std::ranges::copy(std::string_view{&escaped_token.value(), 1}, token_buffer.begin() + cursor);
                    cursor += sizeof(char);
                    continue;
                }

                if (ConfLexer::peekTokenKind(stream, LINE_FEED)) {
                    stream.seekg(reset);
                    return std::nullopt;
                }

                stream.read(&token_buffer[cursor++], 1);
            }
        } break;

        case NUMBER_LITERAL: {
            stream.read(&token_buffer[cursor++], 1);
            auto const valid_next_token = [](char c) {
                return !ConfLexer::isSpace(c)
                    && !ConfLexer::isStatementTerminator(c)
                    && !ConfLexer::isStatementSeparator(c);
            };

            while (valid_next_token(stream.peek())) {
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

    auto punctuator = ConfLexer::peekTokenFor(stream, ConfLexer::PUNCTUATORS);
    if (!punctuator) {
        return std::nullopt;
    }

    auto const punctuator_string = ConfLexer::tokenKindString(punctuator.value());
    if (!punctuator_string) {
        return std::nullopt;
    }

    return Token {
        .data = punctuator_string.value().data(),
        .kind = punctuator.value(),
        .position = static_cast<size_t>(stream.tellg()),
        .length = punctuator_string.value().size(),
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
