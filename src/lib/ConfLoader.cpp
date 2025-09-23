#include <concepts>
#include <expected>
#include <print>
#include <utility>
#include <variant>

namespace detail {
    template<typename T>
    concept HasNodeKind = requires (T t) {
        { t.kind };
    };
}

ConfLoader::ConfLoader(std::string_view config_file_path) noexcept
    : m_ast{nullptr}
    , m_config_file_path{config_file_path.data()}
{}

std::expected<void, ConfLoader::Error> ConfLoader::load() {
    using enum Error;

    auto conf_lexer = ConfLoader::LexerType::lexFile(m_config_file_path.c_str());
    if (!conf_lexer) {
        return std::unexpected(FAILED_TO_LEX);
    }

    auto ast = ConfLoader::ParserType::parseTokenList(conf_lexer.value());
    if (!ast) {
        return std::unexpected(FAILED_TO_PARSE);
    }

    std::swap(m_ast, ast.value());

    auto const result = this->preProcess();

    return result;
}

std::expected<void, ConfLoader::Error> ConfLoader::preProcess() {
    return this->visitIncludes(m_ast);
}

std::expected<void, ConfLoader::Error> ConfLoader::visitIncludes(ConfLoader::AstType& ast) {
    using enum Error;
    using enum ConfLoader::NodeKindType;
    using enum ConfLoader::TokenKindType;
    using KeywordBinOp = ConfLoader::ParserType::KeywordBinOp;
    using RootBlock = ConfLoader::ParserType::RootBlock;
    using NamedBlock = ConfLoader::ParserType::NamedBlock;

    if (!ast) {
        return std::unexpected(NULL_AST_POINTER);
    }

    auto const visitor = Visitors {
        [&](KeywordBinOp& keyword_bin_op) -> std::expected<void, Error> {
            if (keyword_bin_op.keyword.data != Conf::STRING_KEYWORD_INCLUDE) {
                return {};
            }

            auto conf_lexer = ConfLoader::LexerType::lexFile(keyword_bin_op.expression.data);
            if (!conf_lexer) {
                return std::unexpected(FAILED_TO_LEX);
            }

            auto include_ast = ConfLoader::ParserType::parseTokenList(conf_lexer.value());
            if (!include_ast) {
                return std::unexpected(FAILED_TO_PARSE);
            }

            auto recursed_include_ast = this->visitIncludes(include_ast.value());
            if (!recursed_include_ast) {
                return recursed_include_ast;
            }

            return this->visitSpliceIncludes(keyword_bin_op.parent, keyword_bin_op.me, include_ast.value());
        },

        [&]<detail::HasNodeKind T>(T& ast) -> std::expected<void, Error> {
            constexpr bool has_children = std::same_as<T, typename ConfParser::RootBlock>
                || std::same_as<T, typename ConfParser::NamedBlock>;

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

std::expected<void, ConfLoader::Error> ConfLoader::visitSpliceIncludes(ConfLoader::NodeType* parent, ConfLoader::NodeType* me, ConfLoader::AstType& splice) {
    using enum Error;

    if (!parent || !splice) {
        return {};
    }

    auto const visitor = Visitors {
        [this, &me, &splice]<detail::HasNodeKind T>(T& ast) -> std::expected<void, Error> {
            constexpr bool has_children = std::same_as<T, typename ConfParser::RootBlock>
                or std::same_as<T, typename ConfParser::NamedBlock>;

            if constexpr (has_children) {
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
            }

            return {};
        }
    };

    return std::visit(visitor, *parent);
}

ConfLoader::AstType const& ConfLoader::ast() {
    return m_ast;
}
