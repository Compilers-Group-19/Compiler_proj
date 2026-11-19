// Owner: PS/CSC/23/0208

#include <cctype>
#include "scanner.h"

void Scanner::scanToken() {
    int startLine = line_, startCol = col_;
    char c = advance();

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        pos_--; col_--;  // let identifier() re-read the first character
        identifier(startLine, startCol);
        return;
    }
    if (std::isdigit(static_cast<unsigned char>(c))) {
        pos_--; col_--;
        number(startLine, startCol);
        return;
    }

    auto simple = [&](TokenType t) { addToken(t, std::string(1, c), startLine, startCol); };

    switch (c) {
        case '"': stringLiteral(startLine, startCol); return;
        case '+': simple(TokenType::PLUS); return;
        case '-': simple(TokenType::MINUS); return;
        case '*': simple(TokenType::STAR); return;
        case '/': simple(TokenType::SLASH); return;
        case '%': simple(TokenType::PERCENT); return;
        case '(': simple(TokenType::LPAREN); return;
        case ')': simple(TokenType::RPAREN); return;
        case '{': simple(TokenType::LBRACE); return;
        case '}': simple(TokenType::RBRACE); return;
        case '[': simple(TokenType::LBRACKET); return;
        case ']': simple(TokenType::RBRACKET); return;
        case ',': simple(TokenType::COMMA); return;
        case ';': simple(TokenType::SEMICOLON); return;
        case ':': simple(TokenType::COLON); return;
        case '.': simple(TokenType::DOT); return;
        case '=':
            if (match('=')) addToken(TokenType::EQ, "==", startLine, startCol);
            else simple(TokenType::ASSIGN);
            return;
        case '!':
            if (match('=')) addToken(TokenType::NEQ, "!=", startLine, startCol);
            else simple(TokenType::NOT);
            return;
        case '<':
            if (match('=')) addToken(TokenType::LE, "<=", startLine, startCol);
            else simple(TokenType::LT);
            return;
        case '>':
            if (match('=')) addToken(TokenType::GE, ">=", startLine, startCol);
            else simple(TokenType::GT);
            return;
        case '&':
            if (match('&')) addToken(TokenType::AND, "&&", startLine, startCol);
            else addError("Unexpected character '&' (did you mean '&&'?)", startLine, startCol);
            return;
        case '|':
            if (match('|')) addToken(TokenType::OR, "||", startLine, startCol);
            else addError("Unexpected character '|' (did you mean '||'?)", startLine, startCol);
            return;
        default:
            addError(std::string("Unexpected character '") + c + "'", startLine, startCol);
            return;
    }
}
