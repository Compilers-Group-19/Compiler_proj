// Owner: PS/CSC/23/0214 

#include "semantic.h"

namespace {

std::string opText(TokenType t) { return std::string("'") + opSymbol(t) + "'"; }

bool isZeroLiteral(const Expr* e) {
    if (e->kind == ExprKind::INT_LIT) return static_cast<const IntLiteral*>(e)->value == 0;
    if (e->kind == ExprKind::FLOAT_LIT) return static_cast<const FloatLiteral*>(e)->value == 0.0;
    return false;
}

}

Type SemanticAnalyzer::checkExpr(const Expr* e) {
    Type t;
    switch (e->kind) {
        case ExprKind::INT_LIT: t = Type::of(DataType::INT); break;
        case ExprKind::FLOAT_LIT: t = Type::of(DataType::FLOAT); break;
        case ExprKind::STRING_LIT: t = Type::of(DataType::STRING); break;
        case ExprKind::BOOL_LIT: t = Type::boolean(); break;
        case ExprKind::VARIABLE: t = checkVariable(static_cast<const VariableExpr&>(*e)); break;
        case ExprKind::INDEX: t = checkIndex(static_cast<const IndexExpr&>(*e)); break;
        case ExprKind::UNARY: t = checkUnary(static_cast<const UnaryExpr&>(*e)); break;
        case ExprKind::BINARY: t = checkBinary(static_cast<const BinaryExpr&>(*e)); break;
        case ExprKind::ASSIGN: t = checkAssign(static_cast<const AssignExpr&>(*e)); break;
        case ExprKind::CALL: t = checkCall(static_cast<const CallExpr&>(*e)); break;
    }
    info_.exprTypes[e] = t;
    return t;
}

Type SemanticAnalyzer::valueOf(const Expr* e) {
    Type t = checkExpr(e);
    if (t.isArray()) {
        std::string name = e->kind == ExprKind::VARIABLE ? static_cast<const VariableExpr*>(e)->name : "?";
        error(*e, "array '" + name + "' can not be used as a value; use one element, e.g. " + name + "[0]");
        return Type::error();
    }
    if (t.isVoid()) {
        std::string name = e->kind == ExprKind::CALL ? static_cast<const CallExpr*>(e)->callee : "?";
        error(*e, "function '" + name + "' is void and has no value to use");
        return Type::error();
    }
    return t;
}

Type SemanticAnalyzer::checkVariable(const VariableExpr& e) {
    const Symbol* s = symbols_.lookup(e.name);
    if (!s) {
        error(e, "'" + e.name + "' is not declared");
        return Type::error();
    }
    if (s->isFunction()) {
        error(e, "'" + e.name + "' is a function; call it with " + e.name + "(...)");
        return Type::error();
    }
    info_.symbols[&e] = s;
    return s->type;
}

Type SemanticAnalyzer::checkIndex(const IndexExpr& e) {
    const Symbol* s = symbols_.lookup(e.name);
    Type index = valueOf(e.index.get());
    if (!index.isError() && !index.is(Type::Base::INT)) {
        error(*e.index, "an array index must be int, found " + index.name());
    }

    if (!s) {
        error(e, "'" + e.name + "' is not declared");
        return Type::error();
    }
    if (s->isFunction() || !s->type.isArray()) {
        error(e, "'" + e.name + "' is not an array, so it can not be indexed");
        return Type::error();
    }
    info_.symbols[&e] = s;

    if (e.index->kind == ExprKind::INT_LIT) {
        long long i = static_cast<const IntLiteral*>(e.index.get())->value;
        if (i < 0 || i >= s->type.arraySize) {
            error(*e.index, "index " + std::to_string(i) + " is out of range for '" + e.name + "' (valid: 0 to " +
                                std::to_string(s->type.arraySize - 1) + ")");
        }
    }
    return s->type.element();
}

