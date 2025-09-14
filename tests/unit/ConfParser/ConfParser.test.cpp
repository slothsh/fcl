#include "lib/Core.hpp"
#include <catch2/catch_test_macros.hpp>
#include <print>
#include <variant>

void printAst(typename ConfParser::NodePtr const& node, int indent = 0) {
    if (!node) return;

    const auto visitor = Visitors {
        [&](ConfParser::NamedBlock const& node) {
            std::println("{:>{}}{}: {}", " ", indent, node.kind, node.name.data);
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
        [&](ConfParser::KeywordBinOp const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.keyword.data);
            std::println("{:>{}}{}", " ", indent + 4, node.expression.data);
        },
        [&](ConfParser::NamedDeclaration const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.name.data);
            std::println("{:>{}}{}", " ", indent + 4, node.expression.data);
        }
    };

    std::visit(visitor, *node);
}

TEST_CASE("Parse Simple Configuration File", "[confparser]") {
    SECTION("Top-Level Block Can Be Parsed") {
        auto token_list = ConfLexer::lexFile("./data/Config.conf");
        auto ast = ConfParser::parse(token_list.value());

        printAst(ast.value());
        
        REQUIRE(1 == 1);
    }
}
