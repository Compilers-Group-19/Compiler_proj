#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include <ostream>
#include "ast.h"

// Pretty-prints the AST as an indented tree (for debugging and the demo).
void printAst(const Program& program, std::ostream& os);

#endif
