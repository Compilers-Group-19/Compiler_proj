// Owner: PS/CSC/23/0215  (Phase 3, Part 1)
// Symbols (what a name means) and the scoped symbol table.

#ifndef SEMANTIC_SYMBOL_TABLE_H
#define SEMANTIC_SYMBOL_TABLE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "types.h"

struct Symbol {
    enum class Kind { VARIABLE, PARAMETER, FUNCTION };

    Kind kind = Kind::VARIABLE;
    std::string name;    // as written in the source
    std::string irName;  // unique name used in the intermediate code (see below)
    Type type;           // variables/parameters: their type; functions: the return type
    std::vector<Type> paramTypes;  // functions only
    bool global = false;           // declared at the top level, outside any block or function
    int line = 0;
    int column = 0;

    bool isFunction() const { return kind == Kind::FUNCTION; }
};

// A stack of scopes. The global scope is always at the bottom.
//
// Unique IR names: two variables with the same name can exist in one function
// (int x; ... { int x; }). The first one keeps its name, later ones get
// "x.1", "x.2", ... A '.' can not appear in a mini identifier, so these names
// never clash with a name written by the programmer.
class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();
    bool atGlobalScope() const { return scopes_.size() == 1; }

    // Call around each function body, so IR names are numbered per function.
    void beginFunction();
    void endFunction();

    // Adds a symbol to the innermost scope and gives it its IR name.
    // Returns nullptr if the name is already declared in that scope.
    Symbol* declare(const Symbol& symbol);

    // Keeps a symbol alive without making it visible (used for a duplicate
    // function, whose body is still checked).
    Symbol* adopt(const Symbol& symbol);

    // Innermost scope outwards / innermost scope only. nullptr if not found.
    Symbol* lookup(const std::string& name) const;
    Symbol* lookupCurrent(const std::string& name) const;

private:
    std::vector<std::unordered_map<std::string, Symbol*>> scopes_;
    std::vector<std::unique_ptr<Symbol>> owned_;  // every symbol; pointers stay valid after a scope closes
    std::unordered_map<std::string, int> topLevelNames_;  // names used by top-level code
    std::unordered_map<std::string, int> functionNames_;  // names used in the current function
    bool inFunction_ = false;

    std::string uniqueName(const std::string& name);
};

#endif
