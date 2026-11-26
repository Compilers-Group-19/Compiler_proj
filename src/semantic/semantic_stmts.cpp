// Owner: PS/CSC/23/0212  (Phase 3, Part 4)

#include "semantic.h"

void SemanticAnalyzer::checkStmt(const Stmt* s) {
    if (!s) return;
    switch (s->kind) {
        case StmtKind::VAR_DECL:
            checkVarDecl(static_cast<const VarDecl&>(*s));
            break;
        case StmtKind::BLOCK:
            checkBlock(static_cast<const BlockStmt&>(*s), true);
            break;
        case StmtKind::IF: {
            auto& x = static_cast<const IfStmt&>(*s);
            checkCondition(x.condition.get(), "if");
            checkBody(x.thenBranch.get(), "if");
            checkBody(x.elseBranch.get(), "else");
            break;
        }
        case StmtKind::WHILE: {
            auto& x = static_cast<const WhileStmt&>(*s);
            checkCondition(x.condition.get(), "while");
            checkBody(x.body.get(), "while");
            break;
        }
        case StmtKind::FOR:
            checkFor(static_cast<const ForStmt&>(*s));
            break;
        case StmtKind::RETURN:
            checkReturn(static_cast<const ReturnStmt&>(*s));
            break;
        case StmtKind::PRINT:
            checkPrint(static_cast<const PrintStmt&>(*s));
            break;
        case StmtKind::READ:
            checkRead(static_cast<const ReadStmt&>(*s));
            break;
        case StmtKind::EXPR: {
            const Expr* e = static_cast<const ExprStmt&>(*s).expr.get();
            if (e->kind == ExprKind::CALL) checkExpr(e);
            else valueOf(e);
            break;
        }
        case StmtKind::FUNC_DECL:
            break;
    }
}

void SemanticAnalyzer::checkBlock(const BlockStmt& b, bool newScope) {
    if (newScope) symbols_.enterScope();
    for (const StmtPtr& s : b.statements) checkStmt(s.get());
    if (newScope) symbols_.exitScope();
}

void SemanticAnalyzer::checkBody(const Stmt* body, const char* owner) {
    if (!body) return;
    if (body->kind == StmtKind::VAR_DECL) {
        error(*body, std::string("a declaration can not be the body of '") + owner +
                         "'; put it inside { }");
    }
    checkStmt(body);
}

void SemanticAnalyzer::checkCondition(const Expr* e, const char* owner) {
    Type t = valueOf(e);
    if (!t.isError() && !t.is(Type::Base::BOOL)) {
        error(*e, std::string("the condition of '") + owner + "' must be bool, found " + t.name());
    }
}

void SemanticAnalyzer::checkFor(const ForStmt& s) {
    symbols_.enterScope();
    checkStmt(s.init.get());
    if (s.condition) checkCondition(s.condition.get(), "for");
    if (s.update) checkExpr(s.update.get());
    checkBody(s.body.get(), "for");
    symbols_.exitScope();
}

void SemanticAnalyzer::checkReturn(const ReturnStmt& s) {
    if (!currentFunction_) {
        error(s, "'return' can only be used inside a function");
        if (s.value) checkExpr(s.value.get());
        return;
    }
    const std::string& name = currentFunction_->name;
    const Type& expected = currentFunction_->type;

    if (expected.isVoid()) {
        if (s.value) {
            checkExpr(s.value.get());
            error(s, "function '" + name + "' is void and can not return a value");
        }
        return;
    }
    if (!s.value) {
        error(s, "function '" + name + "' must return " + aType(expected) + " value");
        return;
    }
    Type value = valueOf(s.value.get());
    if (!assignable(expected, value)) {
        error(*s.value, "function '" + name + "' returns " + expected.name() +
                            ", but this value is " + value.name());
    }
}

void SemanticAnalyzer::checkPrint(const PrintStmt& s) {
    for (const ExprPtr& arg : s.args) valueOf(arg.get());
}

void SemanticAnalyzer::checkRead(const ReadStmt& s) {
    Type t = checkExpr(s.target.get());
    if (t.isArray()) {
        error(*s.target, "can not read into the whole array; read into one element, e.g. read(a[i])");
    }
}
