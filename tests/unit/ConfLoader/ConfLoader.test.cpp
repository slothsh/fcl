#include <catch2/catch_test_macros.hpp>
#include <print>
#include <variant>

template<typename T>
concept SimpleExpression = std::same_as<T, ConfParser::StringExpression>
    || std::same_as<T, ConfParser::PathExpression>;

void printAst(ConfLoader::AstType const& node, int indent = 0) {
    if (!node) return;

    std::println("{:>{}}me: {}", " ", indent, (void*)node.get());
    auto const visitor = Visitors {
        [&](ConfParser::FilePathRootBlock const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.file_path.c_str());
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
        [&](ConfParser::FilePathSubRootBlock const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.file_path.c_str());
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
        [&](ConfParser::NamedBlock const& node) {
            std::println("{:>{}}{}: {}", " ", indent, node.kind, node.name.data);
            std::println("{:>{}}parent: {}", " ", indent, (void*)node.parent);
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
        [&](ConfParser::KeywordStatement const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.keyword.data);
            std::println("{:>{}}parent: {}", " ", indent, (void*)node.parent);
            for (auto const& child : node.arguments) {
                printAst(child, indent + 4);
            }
        },
        [&](ConfParser::VariableAssignmentExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.name.data);
            std::println("{:>{}}parent: {}", " ", indent, (void*)node.parent);
            printAst(node.expression, indent + 4);
        },
        [&](ConfParser::ConstantAssignmentExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.name.data);
            std::println("{:>{}}parent: {}", " ", indent, (void*)node.parent);
            printAst(node.expression, indent + 4);
        },
        [&](ConfParser::ShellExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent, node.command.data);
            std::println("{:>{}}parent: {}", " ", indent, (void*)node.parent);
        },
        [&](ConfParser::NumberExpression const& node) {
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
        auto conf_loader = ConfLoader{"./data/Config.conf"};
        auto const result = conf_loader.load();

        printAst(conf_loader.ast());

        if (!result) {
            FAIL("Loading config @ ./data/Config.conf failed");
        }
        
        REQUIRE(1 == 1);
    }
}
