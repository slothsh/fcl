#include <algorithm>
#include <ranges>
#include <cstddef>
#include <expected>
#include <vector>

#define FORWARD_VISITOR(member_function) \
    [](std::remove_cvref_t<std::tuple_element_t<0, typename FunctionTraits<decltype(member_function)>::argument_types>> const& node) { return member_function(node); }

namespace detail {
    static constexpr auto ANALYZER_VISITOR = Visitors {
        FORWARD_VISITOR(ConfAnalyzer::visitFilePathRootBlock),
        FORWARD_VISITOR(ConfAnalyzer::visitFilePathSubRootBlock),
        FORWARD_VISITOR(ConfAnalyzer::visitNamedBlock),
        FORWARD_VISITOR(ConfAnalyzer::visitKeywordStatement),
        FORWARD_VISITOR(ConfAnalyzer::visitVariableAssignmentExpression),
        FORWARD_VISITOR(ConfAnalyzer::visitConstantAssignmentExpression),
        FORWARD_VISITOR(ConfAnalyzer::visitStringExpression),
        FORWARD_VISITOR(ConfAnalyzer::visitNumberExpression),
        FORWARD_VISITOR(ConfAnalyzer::visitPathExpression),
        FORWARD_VISITOR(ConfAnalyzer::visitShellExpression),
    };
}

ConfAnalyzer::ConfAnalyzer(ConfAnalyzer::AstType const& ast)
    : m_ast{ast}
{}

std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::analyze() const noexcept {
    return std::visit(detail::ANALYZER_VISITOR, *m_ast);
}

std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::visitFilePathRootBlock(ConfAnalyzer::FilePathRootBlock const& node) noexcept {
    using enum ConfAnalyzer::Error;

    if (!node.file_path.is_absolute()) {
        return std::unexpected(FILE_PATH_NOT_ABSOLUTE);
    }

    for (auto const& child : node.nodes) {
        auto const child_result = std::visit(detail::ANALYZER_VISITOR, *child);
        if (!child_result) {
            return child_result;
        }
    }

    return {};
}

std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::visitFilePathSubRootBlock(ConfAnalyzer::FilePathSubRootBlock const& node) noexcept {
    using enum ConfAnalyzer::Error;

    if (!node.file_path.is_absolute()) {
        return std::unexpected(FILE_PATH_NOT_ABSOLUTE);
    }

    for (auto const& child : node.nodes) {
        auto const child_result = std::visit(detail::ANALYZER_VISITOR, *child);
        if (!child_result) {
            return child_result;
        }
    }

    return {};
}

std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::visitNamedBlock(ConfAnalyzer::NamedBlock const& node) noexcept {
    for (auto const& child : node.nodes) {
        auto const child_result = std::visit(detail::ANALYZER_VISITOR, *child);
        if (!child_result) {
            return child_result;
        }
    }

    return {};
}

std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::visitKeywordStatement(ConfAnalyzer::KeywordStatement const& node) noexcept {
    using enum Error;
    using enum ConfAnalyzer::TokenKind;

    switch (node.keyword.kind) {
        case KEYWORD_INCLUDE: {
            return ConfAnalyzer::typeCheckFunctionArguments(node.arguments, ConfAnalyzer::KEYWORD_INCLUDE_ARGS_SCHEMA);
        } break;

        default: {
            WARN("unknown keyword {}", node.kind);
        } break;
    }

    return {};
}

std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::visitVariableAssignmentExpression(ConfAnalyzer::VariableAssignmentExpression const& node) noexcept {
    WARN("not implemented");
    return {};
}

std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::visitConstantAssignmentExpression(ConfAnalyzer::ConstantAssignmentExpression const& node) noexcept {
    WARN("not implemented");
    return {};
}

std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::visitStringExpression(ConfAnalyzer::StringExpression const& node) noexcept {
    WARN("not implemented");
    return {};
}

std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::visitNumberExpression(ConfAnalyzer::NumberExpression const& node) noexcept {
    WARN("not implemented");
    return {};
}

std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::visitPathExpression(ConfAnalyzer::PathExpression const& node) noexcept {
    WARN("not implemented");
    return {};
}

std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::visitShellExpression(ConfAnalyzer::ShellExpression const& node) noexcept {
    WARN("not implemented");
    return {};
}


std::expected<void, ConfAnalyzer::Error> ConfAnalyzer::typeCheckFunctionArguments(std::vector<ConfAnalyzer::AstType> const& argument_nodes, ConfAnalyzer::KeywordSchema const& schema) noexcept {
    using enum Error;
    using TokenKindResult = std::expected<ConfAnalyzer::TokenKind, Error>;

    int const arity_diff = static_cast<int>(schema.arity) - static_cast<int>(argument_nodes.size());
    if (arity_diff != 0) {
        return std::unexpected(FUNCTION_ARITY_MISMATCH);
    }

    auto const visitor = Visitors {
        [](StringExpression const& string_expression) -> TokenKindResult { return string_expression.token.kind; },
        [](NumberExpression const& number_expression) -> TokenKindResult { return number_expression.token.kind; },
        [](PathExpression const& path_expression)     -> TokenKindResult { return path_expression.token.kind; },
        [](ShellExpression const& shell_expression)   -> TokenKindResult { return shell_expression.command.kind; },
        [](auto const&)                               -> TokenKindResult { return std::unexpected(FUNCTION_INVALID_EXPRESSION); }
    };

    for (auto const& [node, allowed_types] : std::views::zip(argument_nodes, schema.parameters)) {
        auto const argument_kind_result = std::visit(visitor, *node);
        if (!argument_kind_result) {
            return std::unexpected(argument_kind_result.error());
        }

        auto valid_argument = std::ranges::any_of(allowed_types, [argument_kind = argument_kind_result.value()](auto const& parameter_kind) {
            return parameter_kind == argument_kind;
        });

        if (!valid_argument) {
            return std::unexpected(FUNCTION_ARGUMENT_TYPE_MISMATCH);
        }
    }

    return {};
}
