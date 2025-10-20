export module Conf:Common;

import std;
import Traits;

export namespace Conf {

} // END OF NAMESPACE `Conf`

export namespace Conf::Number {

template<std::floating_point T, HasRawData<std::string::value_type> S = std::string>
std::optional<T> fromHexadecimalString(S const& number_string) noexcept {
    auto const digit_value = [](char c) -> std::int64_t {
        if ('0' <= c && c <= '9') return c - '0';
        if ('A' <= c && c <= 'F') return 10 + (c - 'A');
        if ('a' <= c && c <= 'f') return 10 + (c - 'a');
        return -1;
    };

    auto const group_value = [](auto group) -> std::int64_t {
        return std::pow(16, std::get<0>(group)) * std::get<1>(group);
    };

    auto const digits = number_string | std::views::transform(digit_value) | std::views::reverse;

    if (std::ranges::any_of(digits, [](int digit){ return digit == -1; })) {
        return std::nullopt;
    }

    std::int64_t value = std::ranges::fold_left(
        std::views::zip(std::ranges::iota_view(0), digits) | std::views::transform(group_value),
        0LL,
        std::plus{}
    );

    return static_cast<T>(value);
}

template<std::floating_point T, typename S = std::string>
std::optional<T> fromDecimalString(S const& number_string) noexcept {
    auto const digit_value = [](char c) -> std::int64_t {
        if ('0' <= c && c <= '9') return c - '0';
        if (c == '.') return 0;
        return -1;
    };

    auto const group_value = [](auto group) -> T {
        return std::pow(10, std::get<0>(group)) * std::get<1>(group);
    };

    auto const digits = number_string | std::views::transform(digit_value);

    if (std::ranges::any_of(digits, [](int digit){ return digit == -1; })) {
        return std::nullopt;
    }

    auto const decimal_position = std::ranges::find(number_string, '.');
    auto const decimal_offset = decimal_position != number_string.end()
        ? std::ranges::distance(number_string.begin(), decimal_position)
        : 0;
    auto const off_by_one = decimal_position != number_string.end() ? 1 : 0;

    auto const whole_part = digits | std::views::take(digits.size() - decimal_offset - off_by_one) | std::views::reverse;
    auto const fractional_part = digits | std::views::drop(digits.size() - decimal_offset) | std::views::take(decimal_offset) | std::views::reverse;

    T const whole_value = std::ranges::fold_left(
        std::views::zip(std::ranges::iota_view(0), whole_part) | std::views::transform(group_value),
        0LL,
        std::plus{}
    );

    T const fractional_value = std::ranges::fold_left(
        std::views::zip(std::ranges::iota_view(-decimal_offset), fractional_part) | std::views::transform(group_value),
        0LL,
        std::plus{}
    );

    return whole_value + fractional_value;
}

template<std::floating_point T, typename S = std::string>
std::optional<T> fromOctalString(S const& number_string) noexcept {
    auto const digit_value = [](char c) -> std::int64_t {
        if ('0' <= c && c <= '7') return c - '0';
        return -1;
    };

    auto const group_value = [](auto group) -> std::int64_t {
        return std::pow(8, std::get<0>(group)) * std::get<1>(group);
    };

    auto const digits = number_string | std::views::transform(digit_value) | std::views::reverse;

    if (std::ranges::any_of(digits, [](int digit){ return digit == -1; })) {
        return std::nullopt;
    }

    std::int64_t value = std::ranges::fold_left(
        std::views::zip(std::ranges::iota_view(0), digits) | std::views::transform(group_value),
        0LL,
        std::plus{}
    );

    return static_cast<T>(value);
}


template<std::floating_point T, typename S = std::string>
std::optional<T> fromBinaryString(S const& number_string) noexcept {
    auto const digit_value = [](char c) -> std::int64_t {
        if ('0' <= c && c <= '1') return c - '0';
        return -1;
    };

    auto const group_value = [](auto group) -> std::int64_t {
        return std::pow(2, std::get<0>(group)) * std::get<1>(group);
    };

    auto const digits = number_string | std::views::transform(digit_value) | std::views::reverse;

    if (std::ranges::any_of(digits, [](int digit){ return digit == -1; })) {
        return std::nullopt;
    }

    std::int64_t value = std::ranges::fold_left(
        std::views::zip(std::ranges::iota_view(0), digits) | std::views::transform(group_value),
        0LL,
        std::plus{}
    );

    return static_cast<T>(value);
}

} // END OF NAMESPACE `Conf::Number`

