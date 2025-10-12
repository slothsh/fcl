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

#define PUSH_TOKEN(token_list, token)                           \
    ConfTokenizer::pushToken(std::move((token)), (token_list));

inline namespace {
    using namespace Conf;
    using namespace Conf::Language;
    using enum ConfTokenizer::TokenKind;
    using enum ConfTokenizer::Error;

    template<typename S>
    constexpr auto makeStreamIncrementor(S& stream) {
        return [&stream](std::string_view str) {
            stream.seekg(str.size(), std::ios::cur);
            return std::make_optional(str);
        };
    }
}

ConfTokenizer::ExpectedType ConfTokenizer::tokenizeFile(std::string_view input_file_path) {
    std::ifstream input_file(input_file_path.data());
    if (!input_file) {
        return std::unexpected(FAILED_TO_OPEN_FILE);
    };

    input_file >> std::noskipws;

    return ConfTokenizer::tokenizeInputFileStream(input_file);
}

ConfTokenizer::ExpectedType ConfTokenizer::tokenizeInputFileStream(std::ifstream& input_file) {
    TokenListType token_list{};

    while (!input_file.eof()) {
        if (auto token = ConfTokenizer::eatSpaces(input_file)) {
        } else if (auto token = ConfTokenizer::eatComment(input_file)) {
            PUSH_TOKEN(token_list, token.value());
        } else if (auto token = ConfTokenizer::eatKeyword(input_file)) {
            PUSH_TOKEN(token_list, token.value());
        } else if (auto token = ConfTokenizer::eatShellLiteral(input_file)) {
            PUSH_TOKEN(token_list, token.value());
        } else if (auto token = ConfTokenizer::eatLiteral(input_file)) {
            PUSH_TOKEN(token_list, token.value());
        } else if (auto token = ConfTokenizer::eatIdentifier(input_file)) {
            PUSH_TOKEN(token_list, token.value());
        } else if (auto token = ConfTokenizer::eatPunctuator(input_file)) {
            PUSH_TOKEN(token_list, token.value());
        } else {
            WARN("unknown token: {}", static_cast<char>(input_file.peek()));
            token = Token {
                .data = std::string{static_cast<char>(input_file.peek()), 1},
                .kind = UNKNOWN,
                .position = static_cast<size_t>(input_file.tellg()),
                .length = 1,
            };
            PUSH_TOKEN(token_list, token.value());
            input_file.seekg(1, std::ios::cur);
        }
    }

    return token_list;
}

std::optional<ConfTokenizer::Error> ConfTokenizer::pushToken(Token&& token, TokenListType& token_list) {
    token_list.push_back(std::move(token));
    return std::nullopt;
}

constexpr std::optional<TokenKind> ConfTokenizer::terminatorFor(TokenKind token_kind) {
    switch (token_kind) {
        case OPEN_BRACE:        return CLOSE_BRACE;
        case OPEN_DOUBLE_BRACE: return CLOSE_DOUBLE_BRACE;
        case OPEN_QUOTE:        return CLOSE_QUOTE;
        case OPEN_DOUBLE_QUOTE: return CLOSE_DOUBLE_QUOTE;
        default: return std::nullopt;
    }
}

std::optional<std::string_view> ConfTokenizer::peekTokenKind(std::ifstream& stream, TokenKind token_kind) {
    size_t reset = stream.tellg();

    std::array<char, 32> token_buffer{};

    auto token_kind_string = ConfTokenizer::tokenKindString(token_kind);
    if (!token_kind_string) {
        return std::nullopt;
    }

    stream.read(&token_buffer[0], token_kind_string->size());
    stream.seekg(reset);

    if (std::string_view{token_buffer.data(), token_kind_string->size()} == token_kind_string) {
        return token_kind_string;
    }

    return std::nullopt;
}

std::optional<char> ConfTokenizer::peekEscapedCharacter(std::ifstream& stream) {
    size_t reset = stream.tellg();

    std::array<char, 2> token_buffer{};

    char c = stream.peek();
    if (std::string_view{&c, 1} == STRING_ESCAPE_SEQUENCE) {
        stream.seekg(1, std::ios::cur);
        c = stream.peek();
        stream.seekg(1, std::ios::cur);
        return c;
    }

    return std::nullopt;
}

