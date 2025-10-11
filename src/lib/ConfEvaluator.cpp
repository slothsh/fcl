#include "Concepts.hpp"
#include <concepts>
#include <expected>
#include <utility>
#include <variant>

namespace detail {
    template<typename T>
    concept HasNodeKind = requires (T t) {
        { t.kind } -> std::same_as<ConfEvaluator::NodeKindType&>;
    };

    template<typename T>
    concept IsRootBlock = AnyOf<
        T,
        ConfEvaluator::ParserType::FilePathRootBlock,
        ConfEvaluator::ParserType::FilePathSubRootBlock
    >;

    template<typename T>
    concept HasParent = requires (T t) {
        { t.parent } -> std::same_as<ConfEvaluator::NodeType*&>;
    };

    template<typename T>
    concept HasChildren = requires (T t) {
        { t.nodes } -> std::same_as<std::vector<ConfEvaluator::ParserType::NodePtr>&>;
    };
}

ConfEvaluator::ConfEvaluator(std::string_view config_file_path) noexcept
    : m_ast{nullptr}
    , m_config_file_path{std::filesystem::weakly_canonical(config_file_path)}
{}

std::expected<void, ConfEvaluator::Error> ConfEvaluator::load() {
    using enum Error;

    auto conf_lexer = ConfEvaluator::LexerType::lexFile(m_config_file_path.c_str());
    if (!conf_lexer) {
        return std::unexpected(FAILED_TO_LEX);
    }

    auto ast = ConfEvaluator::ParserType::parseTokenListWithFilePathRoot(conf_lexer.value(), m_config_file_path.c_str());
    if (!ast) {
        return std::unexpected(FAILED_TO_PARSE);
    }

    std::swap(m_ast, ast.value());

    return this->analyzeAst()
        .and_then([this](){ return this->preProcess(); });
}

std::expected<void, ConfEvaluator::Error> ConfEvaluator::analyzeAst() const {
    using enum Error;

    auto const analyzer = ConfAnalyzer{m_ast};

    return analyzer
        .analyze()
        .transform_error([](auto const& error) {
            return FAILED_TO_ANALYZE;
        });
}

std::expected<void, ConfEvaluator::Error> ConfEvaluator::preProcess() {
    return this->visitIncludes(m_ast);
}

std::expected<void, ConfEvaluator::Error> ConfEvaluator::visitIncludes(ConfEvaluator::AstType& ast) {
    using enum Error;
    using enum ConfEvaluator::NodeKindType;
    using enum ConfEvaluator::TokenKindType;
    using KeywordStatement = ConfEvaluator::ParserType::KeywordStatement;
    using FilePathRootBlock = ConfEvaluator::ParserType::FilePathRootBlock;
    using FilePathSubRootBlock = ConfEvaluator::ParserType::FilePathSubRootBlock;
    using NamedBlock = ConfEvaluator::ParserType::NamedBlock;
    using StringExpression = ConfEvaluator::ParserType::StringExpression;
    using PathExpression = ConfEvaluator::ParserType::PathExpression;

    if (!ast) {
        return std::unexpected(NULL_AST_POINTER);
    }

    auto const visitor = Visitors {
        [&](KeywordStatement& keyword_statement) -> std::expected<void, Error> {
            if (keyword_statement.keyword.data != Conf::STRING_KEYWORD_INCLUDE) {
                return {};
            }

            auto const nearest_root_ancestor = ConfEvaluator::findNearestRootAncestor(keyword_statement.me);
            if (!nearest_root_ancestor) {
                return std::unexpected(FAILED_TO_RESOLVE_INCLUDE_PATH);
            }

            auto const parent_directory = std::visit(
                Visitors {
                    []<detail::IsRootBlock T>(T& root_block) -> std::expected<PathType, Error> {
                        return root_block.file_path.parent_path();
                    },
                    [](auto&&) -> std::expected<PathType, Error> {
                        return std::unexpected(FAILED_TO_RESOLVE_INCLUDE_PATH);
                    },
                },
                *nearest_root_ancestor.value()
            );

            if (!parent_directory) {
                return std::unexpected(FAILED_TO_RESOLVE_INCLUDE_PATH);
            }

            // TODO: handle argument unwrapping gracefully
            // TODO: check if PathExpression is a relative path
            auto const include_path = std::filesystem::weakly_canonical(
                parent_directory.value() / std::get<PathExpression>(*keyword_statement.arguments.front()).token.data
            );

            auto conf_lexer = ConfEvaluator::LexerType::lexFile(include_path.c_str());
            if (!conf_lexer) {
                return std::unexpected(FAILED_TO_LEX);
            }

            auto include_ast = ConfEvaluator::ParserType::parseTokenListWithFilePathSubRoot(conf_lexer.value(), include_path.c_str());
            if (!include_ast) {
                return std::unexpected(FAILED_TO_PARSE);
            }

            auto recursed_include_ast = this->visitIncludes(include_ast.value());
            if (!recursed_include_ast) {
                return recursed_include_ast;
            }

            return this->visitSpliceIncludes(keyword_statement.parent, keyword_statement.me, include_ast.value());
        },

        [&]<detail::HasNodeKind T>(T& ast) -> std::expected<void, Error> {
            constexpr bool has_children = AnyOf<
                T,
                ConfParser::FilePathRootBlock,
                ConfParser::FilePathSubRootBlock,
                ConfParser::NamedBlock
            >;

            if constexpr (has_children) {
                for (auto& child : ast.nodes) {
                    auto _ = this->visitIncludes(child);
                }
            }

            return {};
        }
    };

    return std::visit(visitor, *ast);
}

std::expected<void, ConfEvaluator::Error> ConfEvaluator::visitSpliceIncludes(ConfEvaluator::NodeType* parent, ConfEvaluator::NodeType* me, ConfEvaluator::AstType& splice) {
    using enum Error;

    if (!parent || !splice) {
        return {};
    }

    auto const visitor = Visitors {
        [this, &me, &splice]<detail::HasChildren T>(T& ast) -> std::expected<void, Error> {
            size_t position = -1;
            for (auto& child : ast.nodes) {
                if (child.get() != me) {
                    ++position;
                    continue;
                }
                ++position;
                break;
            }

            if (position == -1 || position >= ast.nodes.size()) {
                return std::unexpected(CHILD_NOT_FOUND);
            }

            std::swap(ast.nodes[position], splice);

            return {};
        },
        [](auto&&) -> std::expected<void, Error> { return {}; }
    };

    return std::visit(visitor, *parent);
}

std::expected<ConfEvaluator::NodeType*, ConfEvaluator::Error> ConfEvaluator::findNearestRootAncestor(ConfEvaluator::NodeType* me) {
    using enum Error;
    using enum NodeKindType;
    using ExpectedType = std::expected<NodeType*, Error>;

    if (!me) {
        return std::unexpected(NULL_SELF_POINTER);
    }

    auto const visitor = Visitors {
        [this]<detail::IsRootBlock T>(T& ast) -> ExpectedType {
            return ast.me;
        },

        [this]<detail::HasParent T>(T& ast) -> ExpectedType requires (!detail::IsRootBlock<T>) {
            return this->findNearestRootAncestor(ast.parent);
        }
    };

    return std::visit(visitor, *me);
}

ConfEvaluator::AstType const& ConfEvaluator::ast() const {
    return m_ast;
}