export namespace Conf::Language {

// Common Types
using NumberType = double;
using PathType = std::filesystem::path;

// Tokenizer

enum class TokenKind {
    UNKNOWN,
    EQUALS,
    WALRUS,
    SEMI_COLON,
    COMMA,
    IDENTIFIER,
    NUMBER_LITERAL_DECIMAL,
    NUMBER_LITERAL_HEXADECIMAL,
    NUMBER_LITERAL_BINARY,
    NUMBER_LITERAL_OCTAL,
    STRING_LITERAL,
    PATH_LITERAL_ABSOLUTE,
    PATH_LITERAL_RELATIVE,
    SHELL_LITERAL,
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
    KEYWORD_PRINT,
};

// Binary Operator Strings
inline constexpr std::string_view STRING_EQUALS              = "=";
inline constexpr std::string_view STRING_WALRUS              = ":=";

// Symmetric Delimiter Strings
inline constexpr std::string_view STRING_OPEN_BRACE          = "{";
inline constexpr std::string_view STRING_CLOSE_BRACE         = "}";
inline constexpr std::string_view STRING_OPEN_DOUBLE_BRACE   = "{{";
inline constexpr std::string_view STRING_CLOSE_DOUBLE_BRACE  = "}}";
inline constexpr std::string_view STRING_OPEN_QUOTE          = "'";
inline constexpr std::string_view STRING_CLOSE_QUOTE         = "'";
inline constexpr std::string_view STRING_OPEN_DOUBLE_QUOTE   = "\"";
inline constexpr std::string_view STRING_CLOSE_DOUBLE_QUOTE  = "\"";

// White Space Strings
inline constexpr std::string_view STRING_TAB_FEED            = "\t";
inline constexpr std::string_view STRING_LINE_FEED           = "\r\n";
inline constexpr std::string_view STRING_VERTICAL_FEED       = "\v";
inline constexpr std::string_view STRING_SPACE               = " ";

// Single Character Strings
inline constexpr std::string_view STRING_SEMI_COLON          = ";";
inline constexpr std::string_view STRING_COMMA               = ",";

// Single Delimiter Strings
inline constexpr std::string_view STRING_COMMENT_SINGLE_LINE = "#";
inline constexpr std::string_view STRING_ESCAPE_SEQUENCE     = "\\";

// Keyword Strings
inline constexpr std::string_view STRING_KEYWORD_INCLUDE     = "include";
inline constexpr std::string_view STRING_KEYWORD_PRINT       = "print";


struct Token {
    std::string data;
    TokenKind kind;
    std::size_t position;
    std::size_t length;
};

inline constexpr std::array KEYWORDS {
    std::pair{ TokenKind::KEYWORD_INCLUDE, STRING_KEYWORD_INCLUDE },
    std::pair{ TokenKind::KEYWORD_PRINT, STRING_KEYWORD_PRINT },
};

// TODO: separate into open/close punctuators
// These must be ordered from longest to shortest
inline constexpr std::array PUNCTUATORS {
    std::pair{ TokenKind::EQUALS, STRING_EQUALS },
    std::pair{ TokenKind::WALRUS, STRING_WALRUS },
    std::pair{ TokenKind::COMMA, STRING_COMMA },
    std::pair{ TokenKind::SEMI_COLON, STRING_SEMI_COLON },
    std::pair{ TokenKind::OPEN_BRACE, STRING_OPEN_BRACE },
    std::pair{ TokenKind::CLOSE_BRACE, STRING_CLOSE_BRACE },
};

inline constexpr std::array STATEMENT_TERMINATORS {
    std::pair{ TokenKind::SEMI_COLON, STRING_SEMI_COLON },
};

inline constexpr std::array STATEMENT_SEPARATORS {
    std::pair{ TokenKind::COMMA, STRING_COMMA },
};

inline constexpr std::array STRING_LITERAL_OPEN_PUNCTUATORS {
    std::pair{ TokenKind::OPEN_QUOTE, STRING_OPEN_QUOTE },
    std::pair{ TokenKind::OPEN_DOUBLE_QUOTE, STRING_OPEN_DOUBLE_QUOTE },
};

inline constexpr std::array STRING_LITERAL_CLOSE_PUNCTUATORS {
    std::pair{ TokenKind::CLOSE_QUOTE, STRING_CLOSE_QUOTE },
    std::pair{ TokenKind::CLOSE_DOUBLE_QUOTE, STRING_CLOSE_DOUBLE_QUOTE },
};

inline constexpr std::array SHELL_LITERAL_OPEN_PUNCTUATORS {
    std::pair{ TokenKind::OPEN_DOUBLE_BRACE, STRING_OPEN_DOUBLE_BRACE },
};

inline constexpr std::array SHELL_LITERAL_CLOSE_PUNCTUATORS {
    std::pair{ TokenKind::CLOSE_DOUBLE_BRACE, STRING_CLOSE_DOUBLE_BRACE },
};

// Parser

enum class NodeKind {
    FILE_PATH_ROOT_BLOCK,
    FILE_PATH_SUB_ROOT_BLOCK,
    NAMED_BLOCK,
    KEYWORD_STATEMENT,
    VARIABLE_ASSIGNMENT_EXPRESSION,
    CONSTANT_ASSIGNMENT_EXPRESSION,
    STRING_EXPRESSION,
    NUMBER_EXPRESSION,
    PATH_EXPRESSION,
    SHELL_EXPRESSION,
};

struct FilePathRootBlock;
struct FilePathSubRootBlock;
struct NamedBlock;
struct KeywordStatement;
struct VariableAssignmentExpression;
struct ConstantAssignmentExpression;
struct StringExpression;
struct NumberExpression;
struct PathExpression;
struct ShellExpression;

using Node = std::variant<
    FilePathRootBlock,
    FilePathSubRootBlock,
    NamedBlock,
    KeywordStatement,
    VariableAssignmentExpression,
    ConstantAssignmentExpression,
    StringExpression,
    NumberExpression,
    PathExpression,
    ShellExpression
>;

using NodePtr = std::unique_ptr<Node>;

struct FilePathRootBlock {
    NodeKind kind;
    std::vector<NodePtr> nodes;
    PathType file_path;
    Node* me;
};

struct FilePathSubRootBlock {
    NodeKind kind;
    std::vector<NodePtr> nodes;
    PathType file_path;
    Node* me;
    Node* parent;
};

struct NamedBlock {
    NodeKind kind;
    Token name;
    std::vector<NodePtr> nodes;
    Node* me;
    Node* parent;
};

struct KeywordStatement {
    NodeKind kind;
    Token keyword;
    std::vector<NodePtr> arguments;
    Node* me;
    Node* parent;
};

struct VariableAssignmentExpression {
    NodeKind kind;
    Token name;
    NodePtr expression;
    Node* me;
    Node* parent;
};

struct ConstantAssignmentExpression {
    NodeKind kind;
    Token name;
    NodePtr expression;
    Node* me;
    Node* parent;
};

struct StringExpression {
    NodeKind kind;
    Token token;
    Node* me;
    Node* parent;
};

struct NumberExpression {
    NodeKind kind;
    NumberType value;
    Token token;
    Node* me;
    Node* parent;
};

struct PathExpression {
    NodeKind kind;
    PathType path;
    Token token;
    Node* me;
    Node* parent;
};

struct ShellExpression {
    NodeKind kind;
    Token command;
    Node* me;
    Node* parent;
};

// Function/Keyword Schemas

struct KeywordSchema {
    using ParametersSchema = std::array<std::array<TokenKind, 32>, 128>;

