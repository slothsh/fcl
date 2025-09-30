#include <optional>
#include <expected>
#include <ranges>
#include <algorithm>
#include <string>
#include <cstddef>
#include <utility>

namespace detail {
    using Node = typename ConfParser::Node;
    using TokenListType = typename ConfParser::TokenListType;
    using NodePtr = typename ConfParser::NodePtr;
    using TokenType = typename ConfParser::TokenType;
    using TokenKindType = typename ConfParser::TokenKindType;
    using NumberType = typename ConfParser::NumberType;
    using Error = typename ConfParser::Error;

    template<std::ranges::range R>
    static bool rangeIsTokenKind(R&& range, TokenKindType token_kind) {
        return std::ranges::find_if(
            range,
            [&token_kind](TokenType const& t) {
                return t.kind == token_kind;
            }
        ) != range.end();
    }

    template<std::ranges::range R, size_t N>
    static bool rangeTokenIsAnyOf(R&& range, std::array<TokenKindType, N> const& token_kinds) {
        return std::ranges::find_if(
            range,
            [&token_kinds](TokenType const& t) {
                return std::ranges::contains(token_kinds, t.kind);
            }
        ) != range.end();
    }
}

std::optional<detail::NodePtr> ConfParser::parseTokenList(detail::TokenListType const& token_list) {
    using enum Error;
    using enum NodeKind;

    auto const push_child = [&](NodePtr& root, NodePtr& child) {
        auto const push_child_visitor = Visitors {
            [&](RootBlock& root) {
                root.nodes.push_back(std::move(child));
            },
            [](auto&&){},
        };

        std::visit(push_child_visitor, *root);
    };

    auto parser = ConfParser{token_list};

    auto root = std::make_unique<Node>(
        Node {
            RootBlock {
                .kind = ROOT_BLOCK,
                .nodes = {},
            }
        }
    );

    while (true) {
        auto node = parser.parse(root);
        if (!node) {
            break;
        }

        push_child(root, node.value());
    }

    return root;
}

ConfParser::ConfParser(detail::TokenListType const& token_list)
    : m_cursor{0}
    , m_token_list{token_list}
    , m_root{nullptr}
{}

std::expected<detail::NodePtr, detail::Error> ConfParser::parse(detail::NodePtr& parent) {
    using enum Error;
    using enum detail::TokenKindType;

    size_t reset = m_cursor;

    auto const head = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!head) {
        return std::unexpected(TODO);
    }

    switch (head.front().kind) {
        case IDENTIFIER: {
            if (auto named_block = this->takeNamedBlock(head.front(), parent)) {
                return std::move(named_block.value());
            }

            if (auto variable_assignment_expression = this->takeVariableAssignmentExpression(head.front(), parent)) {
                return std::move(variable_assignment_expression.value());
            }

            if (auto constant_assignment_expression = this->takeConstantAssignmentExpression(head.front(), parent)) {
                return std::move(constant_assignment_expression.value());
            }

            m_cursor = reset;
        } break;

        case STRING_LITERAL: {
            if (auto string_expression = this->takeStringExpression(head.front(), parent)) {
                return std::move(string_expression.value());
            }

            m_cursor = reset;
        } break;

        case NUMBER_LITERAL_HEXADECIMAL:
        case NUMBER_LITERAL_DECIMAL:
        case NUMBER_LITERAL_OCTAL:
        case NUMBER_LITERAL_BINARY: {
            if (auto number_expression = this->takeNumberExpression(head.front(), parent)) {
                return std::move(number_expression.value());
            }

            m_cursor = reset;
        } break;

        case PATH_LITERAL_ABSOLUTE:
        case PATH_LITERAL_RELATIVE: {
            if (auto path_expression = this->takePathExpression(head.front(), parent)) {
                return std::move(path_expression.value());
            }

            m_cursor = reset;
        } break;

        case SHELL_LITERAL: {
            if (auto shell_expression = this->takeShellExpression(head.front(), parent)) {
                return std::move(shell_expression.value());
            }

            m_cursor = reset;
        } break;

        case KEYWORD_INCLUDE: {
            if (auto keyword_statement = this->takeKeywordStatement(head.front(), parent)) {
                return std::move(keyword_statement.value());
            }

            m_cursor = reset;
        } break;

        case COMMENT: {
            return this->parse(parent);
        }

        default: {
            m_cursor = reset;
        } break;
    }

    return std::unexpected(TODO);
}


