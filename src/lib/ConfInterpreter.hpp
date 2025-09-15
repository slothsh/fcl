#pragma once

class ConfInterpreter {
public:
    using AstType = typename ConfParser::NodePtr;
    using NodeKind = typename ConfParser::NodeKind;

    enum class Error {
        TODO,
    };

    ConfInterpreter() = delete;

    ConfInterpreter(AstType&& root);

    std::expected<void, Error> interpret();
    std::expected<void, Error> preProcess();

private:
    AstType m_ast;
};
