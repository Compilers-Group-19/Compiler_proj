// Owner: PS/CSC/23/0210
#include "scanner.h"

void Scanner::stringLiteral(int startLine, int startCol) {
    std::string value;
    while (!atEnd() && peek() != '"') {
        if (peek() == '\n') {
            addError("Unterminated string literal", startLine, startCol);
            return;
        }
        char c = advance();
        if (c == '\\') {
            if (atEnd()) break;
            char e = advance();
            switch (e) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '0': value += '\0'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                default:
                    addError(std::string("Invalid escape sequence '\\") + e + "'", line_, col_ - 2);
                    break;
            }
        } else {
            value += c;
        }
    }
    if (atEnd()) {
        addError("Unterminated string literal", startLine, startCol);
        return;
    }
    advance();  // closing quote
    addToken(TokenType::STRING_LITERAL, value, startLine, startCol);
}
