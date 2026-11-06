#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>
#include "token.h"

// Phase 2: Abstract Syntax Tree produced by the parser.
// Every node records the line/column of the token it started at, so the
// semantic phase can report errors with positions.

enum class DataType { INT, FLOAT, BOOL, STRING, VOID };

inline const char* dataTypeName(DataType t) {
    switch (t) {
        case DataType::INT: return "int";
        case DataType::FLOAT: return "float";
        case DataType::BOOL: return "bool";
        case DataType::STRING: return "string";
        case DataType::VOID: return "void";
    }
    return "?";
}

// Source text of an operator token (used by the printer and later phases).
inline const char* opSymbol(TokenType t) {
    switch (t) {
        case TokenType::PLUS: return "+";
        case TokenType::MINUS: return "-";
        case TokenType::STAR: return "*";
        case TokenType::SLASH: return "/";
        case TokenType::PERCENT: return "%";
        case TokenType::EQ: return "==";
        case TokenType::NEQ: return "!=";
        case TokenType::LT: return "<";
        case TokenType::LE: return "<=";
        case TokenType::GT: return ">";
        case TokenType::GE: return ">=";
        case TokenType::AND: return "&&";
        case TokenType::OR: return "||";
        case TokenType::NOT: return "!";
        default: return "?";
    }
}

struct Node {
    int line = 0;
    int column = 0;
    virtual ~Node() = default;
};

// ---------------------------------------------------------------- expressions

enum class ExprKind {
    INT_LIT, FLOAT_LIT, STRING_LIT, BOOL_LIT,
    VARIABLE, INDEX, UNARY, BINARY, ASSIGN, CALL
};

struct Expr : Node {
    ExprKind kind;
    explicit Expr(ExprKind k) : kind(k) {}
};
using ExprPtr = std::unique_ptr<Expr>;

struct IntLiteral : Expr {
    long long value;
    explicit IntLiteral(long long v) : Expr(ExprKind::INT_LIT), value(v) {}
};

struct FloatLiteral : Expr {
    double value;
    explicit FloatLiteral(double v) : Expr(ExprKind::FLOAT_LIT), value(v) {}
};

struct StringLiteral : Expr {
    std::string value;  // escapes already resolved by the scanner
    explicit StringLiteral(std::string v) : Expr(ExprKind::STRING_LIT), value(std::move(v)) {}
};

struct BoolLiteral : Expr {
    bool value;
    explicit BoolLiteral(bool v) : Expr(ExprKind::BOOL_LIT), value(v) {}
};

struct VariableExpr : Expr {
    std::string name;
    explicit VariableExpr(std::string n) : Expr(ExprKind::VARIABLE), name(std::move(n)) {}
};

// a[i]
struct IndexExpr : Expr {
    std::string name;
    ExprPtr index;
    IndexExpr(std::string n, ExprPtr i)
        : Expr(ExprKind::INDEX), name(std::move(n)), index(std::move(i)) {}
};

// -x   !x
struct UnaryExpr : Expr {
    TokenType op;
    ExprPtr operand;
    UnaryExpr(TokenType o, ExprPtr e) : Expr(ExprKind::UNARY), op(o), operand(std::move(e)) {}
};

// a + b, a && b, a <= b, ...
struct BinaryExpr : Expr {
    TokenType op;
    ExprPtr left, right;
    BinaryExpr(TokenType o, ExprPtr l, ExprPtr r)
        : Expr(ExprKind::BINARY), op(o), left(std::move(l)), right(std::move(r)) {}
};

// target = value   (target is a VariableExpr or IndexExpr)
struct AssignExpr : Expr {
    ExprPtr target, value;
    AssignExpr(ExprPtr t, ExprPtr v)
        : Expr(ExprKind::ASSIGN), target(std::move(t)), value(std::move(v)) {}
};

// f(a, b)
struct CallExpr : Expr {
    std::string callee;
    std::vector<ExprPtr> args;
    CallExpr(std::string c, std::vector<ExprPtr> a)
        : Expr(ExprKind::CALL), callee(std::move(c)), args(std::move(a)) {}
};

// ------------------------------------------------------------------ statements

enum class StmtKind {
    VAR_DECL, BLOCK, IF, WHILE, FOR, RETURN, PRINT, READ, EXPR, FUNC_DECL
};

struct Stmt : Node {
    StmtKind kind;
    explicit Stmt(StmtKind k) : kind(k) {}
};
using StmtPtr = std::unique_ptr<Stmt>;

// int x;   int x = 5;   int a[10];
struct VarDecl : Stmt {
    DataType type;
    std::string name;
    int arraySize;   // -1 when not an array
    ExprPtr init;    // may be null
    VarDecl(DataType t, std::string n, int size, ExprPtr i)
        : Stmt(StmtKind::VAR_DECL), type(t), name(std::move(n)), arraySize(size), init(std::move(i)) {}
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
    BlockStmt() : Stmt(StmtKind::BLOCK) {}
};

struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;  // may be null
    IfStmt(ExprPtr c, StmtPtr t, StmtPtr e)
        : Stmt(StmtKind::IF), condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
};

struct WhileStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;
    WhileStmt(ExprPtr c, StmtPtr b)
        : Stmt(StmtKind::WHILE), condition(std::move(c)), body(std::move(b)) {}
};

// for (init; condition; update) body   -- init/condition/update may each be null
struct ForStmt : Stmt {
    StmtPtr init;      // VarDecl or ExprStmt
    ExprPtr condition;
    ExprPtr update;
    StmtPtr body;
    ForStmt(StmtPtr i, ExprPtr c, ExprPtr u, StmtPtr b)
        : Stmt(StmtKind::FOR), init(std::move(i)), condition(std::move(c)),
          update(std::move(u)), body(std::move(b)) {}
};

struct ReturnStmt : Stmt {
    ExprPtr value;  // may be null
    explicit ReturnStmt(ExprPtr v) : Stmt(StmtKind::RETURN), value(std::move(v)) {}
};

// print(a, b, ...);
struct PrintStmt : Stmt {
    std::vector<ExprPtr> args;
    explicit PrintStmt(std::vector<ExprPtr> a) : Stmt(StmtKind::PRINT), args(std::move(a)) {}
};

// read(x);   read(a[i]);
struct ReadStmt : Stmt {
    ExprPtr target;  // VariableExpr or IndexExpr
    explicit ReadStmt(ExprPtr t) : Stmt(StmtKind::READ), target(std::move(t)) {}
};

struct ExprStmt : Stmt {
    ExprPtr expr;
    explicit ExprStmt(ExprPtr e) : Stmt(StmtKind::EXPR), expr(std::move(e)) {}
};

struct Param {
    DataType type;
    std::string name;
    int line;
    int column;
};

struct FuncDecl : Stmt {
    DataType returnType;
    std::string name;
    std::vector<Param> params;
    std::unique_ptr<BlockStmt> body;
    FuncDecl(DataType r, std::string n, std::vector<Param> p, std::unique_ptr<BlockStmt> b)
        : Stmt(StmtKind::FUNC_DECL), returnType(r), name(std::move(n)),
          params(std::move(p)), body(std::move(b)) {}
};

// Whole program: functions and top-level statements in source order.
struct Program : Node {
    std::vector<StmtPtr> items;
};

#endif
