// Owner: PS/CSC/23/0207  (Phase 3, Part 2)
// The entry point of semantic analysis and the error messages.
//
// The program is checked in two passes:
//   1. collectFunctions(): every function's name, return type and parameter
//      types go into the global scope. So a function can be called before
//      the line where it is written, and recursion works.
//   2. Everything is checked in source order. Global variables are visible
//      only after their declaration, also inside functions written later.

#include "semantic.h"

#include <algorithm>

void SemanticAnalyzer::error(int line, int column, const std::string& message) {
    messages_.push_back(Message{line, column,
                                "Semantic error at line " + std::to_string(line) + ", col " +
                                    std::to_string(column) + ": " + message});
}

std::string SemanticAnalyzer::aType(const Type& t) {
    std::string name = t.name();
    bool vowel = !name.empty() && std::string("aeiou").find(name[0]) != std::string::npos;
    return (vowel ? "an " : "a ") + name;
}

void SemanticAnalyzer::error(const Node& at, const std::string& message) {
    error(at.line, at.column, message);
}

bool SemanticAnalyzer::analyze(const Program& program) {
    messages_.clear();
    errors_.clear();
    collectFunctions(program);

    for (const StmtPtr& item : program.items) {
        if (item->kind == StmtKind::FUNC_DECL) {
            checkFunction(static_cast<const FuncDecl&>(*item));
        } else {
            checkStmt(item.get());
        }
    }

    // Pass 1 finds duplicate functions before anything else; report all
    // errors in the order they appear in the source.
    std::stable_sort(messages_.begin(), messages_.end(), [](const Message& a, const Message& b) {
        return a.line != b.line ? a.line < b.line : a.column < b.column;
    });
    for (const Message& m : messages_) errors_.push_back(m.text);
    return errors_.empty();
}

void SemanticAnalyzer::collectFunctions(const Program& program) {
    for (const StmtPtr& item : program.items) {
        if (item->kind != StmtKind::FUNC_DECL) continue;
        const auto& f = static_cast<const FuncDecl&>(*item);

        Symbol s;
        s.kind = Symbol::Kind::FUNCTION;
        s.name = f.name;
        s.type = Type::of(f.returnType);
        for (const Param& p : f.params) s.paramTypes.push_back(Type::of(p.type));
        s.line = f.line;
        s.column = f.column;

        Symbol* declared = symbols_.declare(s);
        if (!declared) {
            const Symbol* first = symbols_.lookupCurrent(f.name);
            error(f, "function '" + f.name + "' is already declared (first at line " +
                         std::to_string(first->line) + ")");
            declared = symbols_.adopt(s);  // still check its body
        }
        info_.symbols[&f] = declared;
    }
}
