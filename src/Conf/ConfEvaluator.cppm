module;

#include "../Macros.hpp"

export module Conf:Evaluator;

import :Common;
import :Evaluator.SymbolTable;
import Containers;
import std;

inline namespace {
    using namespace Conf::Language;
}

export class ConfEvaluator {
public:
    enum class Error {
        FAILED_TO_ANALYZE,
        FAILED_TO_TOKENIZE,
        FAILED_TO_PARSE,
        FAILED_TO_RESOLVE_INCLUDE_PATH,
        FAILED_TO_RESOLVE_INCLUDES,
        FAILED_TO_BUILD_SYMBOL_TABLE,
        FAILED_TO_GET_ARGUMENT,
        NULL_AST_POINTER,
        NULL_SELF_POINTER,
        CHILD_NOT_FOUND,
    };

    ConfEvaluator() = delete;

    explicit ConfEvaluator(std::string_view config_file_path) noexcept;

    std::expected<void, Error> load();
    std::expected<void, Error> analyzeAst() const;
    std::expected<void, Error> preProcess();
    std::expected<void, Error> evaluate(NodePtr& ast) const;

    std::expected<void, Error> visitSymbolTable(NodePtr& ast);
    std::expected<void, Error> visitIncludes(NodePtr& ast);
    std::expected<void, Error> visitSpliceIncludes(Node* parent, Node* me, NodePtr& splice);
    std::expected<Node*, Error> findNearestRootAncestor(Node* me);

    template<IsFunctionArgument ArgName, IsSubscriptable<typename ArgName::VariantType> Args>
    auto& getArgument(Args const& arguments) const {
        return ArgName::unwrap(std::get<typename ArgName::InnerType>(*arguments[ArgName::Index]));
    }

    template<IsFunctionArgument ArgName, IsSubscriptable<typename ArgName::VariantType> Args>
    auto& getArgumentChecked(Args const& arguments) const {
        auto const visitor = Visitors {
            // [this, &arguments](SymbolReferenceExpression& symbol_expression) -> ArgName::ReturnType {
            //     auto const data = m_symbol_table.lookup(std::string_view{symbol_expression.symbol.data}, m_namespace_buffer);
            //     if (!data) {
            //         return {};
            //     }
            //     return {};
            // },
            [&arguments](ArgName::InnerType& inner) -> ArgName::ReturnType {
                return ArgName::unwrap(inner);
            },
            [](auto&& node) -> ArgName::ReturnType {
                TODO("unreachable: {}", node.kind);
            }
        };

        return std::visit(visitor, *arguments.at(ArgName::Index));
    }

    NodePtr const& ast() const;

    SymbolTable m_symbol_table;
    NamespaceType m_namespace_buffer;
private:
    NodePtr m_ast;
    PathType m_config_file_path;
};
