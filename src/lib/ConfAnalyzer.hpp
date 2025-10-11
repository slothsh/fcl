#pragma once

#include <array>
#include <expected>
#include <vector>

class ConfAnalyzer {
public:
    using AstType = typename ConfParser::NodePtr;
    using NodeKind = typename ConfParser::NodeKind;
    using Token = typename ConfLexer::Token;
    using TokenKind = typename ConfLexer::TokenKind;

    using FilePathRootBlock = typename ConfParser::FilePathRootBlock;
    using FilePathSubRootBlock = typename ConfParser::FilePathSubRootBlock;
    using NamedBlock = typename ConfParser::NamedBlock;
    using KeywordStatement = typename ConfParser::KeywordStatement;
    using VariableAssignmentExpression = typename ConfParser::VariableAssignmentExpression;
    using ConstantAssignmentExpression = typename ConfParser::ConstantAssignmentExpression;
    using StringExpression = typename ConfParser::StringExpression;
    using NumberExpression = typename ConfParser::NumberExpression;
    using PathExpression = typename ConfParser::PathExpression;
    using ShellExpression = typename ConfParser::ShellExpression;

    struct KeywordSchema {
        size_t arity;
        std::array<std::array<TokenKind, 128>, 128> parameters;
    };

    enum class Error {
        FILE_PATH_NOT_ABSOLUTE,
        FUNCTION_ARITY_MISMATCH,
        FUNCTION_INVALID_EXPRESSION,
        FUNCTION_ARGUMENT_TYPE_MISMATCH,
    };

    inline static constexpr auto KEYWORD_INCLUDE_ARGS_SCHEMA = KeywordSchema {
        .arity = 1,
        .parameters = {
            { TokenKind::PATH_LITERAL_ABSOLUTE, TokenKind::PATH_LITERAL_RELATIVE }
        },
    };

    explicit ConfAnalyzer(AstType const& ast);

    std::expected<void, Error> analyze() const noexcept;

    ConfAnalyzer() = delete;
    ConfAnalyzer(ConfAnalyzer const&) = delete;
    ConfAnalyzer(ConfAnalyzer&&) = delete;
    ConfAnalyzer& operator=(ConfAnalyzer const&) = delete;
    ConfAnalyzer& operator=(ConfAnalyzer&&) = delete;

    static std::expected<void, Error> visitFilePathRootBlock(FilePathRootBlock const& node) noexcept;
    static std::expected<void, Error> visitFilePathSubRootBlock(FilePathSubRootBlock const& node) noexcept;
    static std::expected<void, Error> visitNamedBlock(NamedBlock const& node) noexcept;
    static std::expected<void, Error> visitKeywordStatement(KeywordStatement const& node) noexcept;
    static std::expected<void, Error> visitVariableAssignmentExpression(VariableAssignmentExpression const& node) noexcept;
    static std::expected<void, Error> visitConstantAssignmentExpression(ConstantAssignmentExpression const& node) noexcept;
    static std::expected<void, Error> visitStringExpression(StringExpression const& node) noexcept;
    static std::expected<void, Error> visitNumberExpression(NumberExpression const& node) noexcept;
    static std::expected<void, Error> visitPathExpression(PathExpression const& node) noexcept;
    static std::expected<void, Error> visitShellExpression(ShellExpression const& node) noexcept;

    static std::expected<void, Error> typeCheckFunctionArguments(std::vector<AstType> const& arguments, KeywordSchema const& argument_types) noexcept;

private:
    AstType const& m_ast;
};
