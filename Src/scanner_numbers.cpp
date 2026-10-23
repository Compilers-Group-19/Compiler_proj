// Owner: PS/CSC/23/0211

#include <cctype>
#include "scanner.h"

void Scanner::number(int startLine, int startCol)
{
    std::string text;
    while (std::isdigit(static_cast<unsigned char>(peek())))
        text += advance();

    bool isFloat = false;
    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek(1))))
    {
        isFloat = true;
        text += advance();
        while (std::isdigit(static_cast<unsigned char>(peek())))
            text += advance();
    }

    if (std::isalpha(static_cast<unsigned char>(peek())) || peek() == '_')
    {
        while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
            text += advance();
        addError("Malformed number or identifier '" + text + "'", startLine, startCol);
        return;
    }
    addToken(isFloat ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL, text, startLine, startCol);
}