std::optional<std::pair<TokenKind, std::string>> ConfTokenizer::peekNumberToken(std::ifstream& stream) {
    size_t reset = stream.tellg();
    std::array<char, 1024> token_buffer{};

    auto const check_hexadecimal = [&]() -> std::optional<size_t> {
        stream.read(token_buffer.data(), 2);
        auto const prefix = std::string_view{token_buffer.data(), 2};
        if (prefix != "0x") {
            stream.seekg(reset);
            return std::nullopt;
        }

        size_t cursor = 0;
        static constexpr size_t max_digits = 16;

        while (ConfTokenizer::isHexadecimalDigit(stream.peek()) && cursor <= max_digits) {
            stream.read(&token_buffer[cursor++], 1);
        }

        if (cursor == 0) {
            stream.seekg(reset);
            return std::nullopt;
        }

        return cursor;
    };

    auto const check_decimal = [&]() -> std::optional<size_t> {
        size_t cursor = 0;
        size_t decimal_points = 0;

        if (!ConfTokenizer::isDecimalDigit(stream.peek())) {
            stream.seekg(reset);
            return std::nullopt;
        }

        static constexpr size_t max_digits = 20;

        while ((ConfTokenizer::isDecimalDigit(stream.peek()) || stream.peek() == '.') && cursor <= max_digits && decimal_points <= 1) {
            if (stream.peek() == '.') {
                ++decimal_points;
                stream.read(&token_buffer[cursor++], 1);
                continue;
            }

            stream.read(&token_buffer[cursor++], 1);
        }

        if (cursor == 0) {
            stream.seekg(reset);
            return std::nullopt;
        }

        return cursor;
    };

    auto const check_octal = [&]() -> std::optional<size_t> {
        stream.read(token_buffer.data(), 2);
        auto const prefix = std::string_view{token_buffer.data(), 2};
        if (prefix != "0o") {
            stream.seekg(reset);
            return std::nullopt;
        }

        size_t cursor = 0;
        static constexpr size_t max_digits = 22;

        while (ConfTokenizer::isOctalDigit(stream.peek()) && cursor <= max_digits) {
            stream.read(&token_buffer[cursor++], 1);
        }

        if (cursor == 0) {
            stream.seekg(reset);
            return std::nullopt;
        }

        return cursor;
    };

    auto const check_binary = [&]() -> std::optional<size_t> {
        stream.read(token_buffer.data(), 2);
        auto const prefix = std::string_view{token_buffer.data(), 2};
        if (prefix != "0b") {
            stream.seekg(reset);
            return std::nullopt;
        }

        size_t cursor = 0;
        static constexpr size_t max_digits = 64;

        while (ConfTokenizer::isBinaryDigit(stream.peek()) && cursor <= max_digits) {
            stream.read(&token_buffer[cursor++], 1);
        }

        if (cursor == 0) {
            stream.seekg(reset);
            return std::nullopt;
        }

        return cursor;
    };

    auto const check_scientific = [&]() -> std::optional<size_t> {
        TODO("not implemented");
    };

    if (auto size = check_binary()) {
        return std::make_pair(NUMBER_LITERAL_BINARY, std::string{token_buffer.data(), size.value()});
    } else if (auto size = check_octal()) {
        return std::make_pair(NUMBER_LITERAL_OCTAL, std::string{token_buffer.data(), size.value()});
    } else if (auto size = check_hexadecimal()) {
        return std::make_pair(NUMBER_LITERAL_HEXADECIMAL, std::string{token_buffer.data(), size.value()});
    } else if (auto size = check_decimal()) {
        return std::make_pair(NUMBER_LITERAL_DECIMAL, std::string{token_buffer.data(), size.value()});
    }

    return std::nullopt;
}

