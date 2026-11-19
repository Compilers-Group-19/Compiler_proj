// parser_expressions.cpp  (Part 6)
// PS/CSC/23/0211

#include "parser.h"
#include "parser_util.h"
#include <utility>
#include <vector>

using namespace parser_util;

namespace
{

    // Precedence table, lowest to highest. Each level is left-associative.
    const std::vector<std::vector<TokenType>> &levels()
    {
        static const std::vector<std::vector<TokenType>> table = {
            {TokenType::OR},
            {TokenType::AND},
            {TokenType::EQ, TokenType::NEQ},
            {TokenType::LT, TokenType::LE, TokenType::GT, TokenType::GE},
            {TokenType::PLUS, TokenType::MINUS},
            {TokenType::STAR, TokenType::SLASH, TokenType::PERCENT},
        };
        return table;
    }

} // namespace

ExprPtr Parser::expression() { return assignment(); }

ExprPtr Parser::assignment()
{
    ExprPtr left = binary(0);
    if (check(TokenType::ASSIGN))
    {
        Token eq = advance();
        ExprPtr value = assignment(); // right-associative: a = b = 3
        if (left->kind == ExprKind::VARIABLE || left->kind == ExprKind::INDEX)
        {
            int line = left->line, col = left->column;
            auto a = std::make_unique<AssignExpr>(std::move(left), std::move(value));
            a->line = line;
            a->column = col;
            return a;
        }
        throw makeError(eq, "Invalid assignment target", false);
    }
    return left;
}

// One method handles every left-associative binary level using levels().
ExprPtr Parser::binary(size_t level)
{
    if (level >= levels().size())
        return unary();
    ExprPtr left = binary(level + 1);
    while (matchAny(levels()[level]))
    {
        Token op = previous();
        ExprPtr right = binary(level + 1);
        left = node<BinaryExpr>(op, op.type, std::move(left), std::move(right));
    }
    return left;
}
