// Owner: PS/CSC/23/0209

#include "scanner.h"

void Scanner::addError(const std::string& message, int line, int col) {
    errors_++;
    tokens_.push_back(Token{TokenType::ERROR, message, line, col});
}