Type SemanticAnalyzer::checkUnary(const UnaryExpr& e) {
    Type t = valueOf(e.operand.get());
    if (t.isError()) return e.op == TokenType::NOT ? Type::boolean() : Type::error();

    if (e.op == TokenType::NOT) {
        if (!t.is(Type::Base::BOOL)) error(e, "operator '!' needs a bool, found " + t.name());
        return Type::boolean();
    }
    if (!t.isNumeric()) {
        error(e, "operator '-' needs an int or float, found " + t.name());
        return Type::error();
    }
    return t;
}

Type SemanticAnalyzer::checkBinary(const BinaryExpr& e) {
    Type l = valueOf(e.left.get());
    Type r = valueOf(e.right.get());
    auto mismatch = [&](const char* need) {
        error(e, "operator " + opText(e.op) + " needs " + need + ", found " + l.name() + " and " + r.name());
    };

    switch (e.op) {
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::PERCENT: {
            if (e.op == TokenType::SLASH || e.op == TokenType::PERCENT) {
                if (isZeroLiteral(e.right.get())) error(*e.right, "division by zero");
            }
            if (l.isError() || r.isError()) return Type::error();
            if (e.op == TokenType::PERCENT) {
                if (!l.is(Type::Base::INT) || !r.is(Type::Base::INT)) {
                    mismatch("two ints");
                    return Type::error();
                }
                return l;
            }
            if (!l.isNumeric() || !r.isNumeric()) {
                mismatch("two numbers (int or float)");
                return Type::error();
            }
            return (l.is(Type::Base::FLOAT) || r.is(Type::Base::FLOAT)) ? Type::of(DataType::FLOAT)
                                                                        : Type::of(DataType::INT);
        }

        case TokenType::LT:
        case TokenType::LE:
        case TokenType::GT:
        case TokenType::GE:
            if (!l.isError() && !r.isError() && (!l.isNumeric() || !r.isNumeric()))
                mismatch("two numbers (int or float)");
            return Type::boolean();

        case TokenType::EQ:
        case TokenType::NEQ:
            if (!l.isError() && !r.isError() && !(l.isNumeric() && r.isNumeric()) && l != r)
                mismatch("two values of the same kind");
            return Type::boolean();

        case TokenType::AND:
        case TokenType::OR:
            if (!l.isError() && !r.isError() && (!l.is(Type::Base::BOOL) || !r.is(Type::Base::BOOL)))
                mismatch("two bools");
            return Type::boolean();

        default:
            error(e, "unknown operator " + opText(e.op));
            return Type::error();
    }
}

Type SemanticAnalyzer::checkAssign(const AssignExpr& e) {
    Type target = checkExpr(e.target.get());
    Type value = valueOf(e.value.get());

    if (target.isArray()) {
        auto* v = static_cast<const VariableExpr*>(e.target.get());
        error(e, "can not assign to the whole array '" + v->name + "'; assign one element, e.g. " + v->name + "[0] = ...");
        return Type::error();
    }
    if (!assignable(target, value)) {
        error(e, "can not assign " + aType(value) + " value to " + aType(target) + " variable");
    }
    return target;
}

Type SemanticAnalyzer::checkCall(const CallExpr& e) {
    const Symbol* fn = symbols_.lookup(e.callee);

    if (!fn) {
        error(e, "function '" + e.callee + "' is not declared");
    } else if (!fn->isFunction()) {
        error(e, "'" + e.callee + "' is a variable, not a function");
        fn = nullptr;
    } else if (fn->paramTypes.size() != e.args.size()) {
        error(e, "function '" + e.callee + "' takes " + std::to_string(fn->paramTypes.size()) +
                     " argument(s) but " + std::to_string(e.args.size()) + " were given");
    }

    for (size_t i = 0; i < e.args.size(); i++) {
        Type arg = valueOf(e.args[i].get());
        if (fn && i < fn->paramTypes.size() && !assignable(fn->paramTypes[i], arg)) {
            error(*e.args[i], "argument " + std::to_string(i + 1) + " of '" + e.callee + "' must be " +
                                  fn->paramTypes[i].name() + ", found " + arg.name());
        }
    }

    if (!fn) return Type::error();
    info_.symbols[&e] = fn;
    return fn->type;
}
