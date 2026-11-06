#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <string>
#include <vector>
#include "ast.h"
#include "token.h"

// Phase 2: syntax analysis. Recursive-descent parser that consumes the
// token vector produced by Scanner::scanTokens() and builds an AST.
//
// Usage:
//     Scanner scanner(source);
//     Parser parser(scanner.scanTokens());
//     std::unique_ptr<Program> program = parser.parse();
//     if (parser.errorCount() > 0) { /* parser.errors() has the messages */ }
//
// Grammar (EBNF):
//   program     -> ( funcDecl | statement )* EOF
//   funcDecl    -> "func" type IDENT "(" [ param { "," param } ] ")" block
//   param       -> varType IDENT
//   type        -> "int" | "float" | "bool" | "string" | "void"
//   varType     -> "int" | "float" | "bool" | "string"
//   statement   -> varDecl | block | ifStmt | whileStmt | forStmt | returnStmt
//                | printStmt | readStmt | exprStmt
//   varDecl     -> varType IDENT [ "[" INT "]" ] [ "=" expression ] ";"
//   block       -> "{" statement* "}"
//   ifStmt      -> "if" "(" expression ")" statement [ "else" statement ]
//   whileStmt   -> "while" "(" expression ")" statement
//   forStmt     -> "for" "(" ( varDecl | exprStmt | ";" )
//                  [ expression ] ";" [ expression ] ")" statement
//   returnStmt  -> "return" [ expression ] ";"
//   printStmt   -> "print" "(" expression { "," expression } ")" ";"
//   readStmt    -> "read" "(" IDENT [ "[" expression "]" ] ")" ";"
//   exprStmt    -> expression ";"
//   expression  -> assignment
//   assignment  -> ( IDENT | IDENT "[" expression "]" ) "=" assignment | logicOr
//   logicOr     -> logicAnd { "||" logicAnd }
//   logicAnd    -> equality { "&&" equality }
//   equality    -> comparison { ( "==" | "!=" ) comparison }
//   comparison  -> term { ( "<" | "<=" | ">" | ">=" ) term }
//   term        -> factor { ( "+" | "-" ) factor }
//   factor      -> unary { ( "*" | "/" | "%" ) unary }
//   unary       -> ( "!" | "-" ) unary | primary
//   primary     -> INT | FLOAT | STRING | "true" | "false"
//                | IDENT | IDENT "(" [ args ] ")" | IDENT "[" expression "]"
//                | "(" expression ")"
class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    // Parse the whole token stream. Always returns a Program (possibly partial
    // if there were syntax errors); check errorCount() before using it.
    std::unique_ptr<Program> parse();

    int errorCount() const { return static_cast<int>(errors_.size()); }
    const std::vector<std::string>& errors() const { return errors_; }

private:
    struct ParseError {
        Token token;
        std::string message;
    };

    std::vector<Token> tokens_;
    size_t pos_ = 0;
    std::vector<std::string> errors_;
    bool recovering_ = false;  // true right after a top-level syntax error

    // token helpers
    const Token& peek() const { return tokens_[pos_]; }
    const Token& previous() const { return tokens_[pos_ - 1]; }
    bool atEnd() const { return peek().type == TokenType::END_OF_FILE; }
    bool check(TokenType t) const { return peek().type == t; }
    const Token& advance();
    bool match(TokenType t);
    bool matchAny(const std::vector<TokenType>& types);
    Token consume(TokenType t, const std::string& message);

    // errors
    ParseError makeError(const Token& at, const std::string& message, bool showFound = true) const;
    void report(const ParseError& e);
    void synchronize();

    // declarations and statements
    StmtPtr declaration();
    std::unique_ptr<FuncDecl> funcDecl();
    StmtPtr statement();
    StmtPtr varDecl();
    std::unique_ptr<BlockStmt> block();
    StmtPtr ifStmt();
    StmtPtr whileStmt();
    StmtPtr forStmt();
    StmtPtr returnStmt();
    StmtPtr printStmt();
    StmtPtr readStmt();
    StmtPtr exprStmt();

    // expressions
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr binary(size_t level);
    ExprPtr unary();
    ExprPtr primary();
    std::vector<ExprPtr> argumentList();
};

#endif
