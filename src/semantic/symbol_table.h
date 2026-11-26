// Owner: PS/CSC/23/0215  (Phase 3, Part 1)

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
    std::string name;
    std::string irName;
    Type type;
    std::vector<Type> paramTypes;
    bool global = false;
    int line = 0;
    int column = 0;

    bool isFunction() const { return kind == Kind::FUNCTION; }
};

class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();
    bool atGlobalScope() const { return scopes_.size() == 1; }

    void beginFunction();
    void endFunction();

    Symbol* declare(const Symbol& symbol);

    Symbol* adopt(const Symbol& symbol);

    Symbol* lookup(const std::string& name) const;
    Symbol* lookupCurrent(const std::string& name) const;

private:
    std::vector<std::unordered_map<std::string, Symbol*>> scopes_;
    std::vector<std::unique_ptr<Symbol>> owned_;
    std::unordered_map<std::string, int> topLevelNames_;
    std::unordered_map<std::string, int> functionNames_;
    bool inFunction_ = false;

    std::string uniqueName(const std::string& name);
};

#endif
