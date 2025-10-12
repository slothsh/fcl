#include <expected>
#include <utility>
#include <variant>

inline namespace {
    using namespace Conf;
    using namespace Conf::Language;
    using Error = ConfEvaluator::Error;
    using enum Error;
    using enum TokenKind;
}

ConfEvaluator::ConfEvaluator(std::string_view config_file_path) noexcept
    : m_ast{nullptr}
    , m_config_file_path{std::filesystem::weakly_canonical(config_file_path)}
{}

std::expected<void, Error> ConfEvaluator::load() {
    auto conf_tokenizer = ConfTokenizer::tokenizeFile(m_config_file_path.c_str());
    if (!conf_tokenizer) {
        return std::unexpected(FAILED_TO_TOKENIZE);
    }

    auto ast = ConfParser::parseTokenListWithFilePathRoot(conf_tokenizer.value(), m_config_file_path.c_str());
    if (!ast) {
        return std::unexpected(FAILED_TO_PARSE);
    }

    std::swap(m_ast, ast.value());

    return this->analyzeAst()
        .and_then([this]() { return this->preProcess(); })
        .and_then([this]() { return this->evaluate(m_ast); });
}

std::expected<void, Error> ConfEvaluator::analyzeAst() const {
    auto const analyzer = ConfAnalyzer{m_ast};

    return analyzer
        .analyze()
        .transform_error([](auto const& error) {
            return FAILED_TO_ANALYZE;
        });
}

std::expected<void, Error> ConfEvaluator::preProcess() {
    return this->visitIncludes(m_ast);
}

std::expected<void, Error> ConfEvaluator::evaluate(NodePtr& ast) const {
    using ReturnType = std::expected<void, Error>;

    if (!ast) {
        return std::unexpected(NULL_AST_POINTER);
    }

    auto const visitor = Visitors {
        [](KeywordStatement& keyword_statement) -> ReturnType {
            if (keyword_statement.keyword.kind != KEYWORD_PRINT) {
                return {};
            }

            auto const& arg1 = get_argument_checked<KeywordPrint::StringArg>(keyword_statement.arguments);
            auto const& arg2 = get_argument_checked<KeywordPrint::NumberArg>(keyword_statement.arguments);
            INFO("{} {}", arg1, arg2);

            return {};
        },

        [&]<HasNodeKind T>(T& ast) -> std::expected<void, Error> {
            constexpr bool has_children = AnyOf<
                T,
                FilePathRootBlock,
                FilePathSubRootBlock,
                NamedBlock
            >;

            if constexpr (has_children) {
                for (auto& child : ast.nodes) {
                    auto _ = this->evaluate(child);
                }
            }

            return {};
        }
    };

    return std::visit(visitor, *ast);
}

std::expected<void, Error> ConfEvaluator::visitIncludes(NodePtr& ast) {
    if (!ast) {
        return std::unexpected(NULL_AST_POINTER);
    }

    auto const visitor = Visitors {
        [&](KeywordStatement& keyword_statement) -> std::expected<void, Error> {
            if (keyword_statement.keyword.data != STRING_KEYWORD_INCLUDE) {
                return {};
            }

            auto const nearest_root_ancestor = ConfEvaluator::findNearestRootAncestor(keyword_statement.me);
            if (!nearest_root_ancestor) {
                return std::unexpected(FAILED_TO_RESOLVE_INCLUDE_PATH);
            }

            auto const parent_directory = std::visit(
                Visitors {
                    []<IsRootBlock T>(T& root_block) -> std::expected<PathType, Error> {
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

            auto const& path = get_argument_checked<KeywordInclude::FilePathArg>(keyword_statement.arguments);

            auto const include_path = std::filesystem::weakly_canonical(parent_directory.value() / path);

            auto conf_tokenizer = ConfTokenizer::tokenizeFile(include_path.c_str());
            if (!conf_tokenizer) {
                return std::unexpected(FAILED_TO_TOKENIZE);
            }

            auto include_ast = ConfParser::parseTokenListWithFilePathSubRoot(conf_tokenizer.value(), include_path.c_str());
            if (!include_ast) {
                return std::unexpected(FAILED_TO_PARSE);
            }

            auto recursed_include_ast = this->visitIncludes(include_ast.value());
            if (!recursed_include_ast) {
                return recursed_include_ast;
            }

            return this->visitSpliceIncludes(keyword_statement.parent, keyword_statement.me, include_ast.value());
        },

        [&]<HasNodeKind T>(T& ast) -> std::expected<void, Error> {
            constexpr bool has_children = AnyOf<
                T,
                FilePathRootBlock,
                FilePathSubRootBlock,
                NamedBlock
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

std::expected<void, Error> ConfEvaluator::visitSpliceIncludes(Node* parent, Node* me, NodePtr& splice) {
    if (!parent || !splice) {
        return {};
    }

    auto const visitor = Visitors {
        [this, &me, &splice]<HasChildren T>(T& ast) -> std::expected<void, Error> {
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

std::expected<Node*, Error> ConfEvaluator::findNearestRootAncestor(Node* me) {
    using ExpectedType = std::expected<Node*, Error>;

    if (!me) {
        return std::unexpected(NULL_SELF_POINTER);
    }

    auto const visitor = Visitors {
        [this]<IsRootBlock T>(T& ast) -> ExpectedType {
            return ast.me;
        },

        [this]<HasParent T>(T& ast) -> ExpectedType requires (!IsRootBlock<T>) {
            return this->findNearestRootAncestor(ast.parent);
        }
    };

    return std::visit(visitor, *me);
}

NodePtr const& ConfEvaluator::ast() const {
    return m_ast;
}
