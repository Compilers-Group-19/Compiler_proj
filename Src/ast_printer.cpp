PS/CSC/23/0206
#include "ast_printer.h"
#include <sstream>

namespace {

void emit(std::ostream& os, int depth, const std::string& text) {
    os << std::string(static_cast<size_t>(depth) * 2, ' ') << text << "\n";
}

std::string escapeForDisplay(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r': out += "\\r"; break;
            case '\0': out += "\\0"; break;
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            default: out += c;
        }
    }
    return out;
}

void printStmt(const Stmt* s, int depth, std::ostream& os, const std::string& label = "");

void printExpr(const Expr* e, int depth, std::ostream& os, const std::string& label = "") {
    if (!e) return;
    switch (e->kind) {
        case ExprKind::INT_LIT:
            emit(os, depth, label + "Int " + std::to_string(static_cast<const IntLiteral*>(e)->value));
            break;
        case ExprKind::FLOAT_LIT: {
            std::ostringstream ss;
            ss << static_cast<const FloatLiteral*>(e)->value;
            std::string text = ss.str();
            if (text.find_first_of(".eEn") == std::string::npos) text += ".0";  // keep floats looking like floats
            emit(os, depth, label + "Float " + text);
            break;
        }
        case ExprKind::STRING_LIT:
            emit(os, depth, label + "String \"" + escapeForDisplay(static_cast<const StringLiteral*>(e)->value) + "\"");
            break;
        case ExprKind::BOOL_LIT:
            emit(os, depth, label + "Bool " + (static_cast<const BoolLiteral*>(e)->value ? "true" : "false"));
            break;
        case ExprKind::VARIABLE:
            emit(os, depth, label + "Var " + static_cast<const VariableExpr*>(e)->name);
            break;
        case ExprKind::INDEX: {
            auto* x = static_cast<const IndexExpr*>(e);
            emit(os, depth, label + "Index " + x->name);
            printExpr(x->index.get(), depth + 1, os, "index: ");
            break;
        }
        case ExprKind::UNARY: {
            auto* x = static_cast<const UnaryExpr*>(e);
            emit(os, depth, label + "Unary " + opSymbol(x->op));
            printExpr(x->operand.get(), depth + 1, os);
            break;
        }
        case ExprKind::BINARY: {
            auto* x = static_cast<const BinaryExpr*>(e);
            emit(os, depth, label + "Binary " + opSymbol(x->op));
            printExpr(x->left.get(), depth + 1, os);
            printExpr(x->right.get(), depth + 1, os);
            break;
        }
        case ExprKind::ASSIGN: {
            auto* x = static_cast<const AssignExpr*>(e);
            emit(os, depth, label + "Assign");
            printExpr(x->target.get(), depth + 1, os, "target: ");
            printExpr(x->value.get(), depth + 1, os, "value: ");
            break;
        }
        case ExprKind::CALL: {
            auto* x = static_cast<const CallExpr*>(e);
            emit(os, depth, label + "Call " + x->callee);
            for (const auto& a : x->args) printExpr(a.get(), depth + 1, os, "arg: ");
            break;
        }
    }
}

void printStmt(const Stmt* s, int depth, std::ostream& os, const std::string& label) {
    if (!s) return;
    switch (s->kind) {
        case StmtKind::VAR_DECL: {
            auto* x = static_cast<const VarDecl*>(s);
            std::string text = label + "VarDecl " + dataTypeName(x->type) + " " + x->name;
            if (x->arraySize >= 0) text += "[" + std::to_string(x->arraySize) + "]";
            emit(os, depth, text);
            printExpr(x->init.get(), depth + 1, os, "init: ");
            break;
        }
        case StmtKind::BLOCK: {
            auto* x = static_cast<const BlockStmt*>(s);
            emit(os, depth, label + "Block");
            for (const auto& st : x->statements) printStmt(st.get(), depth + 1, os);
            break;
        }
        case StmtKind::IF: {
            auto* x = static_cast<const IfStmt*>(s);
            emit(os, depth, label + "If");
            printExpr(x->condition.get(), depth + 1, os, "condition: ");
            printStmt(x->thenBranch.get(), depth + 1, os, "then: ");
            printStmt(x->elseBranch.get(), depth + 1, os, "else: ");
            break;
        }
        case StmtKind::WHILE: {
            auto* x = static_cast<const WhileStmt*>(s);
            emit(os, depth, label + "While");
            printExpr(x->condition.get(), depth + 1, os, "condition: ");
            printStmt(x->body.get(), depth + 1, os, "body: ");
            break;
        }
        case StmtKind::FOR: {
            auto* x = static_cast<const ForStmt*>(s);
            emit(os, depth, label + "For");
            printStmt(x->init.get(), depth + 1, os, "init: ");
            printExpr(x->condition.get(), depth + 1, os, "condition: ");
            printExpr(x->update.get(), depth + 1, os, "update: ");
            printStmt(x->body.get(), depth + 1, os, "body: ");
            break;
        }
        case StmtKind::RETURN: {
            auto* x = static_cast<const ReturnStmt*>(s);
            emit(os, depth, label + "Return");
            printExpr(x->value.get(), depth + 1, os);
            break;
        }
        case StmtKind::PRINT: {
            auto* x = static_cast<const PrintStmt*>(s);
            emit(os, depth, label + "Print");
            for (const auto& a : x->args) printExpr(a.get(), depth + 1, os);
            break;
        }
        case StmtKind::READ: {
            auto* x = static_cast<const ReadStmt*>(s);
            emit(os, depth, label + "Read");
            printExpr(x->target.get(), depth + 1, os);
            break;
        }
        case StmtKind::EXPR: {
            auto* x = static_cast<const ExprStmt*>(s);
            emit(os, depth, label + "ExprStmt");
            printExpr(x->expr.get(), depth + 1, os);
            break;
        }
        case StmtKind::FUNC_DECL: {
            auto* x = static_cast<const FuncDecl*>(s);
            emit(os, depth, label + "FuncDecl " + x->name + " -> " + dataTypeName(x->returnType));
            for (const auto& p : x->params)
                emit(os, depth + 1, std::string("Param ") + dataTypeName(p.type) + " " + p.name);
            printStmt(x->body.get(), depth + 1, os, "body: ");
            break;
        }
    }
}

}  // namespace

void printAst(const Program& program, std::ostream& os) {
    emit(os, 0, "Program");
    for (const auto& item : program.items) printStmt(item.get(), 1, os);
}
