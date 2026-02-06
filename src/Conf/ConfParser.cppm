export module Conf:Parser;

import :Common;
import std;

export class ConfParser {
public:
    using Token = Conf::Language::Token;
    using NodePtr = Conf::Language::NodePtr;
    using NumberType = Conf::Language::NumberType;

    enum class Error {
        TODO,
        NUMBER_CONVERSION_ERROR,
    };

    static std::optional<NodePtr> parseTokenListWithFilePathRoot(std::vector<Token> const& token_list, std::string_view file_path);
    static std::optional<NodePtr> parseTokenListWithFilePathSubRoot(std::vector<Token> const& token_list, std::string_view file_path);
    static std::expected<NumberType, Error> convertTokenToNumber(Token const& token) noexcept;

    explicit ConfParser(std::vector<Token> const& token_list);

    std::expected<NodePtr, Error> parse(NodePtr& parent);

    std::optional<NodePtr> takeNamedBlock(Token const& token, NodePtr& parent);
    std::optional<NodePtr> takeKeywordStatement(Token const& token, NodePtr& parent);
    std::optional<NodePtr> takeVariableAssignmentExpression(Token const& token, NodePtr& parent);
    std::optional<NodePtr> takeConstantAssignmentExpression(Token const& token, NodePtr& parent);
    std::optional<NodePtr> takeStringExpression(Token const& token, NodePtr& parent);
    std::optional<NodePtr> takeNumberExpression(Token const& token, NodePtr& parent);
    std::optional<NodePtr> takePathExpression(Token const& token, NodePtr& parent);
    std::optional<NodePtr> takeShellExpression(Token const& token, NodePtr& parent);
    std::optional<NodePtr> takeSymbolReferenceExpression(Token const& token, NodePtr& parent);

private:
    std::size_t m_cursor;
    std::vector<Token> const& m_token_list;
};
