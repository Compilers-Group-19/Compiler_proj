// PS/CSC/23/0209
// The top of the grammar: the whole program and function declarations.
//
//   program   -> ( funcDecl | statement )* EOF
//   funcDecl  -> "func" type IDENT "(" [ param { "," param } ] ")" block
//   param     -> varType IDENT
//
// parse() is also where syntax errors are caught: when a rule throws, the
// error is reported, synchronize() skips to a safe point and parsing goes on,
// so one run can report several errors.

#include "parser.h"
#include "parser_util.h"
#include <utility>

using namespace parser_util;

std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();
    program->line = 1;
    program->column = 1;
    pos_ = 0;
    errors_.clear();
    recovering_ = false;

    while (!atEnd()) {
        // After an error we may have skipped a block's '{' but not its '}'.
        // Drop that leftover brace quietly instead of reporting a second error.
        if (recovering_ && check(TokenType::RBRACE)) {
            advance();
            continue;
        }
        size_t start = pos_;
        try {
            program->items.push_back(declaration());
            recovering_ = false;
        } catch (const ParseError& e) {
            report(e);
            recovering_ = true;
            synchronize();
            if (pos_ == start && !atEnd()) advance();  // always make progress
        }
    }
    return program;
}

StmtPtr Parser::declaration() {
    if (check(TokenType::KW_FUNC)) return funcDecl();
    return statement();
}

std::unique_ptr<FuncDecl> Parser::funcDecl() {
    Token kw = advance();  // 'func'

    if (!isVarTypeToken(peek().type) && !check(TokenType::KW_VOID))
        throw makeError(peek(), "Expected return type after 'func'");
    DataType returnType = toDataType(advance().type);

    Token name = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LPAREN, "Expected '(' after function name");

    std::vector<Param> params;
    if (!check(TokenType::RPAREN)) {
        do {
            if (!isVarTypeToken(peek().type))
                throw makeError(peek(), "Expected parameter type");
            Token typeTok = advance();
            Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name");
            params.push_back(Param{toDataType(typeTok.type), paramName.lexeme, typeTok.line, typeTok.column});
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Expected ')' after parameters");

    if (!check(TokenType::LBRACE)) throw makeError(peek(), "Expected '{' before function body");
    auto body = block();
    return node<FuncDecl>(kw, returnType, name.lexeme, std::move(params), std::move(body));
}
