#ifndef PARSER_UTIL_H
#define PARSER_UTIL_H

// Small helpers shared by the parser_*.cpp files.
// (In the original single-file parser these lived in an anonymous namespace.)

#include <memory>
#include <utility>
#include "ast.h"
#include "token.h"

namespace parser_util {

// Create an AST node and stamp it with the position of a token.
template <class T, class... A>
std::unique_ptr<T> node(const Token& at, A&&... args) {
    auto p = std::make_unique<T>(std::forward<A>(args)...);
    p->line = at.line;
    p->column = at.column;
    return p;
}

inline bool isVarTypeToken(TokenType t) {
    return t == TokenType::KW_INT || t == TokenType::KW_FLOAT ||
           t == TokenType::KW_BOOL || t == TokenType::KW_STRING;
}

inline DataType toDataType(TokenType t) {
    switch (t) {
        case TokenType::KW_INT: return DataType::INT;
        case TokenType::KW_FLOAT: return DataType::FLOAT;
        case TokenType::KW_BOOL: return DataType::BOOL;
        case TokenType::KW_STRING: return DataType::STRING;
        default: return DataType::VOID;
    }
}

}  // namespace parser_util

#endif
