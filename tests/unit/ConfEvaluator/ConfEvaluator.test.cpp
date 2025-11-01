#include <catch2/catch_test_macros.hpp>

import std;
import Conf;
import Traits;

using namespace Conf;
using namespace Conf::Language;

void printAst(NodePtr const& node, int indent = 0) {
    if (!node) return;

    std::println("{:>{}}me: {}", " ", indent, (void*)node.get());
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
            std::println("{:>{}}parent: {}", " ", indent, (void*)node.parent);
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
        [&](KeywordStatement const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.keyword.data);
            std::println("{:>{}}parent: {}", " ", indent, (void*)node.parent);
            for (auto const& child : node.arguments) {
                printAst(child, indent + 4);
            }
        },
        [&](VariableAssignmentExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.name.data);
            std::println("{:>{}}parent: {}", " ", indent, (void*)node.parent);
            printAst(node.expression, indent + 4);
        },
        [&](ConstantAssignmentExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.name.data);
            std::println("{:>{}}parent: {}", " ", indent, (void*)node.parent);
            printAst(node.expression, indent + 4);
        },
        [&](ShellExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.command.data);
            std::println("{:>{}}parent: {}", " ", indent, (void*)node.parent);
        },
        [&](NumberExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.value);
            std::println("{:>{}}{}", " ", indent, node.token.data);
        },
        [&]<SimpleExpression T>(T const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.token.data);
            std::println("{:>{}}parent: {}", " ", indent, (void*)node.parent);
        },
    };

    std::visit(visitor, *node);
}

TEST_CASE("Load Simple Configuration File", "[confloader]") {
    SECTION("Conf file can be loaded") {
        auto conf_evaluator = ConfEvaluator{"./data/Config.conf"};
        auto const result = conf_evaluator.load();

        // printAst(conf_evaluator.ast());

        if (!result) {
            FAIL("Loading config @ ./data/Config.conf failed");
        }

        for (auto const& [key, value] : conf_evaluator.m_symbol_table.table) {
            std::println("{}", key);
        }

        REQUIRE(1 == 1);
    }
}