    template<HasFunctionTraits<ParametersSchema> T>
    explicit KeywordSchema(T&&)
        : arity{T::arity}
        , parameters{T::parameters}
    {}

    std::size_t const& arity;
    ParametersSchema const& parameters;
};

template<std::size_t Index, typename I, auto F, TokenKind... V>
struct TokenArgument : FunctionArgument<Index, TokenKind, NodePtr, I, F, V...> {};

struct KeywordInclude : FunctionSchemaTraits<
    TokenKind,
    NodePtr,
    TokenArgument<
        0,
        PathExpression,
        [](PathExpression& inner) -> auto const& { return inner; },
        TokenKind::PATH_LITERAL_RELATIVE, TokenKind::PATH_LITERAL_ABSOLUTE
    >
>
{
    using FilePathArg = std::tuple_element_t<0, typename KeywordInclude::Unwrappers>;
};

struct KeywordPrint : FunctionSchemaTraits<
    TokenKind,
    NodePtr,
    TokenArgument<
        0,
        StringExpression,
        [](StringExpression& inner) -> auto const& { return inner.token.data; },
        TokenKind::STRING_LITERAL
    >,
    TokenArgument<
        1,
        NumberExpression,
        [](NumberExpression& inner) -> auto const& { return inner.value; },
        TokenKind::NUMBER_LITERAL_HEXADECIMAL,
        TokenKind::NUMBER_LITERAL_DECIMAL,
        TokenKind::NUMBER_LITERAL_BINARY,
        TokenKind::NUMBER_LITERAL_OCTAL
    >
>
{
    using StringArg = std::tuple_element_t<0, typename KeywordPrint::Unwrappers>;
    using NumberArg = std::tuple_element_t<1, typename KeywordPrint::Unwrappers>;
};

// Function/Keyword Helpers

template<IsFunctionArgument ArgName, IsSubscriptable<typename ArgName::VariantType> Args>
auto& get_argument(Args const& arguments) {
    return ArgName::unwrap(std::get<typename ArgName::InnerType>(*arguments[ArgName::Index]));
}

template<IsFunctionArgument ArgName, IsSubscriptable<typename ArgName::VariantType> Args>
auto& get_argument_checked(Args const& arguments) {
    return ArgName::unwrap(std::get<typename ArgName::InnerType>(*arguments.at(ArgName::Index)));
}

// Concepts

template<typename T>
concept HasNodeKind = requires (T t) {
    { t.kind } -> std::same_as<NodeKind&>;
};

template<typename T>
concept HasToken = requires (T t) {
    { t.token } -> std::same_as<Token&>;
};

template<typename T>
concept HasParent = requires (T t) {
    { t.parent } -> std::same_as<Node*&>;
};

template<typename T>
concept HasChildren = requires (T t) {
    { t.nodes } -> std::same_as<std::vector<NodePtr>&>;
};

template<typename T>
concept IsRootBlock = AnyOf<
    T,
    FilePathRootBlock,
    FilePathSubRootBlock
>;

template<typename T>
concept SimpleExpression = AnyOf<
    T,
    StringExpression,
    PathExpression
>;

} // END OF NAMESPACE `Conf::Number`