std::optional<detail::NodePtr> ConfParser::takeNamedBlock(detail::TokenType const& token, detail::NodePtr& parent) {
    using enum NodeKind;
    using enum TokenKindType;

    size_t const reset = m_cursor;

    auto const block_start = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(block_start, OPEN_BRACE)) {
        m_cursor = reset;
        return std::nullopt;
    }

    auto const push_child = [&](NodePtr& root, NodePtr& child) {
        auto const push_child_visitor = Visitors {
            [&](NamedBlock& root) {
                root.nodes.push_back(std::move(child));
            },
            [](auto&&){},
        };

        std::visit(push_child_visitor, *root);
    };

    auto root = std::make_unique<Node>(
        Node {
            NamedBlock {
                .kind = NAMED_BLOCK,
                .name = token,
                .nodes = {},
                .me = nullptr,
                .parent = parent.get(),
            }
        }
    );

    std::get<NamedBlock>(*root).me = root.get();

    while (true) {
        auto node = this->parse(root);
        if (!node) {
            break;
        }

        push_child(root, node.value());
    }

    auto const block_end = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(block_end, CLOSE_BRACE)) {
        m_cursor = reset;
        return std::nullopt;
    }

    return root;
}

std::optional<detail::NodePtr> ConfParser::takeKeywordStatement(detail::TokenType const& token, detail::NodePtr& parent) {
    using enum NodeKind;
    using enum TokenKindType;

    size_t const reset = m_cursor;

    auto const push_child = [&](NodePtr& root, NodePtr& child) {
        auto const push_child_visitor = Visitors {
            [&](KeywordStatement& root) {
                root.arguments.push_back(std::move(child));
            },
            [](auto&&){},
        };

        std::visit(push_child_visitor, *root);
    };

    auto root = std::make_unique<Node>(
        Node {
            KeywordStatement {
                .kind = KEYWORD_STATEMENT,
                .keyword = token,
                .arguments = {},
                .me = nullptr,
                .parent = parent.get(),
            }
        }
    );

    std::get<KeywordStatement>(*root).me = root.get();

    while (true) {
        auto node = this->parse(root);

        auto const separator_or_terminator_token = m_token_list
            | std::views::drop(m_cursor)
            | std::views::take(1);

        if (!node && detail::rangeIsTokenKind(separator_or_terminator_token, COMMA)) {
            ++m_cursor;
            continue;
        }

        if (!node) {
            break;
        }

        push_child(root, node.value());
    }

    auto const terminator_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(terminator_token, SEMI_COLON)) {
        m_cursor = reset;
        return std::nullopt;
    }

    return root;
}

std::optional<detail::NodePtr> ConfParser::takeVariableAssignmentExpression(TokenType const& token, detail::NodePtr& parent) {
    using enum NodeKind;
    using enum TokenKindType;

    size_t const reset = m_cursor;

    auto const operator_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(operator_token, EQUALS)) {
        m_cursor = reset;
        return std::nullopt;
    }

    auto root = std::make_unique<Node>(
        Node {
            VariableAssignmentExpression {
                .kind = VARIABLE_ASSIGNMENT_EXPRESSION,
                .name = token,
                .expression = {},
                .me = nullptr,
                .parent = parent.get(),
            }
        }
    );

    std::get<VariableAssignmentExpression>(*root).me = root.get();

    auto expression_node = this->parse(root);
    if (!expression_node) {
        m_cursor = reset;
        return std::nullopt;
    }

    std::get<VariableAssignmentExpression>(*root).expression = std::move(expression_node.value());

    auto const terminator_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(terminator_token, SEMI_COLON)) {
        m_cursor = reset;
        return std::nullopt;
    }

    return root;
}

std::optional<detail::NodePtr> ConfParser::takeConstantAssignmentExpression(TokenType const& token, detail::NodePtr& parent) {
    using enum NodeKind;
    using enum TokenKindType;

    size_t const reset = m_cursor;

    auto const operator_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(operator_token, WALRUS)) {
        m_cursor = reset;
        return std::nullopt;
    }

    auto root = std::make_unique<Node>(
        Node {
            ConstantAssignmentExpression {
                .kind = CONSTANT_ASSIGNMENT_EXPRESSION,
                .name = token,
                .expression = {},
                .me = nullptr,
                .parent = parent.get(),
            }
        }
    );

    std::get<ConstantAssignmentExpression>(*root).me = root.get();

    auto expression_node = this->parse(root);
    if (!expression_node) {
        m_cursor = reset;
        return std::nullopt;
    }

    std::get<ConstantAssignmentExpression>(*root).expression = std::move(expression_node.value());

    auto const terminator_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(terminator_token, SEMI_COLON)) {
        m_cursor = reset;
        return std::nullopt;
    }

    return root;
}

