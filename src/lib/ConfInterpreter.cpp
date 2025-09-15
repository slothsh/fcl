#include <variant>
#include <expected>
#include <concepts>
#include <cstdio>

namespace detail {
    using AstType = typename ConfInterpreter::AstType;
    using Error = typename ConfInterpreter::Error;
    using NodeKind = typename ConfInterpreter::NodeKind;
}

ConfInterpreter::ConfInterpreter(detail::AstType&& root)
    : m_ast{std::move(root)}
{}

std::expected<void, detail::Error> ConfInterpreter::interpret() {
    using enum Error;

    auto const result = this->preProcess();

    return result;
}


template<typename T>
concept HasNodeKind = requires (T t) {
    { t.kind };
};

static void processIncludes(detail::AstType const& ast) {
    using enum detail::NodeKind;

    if (!ast) {
        return;
    }

    const auto visitors = Visitors {
        [&](ConfParser::KeywordBinOp const& keyword_bin_op) {
            if (keyword_bin_op.kind != NAMED_SHELL_DECLARATION) {
                return;
            }

            FILE* cmd = popen(keyword_bin_op.expression.data.data(), "r");
            if (cmd == NULL) {
                std::println(stderr, "failed to run command");
                return;
            }

            char buffer[256];
            while (fgets(buffer, sizeof(buffer), cmd) != NULL) {
                std::println("{}", buffer);
            }

            pclose(cmd);
        },
        [&]<HasNodeKind T>(T const& node) {
            if (node.kind == NAMED_BLOCK) {
                for (auto const& child : reinterpret_cast<ConfParser::NamedBlock const&>(node).nodes) {
                    processIncludes(child);
                }
            }
        }
    };

    std::visit(visitors, *ast);
}

std::expected<void, detail::Error> ConfInterpreter::preProcess() {
    processIncludes(m_ast);
    return {};
}
