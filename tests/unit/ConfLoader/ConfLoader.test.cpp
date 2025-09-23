#include <catch2/catch_test_macros.hpp>
#include <print>
#include <variant>

void printAst(ConfLoader::AstType const& node, int indent = 0) {
    if (!node) return;

    std::println("{:>{}}me: {}", " ", indent, (void*)node.get());
    auto const visitor = Visitors {
        [&](ConfParser::RootBlock const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
        [&](ConfParser::NamedBlock const& node) {
            std::println("{:>{}}{}: {}", " ", indent, node.kind, node.name.data);
            std::println("{:>{}}parent: {}", " ", indent + 4, (void*)node.parent);
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
        [&](ConfParser::KeywordBinOp const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.keyword.data);
            std::println("{:>{}}{}", " ", indent + 4, node.expression.data);
            std::println("{:>{}}parent: {}", " ", indent + 4, (void*)node.parent);
        },
        [&](ConfParser::AssignmentExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.name.data);
            std::println("{:>{}}{}", " ", indent + 4, node.expression.data);
            std::println("{:>{}}parent: {}", " ", indent + 4, (void*)node.parent);
        },
        [&](ConfParser::NamedShellDeclaration const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.name.data);
            std::println("{:>{}}{}", " ", indent + 4, node.command.data);
            std::println("{:>{}}parent: {}", " ", indent + 4, (void*)node.parent);
        }
    };

    std::visit(visitor, *node);
}

TEST_CASE("Load Simple Configuration File", "[confloader]") {
    SECTION("Conf file can be loaded") {
        auto conf_loader = ConfLoader{"./data/Config.conf"};
        auto const result = conf_loader.load();

        printAst(conf_loader.ast());
        
        REQUIRE(1 == 1);
    }
}
