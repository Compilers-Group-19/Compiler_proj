// Owner: PS/CSC/23/0206 - Scanner core
// Declares the Scanner class used by every scanner source file.
// Each scanner_*.cpp file implements one group of these member functions.

#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "token.h"

// Phase 1: lexical analysis. Turns source text into a stream of tokens.
class Scanner {
public:
    explicit Scanner(const std::string& source);

    // Scan the whole input. The last token is always END_OF_FILE.
    // Lexical errors are returned as ERROR tokens (scanning continues).
    std::vector<Token> scanTokens();

    // Number of lexical errors found so far.
    int errorCount() const { return errors_; }

private:
    std::string src_;
    size_t pos_ = 0;
    int line_ = 1;
    int col_ = 1;
    int errors_ = 0;
    std::vector<Token> tokens_;

    // scanner_identifiers.cpp (Member 5)
    static const std::unordered_map<std::string, TokenType>& keywords();

    // scanner_core.cpp (Member 3)
    bool atEnd() const { return pos_ >= src_.size(); }
    char peek(size_t offset = 0) const;
    char advance();
    bool match(char expected);
    void addToken(TokenType type, const std::string& lexeme, int line, int col);

    // scanner_operators.cpp (Member 8)
    void scanToken();

    // scanner_comments.cpp (Member 4)
    void skipWhitespaceAndComments();

    // scanner_identifiers.cpp (Member 5)
    void identifier(int startLine, int startCol);

    // scanner_numbers.cpp (Member 6)
    void number(int startLine, int startCol);

    // scanner_strings.cpp (Member 7)
    void stringLiteral(int startLine, int startCol);

    // scanner_errors.cpp (Member 9)
    void addError(const std::string& message, int line, int col);
};

#endif
