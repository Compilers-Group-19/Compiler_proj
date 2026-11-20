// Owner: PS/CSC/23/0207 (Phase 3, Part 2)
// Phase 3a: semantic analysis.
//
// Walks the AST from the parser and checks the rules the grammar can not
// express: every name is declared before use and not declared twice in one
// scope, types match, functions are called with the right arguments, every
// non-void function returns a value, and so on. The full list of rules is in
// docs/SEMANTIC.md.
//
// Usage:
//     SemanticAnalyzer analyzer;
//     if (!analyzer.analyze(*program)) { /* analyzer.errors() has the messages */ }
//     IrGenerator gen(analyzer.info());   // Phase 3b uses what was learned here
//
// The work is split over several files:
//     semantic_core.cpp   analyze(), function signatures, error reporting
//     semantic_decls.cpp  variable and function declarations, "missing return"
//     semantic_stmts.cpp  statements
//     semantic_exprs.cpp  expressions and their types

#ifndef SEMANTIC_SEMANTIC_H
#define SEMANTIC_SEMANTIC_H

#include <string>
#include <unordered_map>
#include <vector>
#include "../ast.h"
#include "symbol_table.h"
#include "types.h"

// What the analysis found out, for the IR generator.
struct SemanticInfo {
    // The type of every checked expression.
    std::unordered_map<const Expr*, Type> exprTypes;
    // The symbol each name refers to. Keys: VariableExpr, IndexExpr, CallExpr
    // (the function), VarDecl (the new variable), FuncDecl (the function).
    std::unordered_map<const Node*, const Symbol*> symbols;
    // The parameter symbols of each function, in order.
    std::unordered_map<const FuncDecl*, std::vector<const Symbol*>> params;

    Type typeOf(const Expr* e) const {
        auto it = exprTypes.find(e);
        return it == exprTypes.end() ? Type::error() : it->second;
    }
    const Symbol* symbolOf(const Node* n) const {
        auto it = symbols.find(n);
        return it == symbols.end() ? nullptr : it->second;
    }
};

class SemanticAnalyzer {
public:
    // Checks the whole program. Returns true when there are no errors.
    bool analyze(const Program& program);

    int errorCount() const { return static_cast<int>(errors_.size()); }
    const std::vector<std::string>& errors() const { return errors_; }
    const SemanticInfo& info() const { return info_; }

private:
    SymbolTable symbols_;
    SemanticInfo info_;
    struct Message {
        int line, column;
        std::string text;
    };
    std::vector<Message> messages_;    // collected in any order
    std::vector<std::string> errors_;  // sorted by position at the end of analyze()
    const Symbol* currentFunction_ = nullptr;  // null in top-level code

    // ---- semantic_core.cpp (Part 2)
    void error(int line, int column, const std::string& message);
    void error(const Node& at, const std::string& message);
    void collectFunctions(const Program& program);
    static std::string aType(const Type& t);  // "an int", "a float", ...

    // ---- semantic_decls.cpp (Part 3)
    void checkFunction(const FuncDecl& f);
    void checkVarDecl(const VarDecl& d);
    bool alwaysReturns(const Stmt* s) const;

    // ---- semantic_stmts.cpp (Part 4)
    void checkStmt(const Stmt* s);
    void checkBlock(const BlockStmt& b, bool newScope);
    void checkBody(const Stmt* body, const char* owner);
    void checkCondition(const Expr* e, const char* owner);
    void checkFor(const ForStmt& s);
    void checkReturn(const ReturnStmt& s);
    void checkPrint(const PrintStmt& s);
    void checkRead(const ReadStmt& s);

    // ---- semantic_exprs.cpp (Part 5)
    Type checkExpr(const Expr* e);  // records the type in info_ and returns it
    Type valueOf(const Expr* e);    // checkExpr + the result must be a single value
    Type checkVariable(const VariableExpr& e);
    Type checkIndex(const IndexExpr& e);
    Type checkUnary(const UnaryExpr& e);
    Type checkBinary(const BinaryExpr& e);
    Type checkAssign(const AssignExpr& e);
    Type checkCall(const CallExpr& e);
};

#endif
