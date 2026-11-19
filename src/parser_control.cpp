// OWNER: PS/CSC/23/0208


#include "parser.h"
#include "parser_util.h"
#include <utility>

using namespace parser_util;

StmtPtr Parser::ifStmt() {
    Token kw = advance();  // 'if'
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    ExprPtr cond = expression();
    consume(TokenType::RPAREN, "Expected ')' after if condition");
    StmtPtr thenBranch = statement();
    StmtPtr elseBranch;
    if (match(TokenType::KW_ELSE)) elseBranch = statement();
    return node<IfStmt>(kw, std::move(cond), std::move(thenBranch), std::move(elseBranch));
}

StmtPtr Parser::whileStmt() {
    Token kw = advance();  // 'while'
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    ExprPtr cond = expression();
    consume(TokenType::RPAREN, "Expected ')' after while condition");
    StmtPtr body = statement();
    return node<WhileStmt>(kw, std::move(cond), std::move(body));
}

StmtPtr Parser::forStmt() {
    Token kw = advance();  // 'for'
    consume(TokenType::LPAREN, "Expected '(' after 'for'");

    StmtPtr init;
    if (match(TokenType::SEMICOLON)) {
        // empty initializer
    } else if (isVarTypeToken(peek().type)) {
        init = varDecl();  // consumes its own ';'
    } else {
        init = exprStmt();  // consumes its own ';'
    }

    ExprPtr cond;
    if (!check(TokenType::SEMICOLON)) cond = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after for condition");

    ExprPtr update;
    if (!check(TokenType::RPAREN)) update = expression();
    consume(TokenType::RPAREN, "Expected ')' after for clauses");

    StmtPtr body = statement();
    return node<ForStmt>(kw, std::move(init), std::move(cond), std::move(update), std::move(body));
}

StmtPtr Parser::returnStmt() {
    Token kw = advance();  // 'return'
    ExprPtr value;
    if (!check(TokenType::SEMICOLON)) value = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after return statement");
    return node<ReturnStmt>(kw, std::move(value));
}

StmtPtr Parser::printStmt() {
    Token kw = advance();  // 'print'
    consume(TokenType::LPAREN, "Expected '(' after 'print'");
    std::vector<ExprPtr> args;
    do {
        args.push_back(expression());
    } while (match(TokenType::COMMA));
    consume(TokenType::RPAREN, "Expected ')' after print arguments");
    consume(TokenType::SEMICOLON, "Expected ';' after print statement");
    return node<PrintStmt>(kw, std::move(args));
}

StmtPtr Parser::readStmt() {
    Token kw = advance();  // 'read'
    consume(TokenType::LPAREN, "Expected '(' after 'read'");
    Token id = consume(TokenType::IDENTIFIER, "Expected variable name inside read()");
    ExprPtr target;
    if (match(TokenType::LBRACKET)) {
        ExprPtr index = expression();
        consume(TokenType::RBRACKET, "Expected ']' after index");
        target = node<IndexExpr>(id, id.lexeme, std::move(index));
    } else {
        target = node<VariableExpr>(id, id.lexeme);
    }
    consume(TokenType::RPAREN, "Expected ')' after read target");
    consume(TokenType::SEMICOLON, "Expected ';' after read statement");
    return node<ReadStmt>(kw, std::move(target));
}
