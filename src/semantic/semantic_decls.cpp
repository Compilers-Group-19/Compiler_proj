// Owner: PS/CSC/23/0206  (Phase 3, Part 3)
// Variable declarations, function bodies, and the "every path returns" check.

#include "semantic.h"

void SemanticAnalyzer::checkVarDecl(const VarDecl& d) {
    Type declared = d.arraySize >= 0 ? Type::arrayOf(d.type, d.arraySize) : Type::of(d.type);

    // The initializer is checked before the name is declared, so
    // "int x = x;" is an error (x is not declared yet) instead of reading
    // an uninitialized value.
    if (d.init) {
        Type value = valueOf(d.init.get());
        if (!assignable(declared, value)) {
            error(*d.init, "can not initialize " + aType(declared) + " variable '" + d.name +
                               "' with " + aType(value) + " value");
        }
    }

    Symbol s;
    s.kind = Symbol::Kind::VARIABLE;
    s.name = d.name;
    s.type = declared;
    s.line = d.line;
    s.column = d.column;

    Symbol* declaredSym = symbols_.declare(s);
    if (!declaredSym) {
        const Symbol* first = symbols_.lookupCurrent(d.name);
        std::string what = first->isFunction() ? "a function" : "already declared in this scope";
        error(d, "'" + d.name + "' is " + what + " (line " + std::to_string(first->line) + ")");
        declaredSym = symbols_.adopt(s);  // keep going with the new declaration
    }
    info_.symbols[&d] = declaredSym;
}

void SemanticAnalyzer::checkFunction(const FuncDecl& f) {
    const Symbol* fn = info_.symbolOf(&f);
    const Symbol* outer = currentFunction_;
    currentFunction_ = fn;

    symbols_.beginFunction();
    symbols_.enterScope();  // parameters and the body's own declarations share this scope

    std::vector<const Symbol*>& params = info_.params[&f];
    for (const Param& p : f.params) {
        Symbol s;
        s.kind = Symbol::Kind::PARAMETER;
        s.name = p.name;
        s.type = Type::of(p.type);
        s.line = p.line;
        s.column = p.column;
        Symbol* declared = symbols_.declare(s);
        if (!declared) {
            error(p.line, p.column, "parameter '" + p.name + "' is declared twice in function '" + f.name + "'");
            declared = symbols_.adopt(s);
        }
        params.push_back(declared);
    }

    checkBlock(*f.body, /*newScope=*/false);

    if (!fn->type.isVoid() && !alwaysReturns(f.body.get())) {
        error(f, "function '" + f.name + "' must return " + aType(fn->type) +
                     " value on every path (add a return at the end)");
    }

    symbols_.exitScope();
    symbols_.endFunction();
    currentFunction_ = outer;
}

// True when running `s` can never finish normally: it always reaches a
// return (or loops forever). Used to find non-void functions that can end
// without returning a value.
bool SemanticAnalyzer::alwaysReturns(const Stmt* s) const {
    if (!s) return false;
    switch (s->kind) {
        case StmtKind::RETURN:
            return true;
        case StmtKind::BLOCK: {
            for (const StmtPtr& inner : static_cast<const BlockStmt*>(s)->statements)
                if (alwaysReturns(inner.get())) return true;
            return false;
        }
        case StmtKind::IF: {
            auto* x = static_cast<const IfStmt*>(s);
            return x->elseBranch && alwaysReturns(x->thenBranch.get()) && alwaysReturns(x->elseBranch.get());
        }
        case StmtKind::WHILE: {
            // while (true) never ends normally (the language has no 'break').
            auto* x = static_cast<const WhileStmt*>(s);
            return x->condition->kind == ExprKind::BOOL_LIT &&
                   static_cast<const BoolLiteral*>(x->condition.get())->value;
        }
        case StmtKind::FOR: {
            // for (...; ; ...) and for (...; true; ...) never end normally.
            auto* x = static_cast<const ForStmt*>(s);
            if (!x->condition) return true;
            return x->condition->kind == ExprKind::BOOL_LIT &&
                   static_cast<const BoolLiteral*>(x->condition.get())->value;
        }
        default:
            return false;
    }
}
