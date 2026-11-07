// parser_stubs.cpp  -- TEMPORARY FILE (created in Part 4, shrinks in Parts 5 to 9)
//
// The parser is written by several people, one part after another. Until every
// grammar rule exists, this file supplies placeholder versions of the rules
// that are still missing, so the whole project compiles and runs after EVERY
// upload. A placeholder simply reports "not written yet".
//
// Each part's folder contains a new copy of this file with that part's
// placeholders removed: copy it over the old one, do not edit it by hand.
// After Part 9 nothing is left in it and it can be deleted.
// created by index PS/CSC/23/0214

#include "parser.h"

// ---- replaced in Part 6 (src/parser_expressions.cpp)
ExprPtr Parser::expression() {
    throw makeError(peek(), "expression() is not written yet (Part 6)", false);
}

// ---- replaced in Part 6 (src/parser_expressions.cpp)
ExprPtr Parser::assignment() {
    throw makeError(peek(), "assignment() is not written yet (Part 6)", false);
}

// ---- replaced in Part 6 (src/parser_expressions.cpp)
ExprPtr Parser::binary(size_t) {
    throw makeError(peek(), "binary() is not written yet (Part 6)", false);
}

// ---- replaced in Part 7 (src/parser_statements.cpp)
StmtPtr Parser::statement() {
    throw makeError(peek(), "statement() is not written yet (Part 7)", false);
}

// ---- replaced in Part 7 (src/parser_statements.cpp)
StmtPtr Parser::varDecl() {
    throw makeError(peek(), "varDecl() is not written yet (Part 7)", false);
}

// ---- replaced in Part 7 (src/parser_statements.cpp)
std::unique_ptr<BlockStmt> Parser::block() {
    throw makeError(peek(), "block() is not written yet (Part 7)", false);
}

// ---- replaced in Part 7 (src/parser_statements.cpp)
StmtPtr Parser::exprStmt() {
    throw makeError(peek(), "exprStmt() is not written yet (Part 7)", false);
}

// ---- replaced in Part 8 (src/parser_control.cpp)
StmtPtr Parser::ifStmt() {
    throw makeError(peek(), "ifStmt() is not written yet (Part 8)", false);
}

// ---- replaced in Part 8 (src/parser_control.cpp)
StmtPtr Parser::whileStmt() {
    throw makeError(peek(), "whileStmt() is not written yet (Part 8)", false);
}

// ---- replaced in Part 8 (src/parser_control.cpp)
StmtPtr Parser::forStmt() {
    throw makeError(peek(), "forStmt() is not written yet (Part 8)", false);
}

// ---- replaced in Part 8 (src/parser_control.cpp)
StmtPtr Parser::returnStmt() {
    throw makeError(peek(), "returnStmt() is not written yet (Part 8)", false);
}

// ---- replaced in Part 8 (src/parser_control.cpp)
StmtPtr Parser::printStmt() {
    throw makeError(peek(), "printStmt() is not written yet (Part 8)", false);
}

// ---- replaced in Part 8 (src/parser_control.cpp)
StmtPtr Parser::readStmt() {
    throw makeError(peek(), "readStmt() is not written yet (Part 8)", false);
}

// ---- replaced in Part 9 (src/parser_program.cpp)
std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();
    program->line = 1;
    program->column = 1;
    errors_.push_back("Parser is not finished yet: Parser::parse() is still a placeholder in src/parser_stubs.cpp (Part 9 replaces it)");
    return program;
}

// ---- replaced in Part 9 (src/parser_program.cpp)
StmtPtr Parser::declaration() {
    throw makeError(peek(), "declaration() is not written yet (Part 9)", false);
}

// ---- replaced in Part 9 (src/parser_program.cpp)
std::unique_ptr<FuncDecl> Parser::funcDecl() {
    throw makeError(peek(), "funcDecl() is not written yet (Part 9)", false);
}
