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
}

std::optional<detail::NodePtr> ConfParser::parse(detail::TokenListType const& token_list) {
    using enum Error;
    using enum NodeKind;

    auto parser = ConfParser{token_list};

    std::vector<NodePtr> nodes{};

    while (true) {
        auto node = parser.parse();
        if (!node) {
            break;
        }

        nodes.push_back(std::move(node.value()));
    }

    return std::make_unique<Node>(
        Node {
            RootBlock {
                .kind = ROOT_BLOCK,
                .nodes = std::move(nodes),
            }
        }
    );
}

ConfParser::ConfParser(detail::TokenListType const& token_list)
    : m_cursor{0}
    , m_token_list{token_list}
    , m_root{nullptr}
{}

std::expected<detail::NodePtr, detail::Error> ConfParser::parse() {
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
            if (auto named_block = this->takeNamedBlock(head.front())) {
                return std::move(named_block.value());
            }

            if (auto named_declaration = this->takeNamedDeclaration(head.front())) {
                return std::move(named_declaration.value());
            }

            if (auto shell_expression = this->takeShellExpression(head.front())) {
                return std::move(shell_expression.value());
            }

            m_cursor = reset;
        } break;

        case KEYWORD_INCLUDE: {
            if (auto keyword_bin_op = this->takeKeywordBinOp(head.front())) {
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

std::optional<detail::NodePtr> ConfParser::takeNamedBlock(detail::TokenType const& token) {
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

    std::vector<NodePtr> nodes{};

    while (true) {
        auto node = this->parse();
        if (!node) {
            break;
        }

        nodes.push_back(std::move(node.value()));
    }

    auto const block_end = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(block_end, CLOSE_BRACE)) {
        m_cursor = reset;
        return std::nullopt;
    }

    return std::make_unique<Node>(
        Node {
            NamedBlock {
                .kind = NAMED_BLOCK,
                .name = token,
                .nodes = std::move(nodes),
            }
        }
    );
}

std::optional<detail::NodePtr> ConfParser::takeKeywordBinOp(detail::TokenType const& token) {
    using enum NodeKind;
    using enum TokenKindType;

    size_t const reset = m_cursor;

    auto const bin_op_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(bin_op_token, EQUALS)) {
        m_cursor = reset;
        return std::nullopt;
    }

    auto const expression_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!ConfParser::isExpressionToken(expression_token.front().kind)) {
        m_cursor = reset;
        return std::nullopt;
    }

    return std::make_unique<Node>(
        Node {
            KeywordBinOp {
                .kind = KEYWORD_BIN_OP,
                .keyword = token,
                .expression = expression_token.front(),
            }
        }
    );
}

std::optional<detail::NodePtr> ConfParser::takeNamedDeclaration(TokenType const& token) {
    using enum NodeKind;
    using enum TokenKindType;

    size_t const reset = m_cursor;

    auto const bin_op_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(bin_op_token, EQUALS)) {
        m_cursor = reset;
        return std::nullopt;
    }

    auto const expression_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!ConfParser::isExpressionToken(expression_token.front().kind)) {
        m_cursor = reset;
        return std::nullopt;
    }

    return std::make_unique<Node>(
        Node {
            NamedDeclaration {
                .kind = NAMED_DECLARATION,
                .name = token,
                .expression = expression_token.front(),
            }
        }
    );
}

std::optional<detail::NodePtr> ConfParser::takeShellExpression(detail::TokenType const& token) {
    using enum NodeKind;
    using enum TokenKindType;

    size_t const reset = m_cursor;

    auto const bin_op_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(bin_op_token, EQUALS)) {
        m_cursor = reset;
        return std::nullopt;
    }

    auto const expression_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!detail::rangeIsTokenKind(expression_token, SHELL_EXPRESSION)) {
        m_cursor = reset;
        return std::nullopt;
    }

    return std::make_unique<Node>(
        Node {
            NamedShellDeclaration {
                .kind = NAMED_SHELL_DECLARATION,
                .name = token,
                .command = expression_token.front(),
            }
        }
    );
}
