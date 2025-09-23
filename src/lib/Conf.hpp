#pragma once

namespace Conf {

// Binary Operator Strings
static constexpr std::string_view STRING_EQUALS              = "=";
static constexpr std::string_view STRING_WALRUS              = ":=";

// Symmetric Delimiter Strings
static constexpr std::string_view STRING_OPEN_BRACE          = "{";
static constexpr std::string_view STRING_CLOSE_BRACE         = "}";
static constexpr std::string_view STRING_OPEN_DOUBLE_BRACE   = "{{";
static constexpr std::string_view STRING_CLOSE_DOUBLE_BRACE  = "}}";
static constexpr std::string_view STRING_OPEN_QUOTE          = "'";
static constexpr std::string_view STRING_CLOSE_QUOTE         = "'";
static constexpr std::string_view STRING_OPEN_DOUBLE_QUOTE   = "\"";
static constexpr std::string_view STRING_CLOSE_DOUBLE_QUOTE  = "\"";

// White Space Strings
static constexpr std::string_view STRING_TAB_FEED            = "\t";
static constexpr std::string_view STRING_LINE_FEED           = "\r\n";
static constexpr std::string_view STRING_VERTICAL_FEED       = "\v";
static constexpr std::string_view STRING_SPACE               = " ";

// Single Character Strings
static constexpr std::string_view STRING_SEMI_COLON          = ";";

// Single Delimiter Strings
static constexpr std::string_view STRING_COMMENT_SINGLE_LINE = "#";
static constexpr std::string_view STRING_ESCAPE_SEQUENCE     = "\\";

// Keyword Strings
static constexpr std::string_view STRING_KEYWORD_INCLUDE     = "include";


} // END OF NAMESPACE `Conf`
