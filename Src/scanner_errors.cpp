// Owner: Member 9 - Error reporting
// A lexical error does not stop the scanner. It is recorded as an ERROR token
// (the lexeme holds the message) and counted, so one run can report several
// errors. main.cpp prints them and sets the exit code.

#include "scanner.h"

void Scanner::addError(const std::string& message, int line, int col) {
    errors_++;
    tokens_.push_back(Token{TokenType::ERROR, message, line, col});
}
