// Owner: PS/CSC/23/0215  (Phase 3, Part 1)

#include "symbol_table.h"

SymbolTable::SymbolTable() { scopes_.emplace_back(); }

void SymbolTable::enterScope() { scopes_.emplace_back(); }

void SymbolTable::exitScope() {
    if (scopes_.size() > 1) scopes_.pop_back();  // the global scope never closes
}

void SymbolTable::beginFunction() {
    inFunction_ = true;
    functionNames_.clear();
}

void SymbolTable::endFunction() { inFunction_ = false; }

std::string SymbolTable::uniqueName(const std::string& name) {
    auto& counts = inFunction_ ? functionNames_ : topLevelNames_;
    int n = counts[name]++;
    return n == 0 ? name : name + "." + std::to_string(n);
}

Symbol* SymbolTable::declare(const Symbol& symbol) {
    auto& scope = scopes_.back();
    if (scope.count(symbol.name)) return nullptr;

    auto owned = std::make_unique<Symbol>(symbol);
    owned->global = atGlobalScope() && symbol.kind != Symbol::Kind::FUNCTION;
    if (symbol.kind == Symbol::Kind::FUNCTION || owned->global) {
        owned->irName = symbol.name;  // globals and functions are unique already
    } else {
        owned->irName = uniqueName(symbol.name);
    }
    Symbol* p = owned.get();
    owned_.push_back(std::move(owned));
    scope[symbol.name] = p;
    return p;
}

Symbol* SymbolTable::adopt(const Symbol& symbol) {
    owned_.push_back(std::make_unique<Symbol>(symbol));
    return owned_.back().get();
}

Symbol* SymbolTable::lookup(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second;
    }
    return nullptr;
}

Symbol* SymbolTable::lookupCurrent(const std::string& name) const {
    auto found = scopes_.back().find(name);
    return found == scopes_.back().end() ? nullptr : found->second;
}
