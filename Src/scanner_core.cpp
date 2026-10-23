// Owner: PS/CSC/23/0206

#include "scanner.h"

Scanner::Scanner(const std::string& source) : src_(source) {}

// Look at a character without consuming it ('\0' past the end).
char Scanner::peek(size_t offset) const {
    size_t i = pos_ + offset;
    return i < src_.size() ? src_[i] : '\0';
}

// Consume one character and keep line/column up to date.
char Scanner::advance() {
    char c = src_[pos_++];
    if (c == '\n') {
        line_++;
        col_ = 1;
    } else {
        col_++;
    }
    return c;
}

// Consume the next character only if it equals `expected`.
bool Scanner::match(char expected) {
    if (atEnd() || src_[pos_] != expected) return false;
    advance();
    return true;
}

void Scanner::addToken(TokenType type, const std::string& lexeme, int line, int col) {
    tokens_.push_back(Token{type, lexeme, line, col});
}

// Main loop: skip whitespace/comments, then scan one token, until end of input.
std::vector<Token> Scanner::scanTokens() {
    tokens_.clear();
    pos_ = 0;
    line_ = 1;
    col_ = 1;
    errors_ = 0;

    while (true) {
        skipWhitespaceAndComments();
        if (atEnd()) break;
        scanToken();
    }
    addToken(TokenType::END_OF_FILE, "", line_, col_);
    return tokens_;
}
