// Owner: PS/CSC/23/XXXX  (Phase 3, Part 1)
// Types used by semantic analysis, and the rules for when one type can be
// used where another is expected.

#ifndef SEMANTIC_TYPES_H
#define SEMANTIC_TYPES_H

#include <string>
#include "../ast.h"

// The type of a variable, parameter, function result or expression.
//
// ERROR is given to an expression that already produced an error. It is
// compatible with everything, so one mistake is reported only once instead
// of causing a chain of follow-up errors.
struct Type {
    enum class Base { INT, FLOAT, BOOL, STRING, VOID, ERROR };

    Base base = Base::ERROR;
    int arraySize = -1;  // number of elements for arrays, -1 otherwise

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
    // Single value of exactly this base type (not an array).
    bool is(Base b) const { return base == b && !isArray(); }
    bool isNumeric() const { return !isArray() && (base == Base::INT || base == Base::FLOAT); }

    // The type of one element of an array (the type itself for non-arrays).
    Type element() const {
        Type r = *this;
        r.arraySize = -1;
        return r;
    }

    // "int", "float", "int[5]", ...
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

// Can a value of type `from` be stored where a `to` is expected
// (initializer, assignment, argument, return value)?
// Allowed: the same single-value type, or int -> float (widening).
// Not allowed: float -> int (it would lose the fraction), arrays, void.
inline bool assignable(const Type& to, const Type& from) {
    if (to.isError() || from.isError()) return true;
    if (to.isArray() || from.isArray()) return false;
    if (to.isVoid() || from.isVoid()) return false;
    if (to.base == from.base) return true;
    return to.base == Type::Base::FLOAT && from.base == Type::Base::INT;
}

#endif
