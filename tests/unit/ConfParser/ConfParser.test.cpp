#include <catch2/catch_test_macros.hpp>
#include <print>
#include <variant>

template<typename T>
concept HasNodeKind = requires (T t) {
    { t.kind };
};

template<typename T>
concept SimpleExpression = std::same_as<T, ConfParser::StringExpression>
    || std::same_as<T, ConfParser::PathExpression>;

void printAst(typename ConfParser::NodePtr const& node, int indent = 0) {
    if (!node) return;

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
            for (auto const& child : node.nodes) {
                printAst(child, indent + 4);
            }
        },
        [&](ConfParser::KeywordStatement const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.keyword.data);
            for (auto const& child : node.arguments) {
                printAst(child, indent + 4);
            }
        },
        [&](ConfParser::VariableAssignmentExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.name.data);
            printAst(node.expression, indent + 4);
        },
        [&](ConfParser::ConstantAssignmentExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.name.data);
            printAst(node.expression, indent + 4);
        },
        [&](ConfParser::ShellExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.command.data);
        },
        [&](ConfParser::NumberExpression const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.value);
            std::println("{:>{}}{}", " ", indent + 4, node.token.data);
        },
        [&]<SimpleExpression T>(T const& node) {
            std::println("{:>{}}{}", " ", indent, node.kind);
            std::println("{:>{}}{}", " ", indent + 4, node.token.data);
        },
    };

    std::visit(visitor, *node);
}

void includeFiles(typename ConfParser::NodePtr const& ast) {
    using enum ConfParser::NodeKind;

    if (!ast) {
        return;
    }

    auto const visitor = Visitors {
        [&](ConfParser::KeywordStatement const& keyword_statement) {
        },
        [&]<HasNodeKind T>(T const& node) {
            constexpr bool has_children = std::same_as<T, typename ConfParser::FilePathRootBlock>
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
        auto ast = ConfParser::parseTokenListWithFilePathRoot(token_list.value(), "./data/Config.conf");

        printAst(ast.value());
        
        REQUIRE(1 == 1);
    }
}