export template <>
struct std::formatter<Conf::Language::TokenKind> : std::formatter<std::string_view> {
    using enum Conf::Language::TokenKind;

    static constexpr std::string_view to_string(Conf::Language::TokenKind kind) {
        switch (kind) {
            case UNKNOWN:                    return "UNKNOWN";
            case IDENTIFIER:                 return "IDENTIFIER";
            case EQUALS:                     return "EQUALS";
            case WALRUS:                     return "WALRUS";
            case SEMI_COLON:                 return "SEMI_COLON";
            case COMMA:                      return "COMMA";
            case NUMBER_LITERAL_DECIMAL:     return "NUMBER_LITERAL_DECIMAL";
            case NUMBER_LITERAL_HEXADECIMAL: return "NUMBER_LITERAL_HEXADECIMAL";
            case NUMBER_LITERAL_BINARY:      return "NUMBER_LITERAL_BINARY";
            case NUMBER_LITERAL_OCTAL:       return "NUMBER_LITERAL_OCTAL";
            case STRING_LITERAL:             return "STRING_LITERAL";
            case PATH_LITERAL_ABSOLUTE:      return "PATH_LITERAL_ABSOLUTE";
            case PATH_LITERAL_RELATIVE:      return "PATH_LITERAL_RELATIVE";
            case SHELL_LITERAL:              return "SHELL_LITERAL";
            case COMMENT:                    return "COMMENT";
            case OPEN_BRACE:                 return "OPEN_BRACE";
            case CLOSE_BRACE:                return "CLOSE_BRACE";
            case OPEN_DOUBLE_BRACE:          return "OPEN_DOUBLE_BRACE";
            case CLOSE_DOUBLE_BRACE:         return "CLOSE_DOUBLE_BRACE";
            case OPEN_QUOTE:                 return "OPEN_QUOTE";
            case CLOSE_QUOTE:                return "CLOSE_QUOTE";
            case OPEN_DOUBLE_QUOTE:          return "OPEN_DOUBLE_QUOTE";
            case CLOSE_DOUBLE_QUOTE:         return "CLOSE_DOUBLE_QUOTE";
            case TAB_FEED:                   return "TAB_FEED";
            case LINE_FEED:                  return "LINE_FEED";
            case VERTICAL_FEED:              return "VERTICAL_FEED";
            case SPACE:                      return "SPACE";
            case KEYWORD_INCLUDE:            return "KEYWORD_INCLUDE";
            case KEYWORD_PRINT:              return "KEYWORD_PRINT";
        }
    }

