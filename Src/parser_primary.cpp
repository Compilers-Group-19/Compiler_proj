// Created by index PS/CSC/23/0214
// The tightest-binding parts of an expression: unary operators, function
// calls, array indexing, literals, names and parenthesised expressions.
//
//   unary    -> ( "!" | "-" ) unary | primary
//   primary  -> INT | FLOAT | STRING | "true" | "false"
//             | IDENT | IDENT "(" [ args ] ")" | IDENT "[" expression "]"
//             | "(" expression ")"
//   args     -> expression { "," expression }

#include "parser.h"
#include "parser_util.h"
#include <stdexcept>
#include <string>
#include <utility>

using namespace parser_util;

ExprPtr Parser::unary() {
    if (match(TokenType::NOT) || match(TokenType::MINUS)) {
        Token op = previous();
        ExprPtr operand = unary();
        return node<UnaryExpr>(op, op.type, std::move(operand));
    }
    return primary();
}

std::vector<ExprPtr> Parser::argumentList() {
    std::vector<ExprPtr> args;
    if (!check(TokenType::RPAREN)) {
        do {
            args.push_back(expression());
        } while (match(TokenType::COMMA));
    }
    return args;
}

ExprPtr Parser::primary() {
    const Token t = peek();
    switch (t.type) {
        case TokenType::INT_LITERAL: {
            advance();
            try {
                return node<IntLiteral>(t, std::stoll(t.lexeme));
            } catch (const std::exception&) {
                throw makeError(t, "Integer literal is out of range", false);
            }
        }
        case TokenType::FLOAT_LITERAL: {
            advance();
            try {
                return node<FloatLiteral>(t, std::stod(t.lexeme));
            } catch (const std::exception&) {
                throw makeError(t, "Float literal is out of range", false);
            }
        }
        case TokenType::STRING_LITERAL:
            advance();
            return node<StringLiteral>(t, t.lexeme);
        case TokenType::KW_TRUE:
            advance();
            return node<BoolLiteral>(t, true);
        case TokenType::KW_FALSE:
            advance();
            return node<BoolLiteral>(t, false);
        case TokenType::IDENTIFIER: {
            advance();
            if (match(TokenType::LPAREN)) {
                std::vector<ExprPtr> args = argumentList();
                consume(TokenType::RPAREN, "Expected ')' after arguments");
                return node<CallExpr>(t, t.lexeme, std::move(args));
            }
            if (match(TokenType::LBRACKET)) {
                ExprPtr index = expression();
                consume(TokenType::RBRACKET, "Expected ']' after index");
                return node<IndexExpr>(t, t.lexeme, std::move(index));
            }
            return node<VariableExpr>(t, t.lexeme);
        }
        case TokenType::LPAREN: {
            advance();
            ExprPtr inner = expression();
            consume(TokenType::RPAREN, "Expected ')' after expression");
            return inner;  // grouping needs no node of its own
        }
        default:
            throw makeError(t, "Expected expression");
    }
}
