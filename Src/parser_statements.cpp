// PS/CSC/23/0210
// The statement dispatcher plus the simple statements: variable declarations,
// blocks and expression statements.
//
//   statement  -> varDecl | block | ifStmt | whileStmt | forStmt | returnStmt
//               | printStmt | readStmt | exprStmt
//   varDecl    -> varType IDENT [ "[" INT "]" ] [ "=" expression ] ";"
//   block      -> "{" statement* "}"
//   exprStmt   -> expression ";"
//
// ifStmt, whileStmt, forStmt, returnStmt, printStmt and readStmt are in
// parser_control.cpp.

#include "parser.h"
#include "parser_util.h"
#include <stdexcept>
#include <string>
#include <utility>

using namespace parser_util;

StmtPtr Parser::statement() {
    switch (peek().type) {
        case TokenType::KW_INT:
        case TokenType::KW_FLOAT:
        case TokenType::KW_BOOL:
        case TokenType::KW_STRING:
            return varDecl();
        case TokenType::KW_VOID:
            throw makeError(peek(), "'void' can only be used as a function return type", false);
        case TokenType::KW_FUNC:
            throw makeError(peek(), "Functions can only be declared at the top level", false);
        case TokenType::LBRACE: return block();
        case TokenType::KW_IF: return ifStmt();
        case TokenType::KW_WHILE: return whileStmt();
        case TokenType::KW_FOR: return forStmt();
        case TokenType::KW_RETURN: return returnStmt();
        case TokenType::KW_PRINT: return printStmt();
        case TokenType::KW_READ: return readStmt();
        default: return exprStmt();
    }
}

StmtPtr Parser::varDecl() {
    Token typeTok = advance();
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name after type");

    int arraySize = -1;
    if (match(TokenType::LBRACKET)) {
        Token sizeTok = consume(TokenType::INT_LITERAL, "Expected integer array size");
        try {
            arraySize = std::stoi(sizeTok.lexeme);
        } catch (const std::exception&) {
            throw makeError(sizeTok, "Array size is out of range", false);
        }
        if (arraySize <= 0) throw makeError(sizeTok, "Array size must be greater than 0", false);
        consume(TokenType::RBRACKET, "Expected ']' after array size");
    }

    ExprPtr init;
    if (match(TokenType::ASSIGN)) {
        if (arraySize >= 0)
            throw makeError(previous(), "Array declarations cannot have an initializer", false);
        init = expression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return node<VarDecl>(typeTok, toDataType(typeTok.type), name.lexeme, arraySize, std::move(init));
}

std::unique_ptr<BlockStmt> Parser::block() {
    Token open = consume(TokenType::LBRACE, "Expected '{'");
    auto blk = node<BlockStmt>(open);

    while (!check(TokenType::RBRACE) && !atEnd()) {
        size_t start = pos_;
        try {
            blk->statements.push_back(statement());
        } catch (const ParseError& e) {
            report(e);
            synchronize();
            if (pos_ == start && !atEnd() && !check(TokenType::RBRACE)) advance();
        }
    }
    consume(TokenType::RBRACE, "Expected '}' to close block");
    return blk;
}

StmtPtr Parser::exprStmt() {
    Token start = peek();
    ExprPtr e = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return node<ExprStmt>(start, std::move(e));
}
