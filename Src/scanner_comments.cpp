// Owner: Member 4 - Whitespace and comments
// Skips spaces, tabs, newlines, "// line" comments and "/* block */" comments.
// An unterminated block comment is reported as an error token.

#include "scanner.h"

void Scanner::skipWhitespaceAndComments() {
    while (!atEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && peek(1) == '/') {
            // Line comment: skip to the end of the line
            while (!atEnd() && peek() != '\n') advance();
        } else if (c == '/' && peek(1) == '*') {
            // Block comment: skip until the closing */
            int startLine = line_, startCol = col_;
            advance();
            advance();
            bool closed = false;
            while (!atEnd()) {
                if (peek() == '*' && peek(1) == '/') {
                    advance();
                    advance();
                    closed = true;
                    break;
                }
                advance();
            }
            if (!closed) addError("Unterminated block comment", startLine, startCol);
        } else {
            break;
        }
    }
}
