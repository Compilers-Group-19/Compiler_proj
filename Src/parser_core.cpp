// parser_core.cpp  (Part 4)
// The basic machinery every grammar rule relies on:
//   - looking at and consuming tokens: advance, match, matchAny, consume
//   - building and recording syntax errors: makeError, report
//   - panic-mode error recovery: synchronize
//
// Nothing here knows about the mini language's grammar; the rules themselves
// are in the other parser_*.cpp files.

#include "parser.h"
#include <string>
#include <utility>

namespace {

std::string describe(const Token& t) {
    switch (t.type) {
        case TokenType::END_OF_FILE: return "end of file";
        case TokenType::STRING_LITERAL: return "string \"" + t.lexeme + "\"";
        case TokenType::ERROR: return "invalid token";
        default: return "'" + t.lexeme + "'";
    }
}

}  // namespace

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {
    // Guarantee the stream ends with EOF so peek() is always valid.
    if (tokens_.empty() || tokens_.back().type != TokenType::END_OF_FILE) {
        int line = tokens_.empty() ? 1 : tokens_.back().line;
        int col = tokens_.empty() ? 1 : tokens_.back().column;
        tokens_.push_back(Token{TokenType::END_OF_FILE, "", line, col});
    }
}

const Token& Parser::advance() {
    if (!atEnd()) pos_++;
    return tokens_[pos_ == 0 ? 0 : pos_ - 1];
}

bool Parser::match(TokenType t) {
    if (!check(t)) return false;
    advance();
    return true;
}

bool Parser::matchAny(const std::vector<TokenType>& types) {
    for (TokenType t : types) {
        if (check(t)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType t, const std::string& message) {
    if (check(t)) return advance();
    throw makeError(peek(), message);
}

Parser::ParseError Parser::makeError(const Token& at, const std::string& message, bool showFound) const {
    std::string text = message;
    if (showFound) text += ", found " + describe(at);
    return ParseError{at, text};
}

void Parser::report(const ParseError& e) {
    errors_.push_back("Syntax error at line " + std::to_string(e.token.line) + ", col " +
                      std::to_string(e.token.column) + ": " + e.message);
}

// Panic-mode recovery: skip tokens until a likely statement boundary so that
// several syntax errors can be reported in one run.
void Parser::synchronize() {
    while (!atEnd()) {
        if (pos_ > 0 && previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
            case TokenType::KW_INT:
            case TokenType::KW_FLOAT:
            case TokenType::KW_BOOL:
            case TokenType::KW_STRING:
            case TokenType::KW_FUNC:
            case TokenType::KW_IF:
            case TokenType::KW_WHILE:
            case TokenType::KW_FOR:
            case TokenType::KW_RETURN:
            case TokenType::KW_PRINT:
            case TokenType::KW_READ:
            case TokenType::RBRACE:
                return;
            default:
                break;
        }
        advance();
    }
}