std::optional<std::pair<TokenKind, std::string>> ConfTokenizer::peekPathToken(std::ifstream& stream) {
    size_t reset = stream.tellg();
    std::array<char, 1024> token_buffer{};

    auto const valid_next_token = [](char c) {
        return !ConfTokenizer::isSpace(c)
            && !ConfTokenizer::isStatementTerminator(c)
            && !ConfTokenizer::isStatementSeparator(c);
    };

    auto const check_absolute_path = [&]() -> std::optional<size_t> {
        if (stream.peek() != '/') {
            stream.seekg(reset);
            return std::nullopt;
        }

        size_t cursor = 0;

        while (valid_next_token(stream.peek())) {
            if (auto escaped_token = ConfTokenizer::peekEscapedCharacter(stream)) {
                std::ranges::copy(std::string_view{&escaped_token.value(), 1}, token_buffer.begin() + cursor);
                cursor += sizeof(char);
                continue;
            }

            stream.read(&token_buffer[cursor++], 1);
        }

        if (cursor == 0) {
            stream.seekg(reset);
            return std::nullopt;
        }

        return cursor;
    };

    auto const check_relative_path = [&]() -> std::optional<size_t> {
        if (stream.peek() == '/') {
            stream.seekg(reset);
            return std::nullopt;
        }

        size_t cursor = 0;

        while (valid_next_token(stream.peek())) {
            if (auto escaped_token = ConfTokenizer::peekEscapedCharacter(stream)) {
                std::ranges::copy(std::string_view{&escaped_token.value(), 1}, token_buffer.begin() + cursor);
                cursor += sizeof(char);
                continue;
            }

            stream.read(&token_buffer[cursor++], 1);
        }

        auto const path = std::string_view{token_buffer.data(), cursor};
        bool const has_prefix = path.starts_with("./");
        bool const has_separator = path.contains("/");

        if (cursor == 0 || (!has_prefix && !has_separator)) {
            stream.seekg(reset);
            return std::nullopt;
        }

        return cursor;
    };

    if (auto size = check_absolute_path()) {
        return std::make_pair(PATH_LITERAL_ABSOLUTE, std::string{token_buffer.data(), size.value()});
    } else if (auto size = check_relative_path()) {
        return std::make_pair(PATH_LITERAL_RELATIVE, std::string{token_buffer.data(), size.value()});
    }

    return std::nullopt;
}

constexpr std::optional<std::string_view> ConfTokenizer::tokenKindString(TokenKind token_kind) {
    switch (token_kind) {
        // Defined tokens
        case EQUALS:                     return STRING_EQUALS;
        case WALRUS:                     return STRING_WALRUS;
        case SEMI_COLON:                 return STRING_SEMI_COLON;
        case COMMA:                      return STRING_COMMA;
        case OPEN_BRACE:                 return STRING_OPEN_BRACE;
        case CLOSE_BRACE:                return STRING_CLOSE_BRACE;
        case OPEN_DOUBLE_BRACE:          return STRING_OPEN_DOUBLE_BRACE;
        case CLOSE_DOUBLE_BRACE:         return STRING_CLOSE_DOUBLE_BRACE;
        case OPEN_DOUBLE_QUOTE:          return STRING_OPEN_DOUBLE_QUOTE;
        case CLOSE_DOUBLE_QUOTE:         return STRING_CLOSE_DOUBLE_QUOTE;
        case OPEN_QUOTE:                 return STRING_OPEN_QUOTE;
        case CLOSE_QUOTE:                return STRING_CLOSE_QUOTE;
        case TAB_FEED:                   return STRING_TAB_FEED;
        case LINE_FEED:                  return STRING_LINE_FEED;
        case VERTICAL_FEED:              return STRING_VERTICAL_FEED;
        case SPACE:                      return STRING_SPACE;
        case KEYWORD_INCLUDE:            return STRING_KEYWORD_INCLUDE;
        case KEYWORD_PRINT:              return STRING_KEYWORD_PRINT;

        // Dynamic tokens
        case IDENTIFIER:
        case COMMENT:
        case NUMBER_LITERAL_DECIMAL:
        case NUMBER_LITERAL_HEXADECIMAL:
        case NUMBER_LITERAL_BINARY:
        case NUMBER_LITERAL_OCTAL:
        case STRING_LITERAL:
        case PATH_LITERAL_ABSOLUTE:
        case PATH_LITERAL_RELATIVE:
        case SHELL_LITERAL:
        case UNKNOWN:                    return std::nullopt;
    }
}

