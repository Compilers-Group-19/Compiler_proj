// Owner: PS/CSC/23/0215  (Phase 3, Part 1)

#ifndef SEMANTIC_TYPES_H
#define SEMANTIC_TYPES_H

#include <string>
#include "../ast.h"

struct Type {
    enum class Base { INT, FLOAT, BOOL, STRING, VOID, ERROR };

    Base base = Base::ERROR;
    int arraySize = -1;

    static Type of(DataType t) {
        Type r;
        switch (t) {
            case DataType::INT: r.base = Base::INT; break;
            case DataType::FLOAT: r.base = Base::FLOAT; break;
            case DataType::BOOL: r.base = Base::BOOL; break;
            case DataType::STRING: r.base = Base::STRING; break;
            case DataType::VOID: r.base = Base::VOID; break;
        }
        return r;
    }
    static Type arrayOf(DataType t, int size) {
        Type r = of(t);
        r.arraySize = size;
        return r;
    }
    static Type error() { return Type{}; }
    static Type boolean() { return of(DataType::BOOL); }

    bool isError() const { return base == Base::ERROR; }
    bool isArray() const { return arraySize >= 0; }
    bool isVoid() const { return base == Base::VOID && !isArray(); }
    bool is(Base b) const { return base == b && !isArray(); }
    bool isNumeric() const { return !isArray() && (base == Base::INT || base == Base::FLOAT); }

    Type element() const {
        Type r = *this;
        r.arraySize = -1;
        return r;
    }

    std::string name() const {
        std::string s;
        switch (base) {
            case Base::INT: s = "int"; break;
            case Base::FLOAT: s = "float"; break;
            case Base::BOOL: s = "bool"; break;
            case Base::STRING: s = "string"; break;
            case Base::VOID: s = "void"; break;
            case Base::ERROR: s = "<error>"; break;
        }
        if (isArray()) s += "[" + std::to_string(arraySize) + "]";
        return s;
    }
};

inline bool operator==(const Type& a, const Type& b) {
    return a.base == b.base && a.arraySize == b.arraySize;
}
inline bool operator!=(const Type& a, const Type& b) { return !(a == b); }

inline bool assignable(const Type& to, const Type& from) {
    if (to.isError() || from.isError()) return true;
    if (to.isArray() || from.isArray()) return false;
    if (to.isVoid() || from.isVoid()) return false;
    if (to.base == from.base) return true;
    return to.base == Type::Base::FLOAT && from.base == Type::Base::INT;
}

#endif
