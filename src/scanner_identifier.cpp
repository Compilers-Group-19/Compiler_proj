// Owner: PS/CSC/23/0214
#include <cctype>
#include "scanner.h"

const std::unordered_map<std::string, TokenType>& Scanner::keywords() {
    static const std::unordered_map<std::string, TokenType> table = {
        {"int", TokenType::KW_INT},       {"float", TokenType::KW_FLOAT},
        {"bool", TokenType::KW_BOOL},     {"string", TokenType::KW_STRING},
        {"void", TokenType::KW_VOID},     {"if", TokenType::KW_IF},
        {"else", TokenType::KW_ELSE},     {"while", TokenType::KW_WHILE},
        {"for", TokenType::KW_FOR},       {"return", TokenType::KW_RETURN},
        {"func", TokenType::KW_FUNC},     {"print", TokenType::KW_PRINT},
        {"read", TokenType::KW_READ},     {"true", TokenType::KW_TRUE},
        {"false", TokenType::KW_FALSE},
    };
    return table;
}

// Identifier: a letter or '_' followed by letters, digits or '_'.
void Scanner::identifier(int startLine, int startCol) {
    std::string text;
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        text += advance();
    }
    auto it = keywords().find(text);
    addToken(it != keywords().end() ? it->second : TokenType::IDENTIFIER, text, startLine, startCol);
}