constexpr bool ConfTokenizer::isSpace(char c) {
    return std::isspace(static_cast<unsigned char>(c));
}

constexpr bool ConfTokenizer::isKeywordStart(char c) {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

constexpr bool ConfTokenizer::isIdentifierStart(char c) {
    return c == '_'
        || ('A' <= c && c <= 'Z')
        || ('a' <= c && c <= 'z');
}

constexpr bool ConfTokenizer::isPunctuatorStart(char c) {
    return std::ranges::find_if(
        PUNCTUATORS,
        [c](std::pair<TokenKind, std::string_view> const& pair) {
            return std::string_view{pair.second}.front() == c;
        }
    ) != PUNCTUATORS.end();
}

constexpr bool ConfTokenizer::isIdentifier(char c) {
    return ConfTokenizer::isIdentifierStart(c)
        || ('0' <= c && c <= '9')
        || c == '-';
}

constexpr bool ConfTokenizer::isStringLiteralStart(char c) {
    auto const char_string = std::string_view{&c, 1};
    return char_string == STRING_OPEN_QUOTE || char_string == STRING_OPEN_DOUBLE_QUOTE;
}

constexpr bool ConfTokenizer::isHexadecimalDigit(char c) {
    return ('A' <= c && c <= 'F')
        || ('a' <= c && c <= 'f')
        || ('0' <= c && c <= '9');
}

constexpr bool ConfTokenizer::isDecimalDigit(char c) {
    return ('0' <= c && c <= '9');
}

constexpr bool ConfTokenizer::isOctalDigit(char c) {
    return ('0' <= c && c <= '7');
}

constexpr bool ConfTokenizer::isBinaryDigit(char c) {
    return ('0' <= c && c <= '1');
}

constexpr bool ConfTokenizer::isCommentStart(char c) {
    return c == '#';
}

constexpr bool ConfTokenizer::isStatementTerminator(char c) {
    return std::ranges::find_if(
        STATEMENT_TERMINATORS,
        [c](std::pair<TokenKind, std::string_view> const& pair) {
            return std::string_view{pair.second}.front() == c;
        }
    ) != STATEMENT_TERMINATORS.end();
}

constexpr bool ConfTokenizer::isStatementSeparator(char c) {
    return std::ranges::find_if(
        STATEMENT_SEPARATORS,
        [c](std::pair<TokenKind, std::string_view> const& pair) {
            return std::string_view{pair.second}.front() == c;
        }
    ) != STATEMENT_SEPARATORS.end();
}

std::optional<Token> ConfTokenizer::eatKeyword(std::ifstream& stream) {
    if (!ConfTokenizer::isKeywordStart(stream.peek())) {
        return std::nullopt;
    }

    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};

    size_t reset = stream.tellg();

    while (!ConfTokenizer::isSpace(stream.peek())) {
        stream.read(&token_buffer[cursor++], 1);
    }

    auto const keyword = std::ranges::find_if(
        KEYWORDS,
        [keyword_chunk = std::string_view{token_buffer.data(), cursor}]
        (std::pair<TokenKind, std::string_view> const& pair) {
            return pair.second == keyword_chunk;
        }
    );

    if (keyword == KEYWORDS.end()) {
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

std::optional<Token> ConfTokenizer::eatIdentifier(std::ifstream& stream) {
    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};

    if (!ConfTokenizer::isIdentifierStart(stream.peek())) {
        return std::nullopt;
    }

    stream.read(&token_buffer[cursor++], 1);

    while (ConfTokenizer::isIdentifier(stream.peek())) {
        stream.read(&token_buffer[cursor++], 1);
    }

    return Token {
        .data = std::string{token_buffer.data(), cursor},
        .kind = IDENTIFIER,
        .position = static_cast<size_t>(stream.tellg()),
        .length = cursor,
    };
}

std::optional<Token> ConfTokenizer::eatShellLiteral(std::ifstream& stream) {
    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};
    size_t reset = stream.tellg();

    auto const delimiters = ConfTokenizer::peekDelimitersFor(stream, SHELL_LITERAL_OPEN_PUNCTUATORS);
    if (!delimiters) {
        return std::nullopt;
    }

    auto const& [_, terminator_kind] = delimiters.value();
    auto const increment_stream = makeStreamIncrementor(stream);

    while (!ConfTokenizer::peekTokenKind(stream, terminator_kind).and_then(increment_stream)) {
        if (auto escaped_token = ConfTokenizer::peekEscapedCharacter(stream)) {
            std::ranges::copy(std::string_view{&escaped_token.value(), 1}, token_buffer.begin() + cursor);
            cursor += sizeof(char);
            continue;
        }

        stream.read(&token_buffer[cursor++], 1);
    }

    auto data = std::string_view{token_buffer.data(), cursor}
        | std::views::drop_while(ConfTokenizer::isSpace)
        | std::views::reverse
        | std::views::drop_while(ConfTokenizer::isSpace)
        | std::views::reverse
        | std::ranges::to<std::string>();

    return Token {
        .data = std::move(data),
        .kind = SHELL_LITERAL,
        .position = static_cast<size_t>(stream.tellg()),
        .length = cursor,
    };
}

