#include <optional>
#include <expected>
#include <ranges>
#include <variant>
#include <algorithm>

namespace detail {
    using Node = typename ConfParser::Node;
    using TokenListType = typename ConfParser::TokenListType;
    using NodePtr = typename ConfParser::NodePtr;
    using TokenType = typename ConfParser::TokenType;
    using TokenKindType = typename ConfParser::TokenKindType;
    using Error = typename ConfParser::Error;
}

std::optional<detail::NodePtr> ConfParser::parse(detail::TokenListType const& token_list) {
    using enum Error;

    auto parser = ConfParser{token_list};

    return parser
        .parse()
        .or_else([](Error error) {
            WARN("not implemented", "");
            return std::expected<NodePtr, Error>{NodePtr{nullptr}};
        }).value();
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

    static auto const print_visitor = Visitors {
        [](ConfParser::NamedBlock const& node) {
            INFO("token kind: {}", node.kind);
        },
        [](ConfParser::KeywordBinOp const& node) {
            INFO("token kind: {}", node.kind);
        }
    };

    auto const head = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!head) {
        return std::unexpected(TODO);
    }

    switch (head.front().kind) {
        case IDENTIFIER: {
            auto const tail = m_token_list
                | std::views::drop(m_cursor);

            if (auto named_block = this->takeNamedBlock(head.front())) {
                std::visit(print_visitor, *named_block.value());
                return std::move(named_block.value());
            }

            m_cursor = reset;
        } break;

        case KEYWORD_INCLUDE: {
            auto const tail = m_token_list
                | std::views::drop(m_cursor);

            if (auto keyword_bin_op = this->takeKeywordBinOp(head.front())) {
                std::visit(print_visitor, *keyword_bin_op.value());
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

    size_t reset = 0;

    auto const block_start = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!block_start && block_start.front().kind != OPEN_BRACE) {
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

    if (!block_end && block_end.front().kind != CLOSE_BRACE) {
        m_cursor = reset;
        return std::nullopt;
    }

    return std::make_unique<Node>(
        Node {
            NamedBlock {
                .kind = NAMED_BLOCK,
                .name = token,
                .parent = nullptr,
                .nodes = std::move(nodes),
            }
        }
    );
}

std::optional<detail::NodePtr> ConfParser::takeKeywordBinOp(detail::TokenType const& token) {
    using enum NodeKind;
    using enum TokenKindType;

    size_t reset = m_cursor;

    auto const bin_op_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!bin_op_token && bin_op_token.front().kind != EQUALS) {
        m_cursor = reset;
        return std::nullopt;
    }

    auto const expression_token = m_token_list
        | std::views::drop(m_cursor++)
        | std::views::take(1);

    if (!expression_token && !ConfParser::isExpressionToken(expression_token.front().kind)) {
        m_cursor = reset;
        return std::nullopt;
    }

    return std::make_unique<Node>(
        Node {
            KeywordBinOp {
                .kind = KEYWORD_BIN_OP,
                .keyword = token,
                .expression = expression_token.front(),
                .parent = nullptr,
            }
        }
    );
}
