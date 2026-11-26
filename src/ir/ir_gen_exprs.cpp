// Owner: PS/CSC/23/0208  (Phase 3, Part 8)

#include "ir_gen.h"

Operand IrGenerator::genExpr(const Expr* e) {
    switch (e->kind) {
        case ExprKind::INT_LIT: return Operand::intConst(static_cast<const IntLiteral*>(e)->value);
        case ExprKind::FLOAT_LIT: return Operand::floatConst(static_cast<const FloatLiteral*>(e)->value);
        case ExprKind::STRING_LIT: return Operand::stringConst(static_cast<const StringLiteral*>(e)->value);
        case ExprKind::BOOL_LIT: return Operand::boolConst(static_cast<const BoolLiteral*>(e)->value);
        case ExprKind::VARIABLE: return varOperand(info_.symbolOf(e));
        case ExprKind::INDEX: return genIndex(static_cast<const IndexExpr&>(*e));
        case ExprKind::UNARY: return genUnary(static_cast<const UnaryExpr&>(*e));
        case ExprKind::BINARY: {
            auto& b = static_cast<const BinaryExpr&>(*e);
            if (b.op == TokenType::AND || b.op == TokenType::OR) return genLogical(b);
            return genBinary(b);
        }
        case ExprKind::ASSIGN: return genAssign(static_cast<const AssignExpr&>(*e));
        case ExprKind::CALL: return genCall(static_cast<const CallExpr&>(*e));
    }
    return Operand::none();
}

Operand IrGenerator::genExprAs(const Expr* e, IrType wanted) { return convert(genExpr(e), wanted); }

Operand IrGenerator::convert(const Operand& o, IrType to) {
    if (to != IrType::FLOAT || o.type != IrType::INT) return o;
    if (o.kind == Operand::Kind::INT) return Operand::floatConst(static_cast<double>(o.intValue));
    Operand t = newTemp(IrType::FLOAT);
    Instr in;
    in.op = Op::INT_TO_FLOAT;
    in.dst = t;
    in.a = o;
    emit(in);
    return t;
}

Operand IrGenerator::stable(const Operand& o, const Expr* later) {
    if (o.kind != Operand::Kind::VAR) return o;
    bool changes = contains(later, ExprKind::ASSIGN) || (o.global && contains(later, ExprKind::CALL));
    if (!changes) return o;
    Operand t = newTemp(o.type);
    Instr in;
    in.op = Op::COPY;
    in.dst = t;
    in.a = o;
    emit(in);
    return t;
}

bool IrGenerator::contains(const Expr* e, ExprKind kind) {
    if (!e) return false;
    if (e->kind == kind) return true;
    switch (e->kind) {
        case ExprKind::INDEX: return contains(static_cast<const IndexExpr*>(e)->index.get(), kind);
        case ExprKind::UNARY: return contains(static_cast<const UnaryExpr*>(e)->operand.get(), kind);
        case ExprKind::BINARY: {
            auto* b = static_cast<const BinaryExpr*>(e);
            return contains(b->left.get(), kind) || contains(b->right.get(), kind);
        }
        case ExprKind::ASSIGN: {
            auto* a = static_cast<const AssignExpr*>(e);
            return contains(a->target.get(), kind) || contains(a->value.get(), kind);
        }
        case ExprKind::CALL:
            for (const ExprPtr& arg : static_cast<const CallExpr*>(e)->args)
                if (contains(arg.get(), kind)) return true;
            return false;
        default:
            return false;
    }
}

Operand IrGenerator::genIndex(const IndexExpr& e) {
    const Symbol* sym = info_.symbolOf(&e);
    Operand index = genExpr(e.index.get());
    Operand t = newTemp(irTypeOf(sym->type));
    Instr in;
    in.op = Op::LOAD;
    in.dst = t;
    in.a = varOperand(sym);
    in.b = index;
    in.line = e.line;
    emit(in);
    return t;
}

Operand IrGenerator::genUnary(const UnaryExpr& e) {
    Operand value = genExpr(e.operand.get());
    Operand t = newTemp(irTypeOf(info_.typeOf(&e)));
    Instr in;
    in.op = Op::UNARY;
    in.oper = e.op;
    in.dst = t;
    in.a = value;
    emit(in);
    return t;
}

Operand IrGenerator::genBinary(const BinaryExpr& e) {
    Operand l = genExpr(e.left.get());
    l = stable(l, e.right.get());
    Operand r = genExpr(e.right.get());

    if ((l.type == IrType::FLOAT && r.type == IrType::INT) || (l.type == IrType::INT && r.type == IrType::FLOAT)) {
        l = convert(l, IrType::FLOAT);
        r = convert(r, IrType::FLOAT);
    }

    Operand t = newTemp(irTypeOf(info_.typeOf(&e)));
    Instr in;
    in.op = Op::BINARY;
    in.oper = e.op;
    in.dst = t;
    in.a = l;
    in.b = r;
    in.line = e.line;
    emit(in);
    return t;
}

Operand IrGenerator::genLogical(const BinaryExpr& e) {
    Operand result = newTemp(IrType::BOOL);
    std::string end = newLabel();

    Instr first;
    first.op = Op::COPY;
    first.dst = result;
    first.a = genExpr(e.left.get());
    emit(first);

    emitJump(e.op == TokenType::AND ? Op::IF_FALSE : Op::IF_TRUE, result, end);

    Instr second;
    second.op = Op::COPY;
    second.dst = result;
    second.a = genExpr(e.right.get());
    emit(second);

    emitLabel(end);
    return result;
}

Operand IrGenerator::genAssign(const AssignExpr& e) {
    const Symbol* sym = info_.symbolOf(e.target.get());
    Operand target = varOperand(sym);

    if (e.target->kind == ExprKind::VARIABLE) {
        Instr in;
        in.op = Op::COPY;
        in.dst = target;
        in.a = genExprAs(e.value.get(), target.type);
        emit(in);
        return target;
    }

    auto& t = static_cast<const IndexExpr&>(*e.target);
    Operand index = genExpr(t.index.get());
    index = stable(index, e.value.get());
    Operand value = genExprAs(e.value.get(), target.type);
    Instr in;
    in.op = Op::STORE;
    in.dst = target;
    in.a = index;
    in.b = value;
    in.line = e.line;
    emit(in);
    return value;
}

Operand IrGenerator::genCall(const CallExpr& e) {
    const Symbol* fn = info_.symbolOf(&e);

    std::vector<Operand> args;
    for (size_t i = 0; i < e.args.size(); i++) {
        Operand a = genExprAs(e.args[i].get(), irTypeOf(fn->paramTypes[i]));
        for (size_t j = i + 1; j < e.args.size(); j++) a = stable(a, e.args[j].get());
        args.push_back(a);
    }
    for (const Operand& a : args) {
        Instr p;
        p.op = Op::PARAM;
        p.a = a;
        emit(p);
    }

    Instr call;
    call.op = Op::CALL;
    call.label = fn->irName;
    call.count = static_cast<int>(args.size());
    call.line = e.line;
    if (!fn->type.isVoid()) call.dst = newTemp(irTypeOf(fn->type));
    emit(call);
    return call.dst;
}