std::optional<Token> ConfTokenizer::eatLiteral(std::ifstream& stream) {
    size_t reset = stream.tellg();
    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};

    if (ConfTokenizer::isStringLiteralStart(stream.peek())) {
        auto const delimiters = ConfTokenizer::peekDelimitersFor(stream, STRING_LITERAL_OPEN_PUNCTUATORS);
        if (!delimiters) {
            return std::nullopt;
        }

        auto const& [_, terminator_kind] = delimiters.value();
        auto const increment_stream = makeStreamIncrementor(stream);

        bool escape_next = false;
        while (!ConfTokenizer::peekTokenKind(stream, terminator_kind).and_then(increment_stream)) {
            if (auto escaped_token = ConfTokenizer::peekEscapedCharacter(stream)) {
                std::ranges::copy(std::string_view{&escaped_token.value(), 1}, token_buffer.begin() + cursor);
                cursor += sizeof(char);
                continue;
            }

            stream.read(&token_buffer[cursor++], 1);
        }

        return Token {
            .data = std::string{token_buffer.data(), cursor},
            .kind = STRING_LITERAL,
            .position = static_cast<size_t>(stream.tellg()),
            .length = cursor,
        };
    } else if (auto number_literal = ConfTokenizer::peekNumberToken(stream)) {
        return Token {
            .data = std::move(number_literal->second),
            .kind = number_literal->first,
            .position = static_cast<size_t>(stream.tellg()),
            .length = number_literal->second.size(),
        };
    } else if (auto path_literal = ConfTokenizer::peekPathToken(stream)) {
        return Token {
            .data = std::move(path_literal->second),
            .kind = path_literal->first,
            .position = static_cast<size_t>(stream.tellg()),
            .length = path_literal->second.size(),
        };
    }

    return std::nullopt;
}

std::optional<Token> ConfTokenizer::eatSpaces(std::ifstream& stream) {
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

std::optional<Token> ConfTokenizer::eatPunctuator(std::ifstream& stream) {
    auto punctuator = ConfTokenizer::peekTokenFor(stream, PUNCTUATORS);
    if (!punctuator) {
        return std::nullopt;
    }

    auto const punctuator_string = ConfTokenizer::tokenKindString(punctuator.value());
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

std::optional<Token> ConfTokenizer::eatComment(std::ifstream& stream) {
    size_t cursor = 0;
    std::array<char, 1024> token_buffer{};

    if (!ConfTokenizer::isCommentStart(stream.peek())) {
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
