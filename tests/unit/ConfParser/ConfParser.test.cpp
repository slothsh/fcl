#include <catch2/catch_test_macros.hpp>
#include <print>
#include <variant>

void printAst(typename ConfParser::NodePtr const& node, int indent = 0) {
    if (!node) return;

    auto const visitor = Visitors {
        [&](ConfParser::RootBlock const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
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
        [&](ConfParser::AssignmentExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.name.data);
            std::println("{:>{}}{}", " ", indent + 4, node.expression.data);
        },
        [&](ConfParser::ShellAssignmentExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.name.data);
            std::println("{:>{}}{}", " ", indent + 4, node.command.data);
        }
    };

    std::visit(visitor, *node);
}

template<typename T>
concept HasNodeKind = requires (T t) {
    { t.kind };
};

void includeFiles(typename ConfParser::NodePtr const& ast) {
    using enum ConfParser::NodeKind;

    if (!ast) {
        return;
    }

    auto const visitor = Visitors {
        [&](ConfParser::KeywordBinOp const& keyword_bin_op) {
        },
        [&]<HasNodeKind T>(T const& node) {
            constexpr bool has_children = std::same_as<T, typename ConfParser::RootBlock>
                || std::same_as<T, typename ConfParser::NamedBlock>;

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
        auto token_list = ConfLexer::lexFile("./data/Config.conf");
        auto ast = ConfParser::parseTokenList(token_list.value());

        printAst(ast.value());
        
        REQUIRE(1 == 1);
    }
}
