#include <catch2/catch_test_macros.hpp>

import std;
import Conf;
import Traits;

using namespace Conf::Language;

void printAst(typename ConfParser::NodePtr const& node, int indent = 0) {
    if (!node) return;

    auto const visitor = Visitors {
        [&](FilePathRootBlock const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.file_path.c_str());
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
        [&](FilePathSubRootBlock const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.file_path.c_str());
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
        [&](NamedBlock const& node) {
            std::println("{:>{}}{}: {}", " ", indent, node.kind, node.name.data);
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
        [&](KeywordStatement const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.keyword.data);
            for (auto const& child : node.arguments) {
                printAst(child, indent + 4);
            }
        },
        [&](VariableAssignmentExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.name.data);
            printAst(node.expression, indent + 4);
        },
        [&](ConstantAssignmentExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.name.data);
            printAst(node.expression, indent + 4);
        },
        [&](ShellExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.command.data);
        },
        [&](NumberExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.value);
            std::println("{:>{}}{}", " ", indent + 4, node.token.data);
        },
        [&]<SimpleExpression T>(T const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.token.data);
        },
        [&](SymbolReferenceExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.symbol.data);
        },
    };

    std::visit(visitor, *node);
}

void includeFiles(typename ConfParser::NodePtr const& ast) {
    if (!ast) {
        return;
    }

    auto const visitor = Visitors {
        [&](KeywordStatement const& keyword_statement) {
        },
        [&]<HasNodeKind T>(T const& node) {
            constexpr bool has_children = std::same_as<T, FilePathRootBlock>
                || std::same_as<T, NamedBlock>;

            if constexpr (has_children) {
                for (auto const& child : node.nodes) {
                    includeFiles(child);
                }
            }
        },
    };

    std::visit(visitor, *ast);
}

TEST_CASE("Parse Simple Configuration File", "[confparser]") {
    SECTION("Top-Level Block Can Be Parsed") {
        auto token_list = ConfTokenizer::tokenizeFile("./data/Config.conf");
        auto ast = ConfParser::parseTokenListWithFilePathRoot(token_list.value(), "./data/Config.conf");

        printAst(ast.value());
        
        REQUIRE(1 == 1);
    }
}