    template <typename FormatContext>
    auto format(Conf::Language::TokenKind kind, FormatContext& ctx) const {
        return std::formatter<std::string_view>::format(to_string(kind), ctx);
    }
};

export template <>
struct std::formatter<Conf::Language::NodeKind> : std::formatter<std::string_view> {
    using enum Conf::Language::NodeKind;

    static constexpr std::string_view to_string(Conf::Language::NodeKind kind) {
        switch (kind) {
            case FILE_PATH_ROOT_BLOCK:           return "FILE_PATH_ROOT_BLOCK";
            case FILE_PATH_SUB_ROOT_BLOCK:       return "FILE_PATH_SUB_ROOT_BLOCK";
            case NAMED_BLOCK:                    return "NAMED_BLOCK";
            case KEYWORD_STATEMENT:              return "KEYWORD_STATEMENT";
            case VARIABLE_ASSIGNMENT_EXPRESSION: return "VARIABLE_ASSIGNMENT_EXPRESSION";
            case CONSTANT_ASSIGNMENT_EXPRESSION: return "CONSTANT_ASSIGNMENT_EXPRESSION";
            case STRING_EXPRESSION:              return "STRING_EXPRESSION";
            case NUMBER_EXPRESSION:              return "NUMBER_EXPRESSION";
            case PATH_EXPRESSION:                return "PATH_EXPRESSION";
            case SHELL_EXPRESSION:               return "SHELL_EXPRESSION";
        }
    }

    template <typename FormatContext>
    auto format(Conf::Language::NodeKind kind, FormatContext& ctx) const {
        return std::formatter<std::string_view>::format(to_string(kind), ctx);
    }
};
