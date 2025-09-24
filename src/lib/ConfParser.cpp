#include <optional>
#include <expected>
#include <ranges>
#include <algorithm>

namespace detail {
    using Node = typename ConfParser::Node;
    using TokenListType = typename ConfParser::TokenListType;
    using NodePtr = typename ConfParser::NodePtr;
    using TokenType = typename ConfParser::TokenType;
    using TokenKindType = typename ConfParser::TokenKindType;
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

        case NUMBER_LITERAL: {
            if (auto number_expression = this->takeNumberExpression(head.front(), parent)) {
                return std::move(number_expression.value());
            }

            m_cursor = reset;
        } break;

        case PATH_LITERAL: {
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
            if (auto keyword_bin_op = this->takeKeywordBinOp(head.front(), parent)) {
                return std::move(keyword_bin_op.value());
            }

            m_cursor = reset;
        } break;

        default: {
            m_cursor = reset;
        } break;
    }

    return std::unexpected(TODO);
}


bool ConfParser::isExpressionToken(TokenKindType token_kind) {
    return std::ranges::contains(ConfParser::EXPRESSION_TOKEN_KINDS, token_kind);
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

std::optional<detail::NodePtr> ConfParser::takeKeywordBinOp(detail::TokenType const& token, detail::NodePtr& parent) {
    using enum NodeKind;
    using enum TokenKindType;

    size_t const reset = m_cursor;

    auto const expression_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!ConfParser::isExpressionToken(expression_token.front().kind)) {
        m_cursor = reset;
        return std::nullopt;
    }

    auto const terminator_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(terminator_token, SEMI_COLON)) {
        m_cursor = reset;
        return std::nullopt;
    }

    auto root = std::make_unique<Node>(
        Node {
            KeywordBinOp {
                .kind = KEYWORD_BIN_OP,
                .keyword = token,
                .expression = expression_token.front(),
                .me = nullptr,
                .parent = parent.get(),
            }
        }
    );

    std::get<KeywordBinOp>(*root).me = root.get();

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

    if (token.kind != NUMBER_LITERAL) {
        return std::nullopt;
    }

    auto root = std::make_unique<Node>(
        Node {
            NumberExpression {
                .kind = NUMBER_EXPRESSION,
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

    if (token.kind != PATH_LITERAL) {
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
