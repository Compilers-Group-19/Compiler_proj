// PS/CSC/23/0208


#include "parser.h"

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