std::optional<detail::NodePtr> ConfParser::takeStringExpression(detail::TokenType const& token, detail::NodePtr& parent) {
    using enum NodeKind;
    using enum TokenKindType;

    if (token.kind != STRING_LITERAL) {
        return std::nullopt;
    }

    auto root = std::make_unique<Node>(
        Node {
            StringExpression {
                .kind = STRING_EXPRESSION,
                .token = token,
                .me = nullptr,
                .parent = parent.get(),
            }
        }
    );

    std::get<StringExpression>(*root).me = root.get();

    return root;
}

std::optional<detail::NodePtr> ConfParser::takeNumberExpression(detail::TokenType const& token, detail::NodePtr& parent) {
    using enum NodeKind;
    using enum TokenKindType;

    static constexpr std::array number_kinds {
        NUMBER_LITERAL_HEXADECIMAL,
        NUMBER_LITERAL_DECIMAL,
        NUMBER_LITERAL_OCTAL,
        NUMBER_LITERAL_BINARY,
    };

    if (!std::ranges::contains(number_kinds, token.kind)) {
        return std::nullopt;
    }

    auto const number_value = ConfParser::convertTokenToNumber(token);
    if (!number_value) {
        return std::nullopt;
    }

    auto root = std::make_unique<Node>(
        Node {
            NumberExpression {
                .kind = NUMBER_EXPRESSION,
                .value = number_value.value(),
                .token = token,
                .me = nullptr,
                .parent = parent.get(),
            }
        }
    );

    std::get<NumberExpression>(*root).me = root.get();

    return root;
}

std::optional<detail::NodePtr> ConfParser::takePathExpression(detail::TokenType const& token, detail::NodePtr& parent) {
    using enum NodeKind;
    using enum TokenKindType;

    static constexpr std::array path_kinds {
        PATH_LITERAL_ABSOLUTE,
        PATH_LITERAL_RELATIVE,
    };

    if (!std::ranges::contains(path_kinds, token.kind)) {
        return std::nullopt;
    }

    auto root = std::make_unique<Node>(
        Node {
            PathExpression {
                .kind = PATH_EXPRESSION,
                .token = token,
                .me = nullptr,
                .parent = parent.get(),
            }
        }
    );

    std::get<PathExpression>(*root).me = root.get();

    return root;
}

std::optional<detail::NodePtr> ConfParser::takeShellExpression(detail::TokenType const& token, detail::NodePtr& parent) {
    using enum NodeKind;
    using enum TokenKindType;

    if (token.kind != SHELL_LITERAL) {
        return std::nullopt;
    }

    auto root = std::make_unique<Node>(
        Node {
            ShellExpression {
                .kind = SHELL_EXPRESSION,
                .command = token,
                .me = nullptr,
                .parent = parent.get(),
            }
        }
    );

    std::get<ShellExpression>(*root).me = root.get();

    return root;
}

std::expected<detail::NumberType, detail::Error> ConfParser::convertTokenToNumber(detail::TokenType const& token) noexcept {
    using enum Error;
    using namespace Conf;
    using enum TokenKindType;

    switch (token.kind) {
        case NUMBER_LITERAL_HEXADECIMAL: {
            auto const number = Number::fromHexadecimalString<NumberType>(token.data);
            if (!number) {
                return std::unexpected(NUMBER_CONVERSION_ERROR);
            }

            return number.value();
        } break;

        case NUMBER_LITERAL_DECIMAL: {
            auto const number = Number::fromDecimalString<NumberType>(token.data);
            if (!number) {
                return std::unexpected(NUMBER_CONVERSION_ERROR);
            }

            return number.value();
        } break;

        case NUMBER_LITERAL_OCTAL: {
            auto const number = Number::fromOctalString<NumberType>(token.data);
            if (!number) {
                return std::unexpected(NUMBER_CONVERSION_ERROR);
            }

            return number.value();
        } break;

        case NUMBER_LITERAL_BINARY: {
            auto const number = Number::fromBinaryString<NumberType>(token.data);
            if (!number) {
                return std::unexpected(NUMBER_CONVERSION_ERROR);
            }

            return number.value();
        } break;

        default: return std::unexpected(NUMBER_CONVERSION_ERROR);
    }
}
