export module Conf:Analyzer;

import :Common;
import std;

export class ConfAnalyzer {
public:
    enum class Error {
        FILE_PATH_NOT_ABSOLUTE,
        FUNCTION_ARITY_MISMATCH,
        FUNCTION_INVALID_EXPRESSION,
        FUNCTION_ARGUMENT_TYPE_MISMATCH,
    };

    explicit ConfAnalyzer(Conf::Language::NodePtr const& ast);

    std::expected<void, Error> analyze() const noexcept;

    ConfAnalyzer() = delete;
    ConfAnalyzer(ConfAnalyzer const&) = delete;
    ConfAnalyzer(ConfAnalyzer&&) = delete;
    ConfAnalyzer& operator=(ConfAnalyzer const&) = delete;
    ConfAnalyzer& operator=(ConfAnalyzer&&) = delete;

    static std::expected<void, Error> visitFilePathRootBlock(Conf::Language::FilePathRootBlock const& node) noexcept;
    static std::expected<void, Error> visitFilePathSubRootBlock(Conf::Language::FilePathSubRootBlock const& node) noexcept;
    static std::expected<void, Error> visitNamedBlock(Conf::Language::NamedBlock const& node) noexcept;
    static std::expected<void, Error> visitKeywordStatement(Conf::Language::KeywordStatement const& node) noexcept;
    static std::expected<void, Error> visitVariableAssignmentExpression(Conf::Language::VariableAssignmentExpression const& node) noexcept;
    static std::expected<void, Error> visitConstantAssignmentExpression(Conf::Language::ConstantAssignmentExpression const& node) noexcept;
    static std::expected<void, Error> visitStringExpression(Conf::Language::StringExpression const& node) noexcept;
    static std::expected<void, Error> visitNumberExpression(Conf::Language::NumberExpression const& node) noexcept;
    static std::expected<void, Error> visitPathExpression(Conf::Language::PathExpression const& node) noexcept;
    static std::expected<void, Error> visitShellExpression(Conf::Language::ShellExpression const& node) noexcept;

    static std::expected<void, Error> typeCheckFunctionArguments(std::vector<Conf::Language::NodePtr> const& arguments, Conf::Language::KeywordSchema const& schema) noexcept;

private:
    Conf::Language::NodePtr const& m_ast;
};
