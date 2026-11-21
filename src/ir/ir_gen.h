// Owner: PS/CSC/23/0210  

#ifndef IR_IR_GEN_H
#define IR_IR_GEN_H

#include <string>
#include "../ast.h"
#include "../semantic/semantic.h"
#include "ir.h"

inline IrType irTypeOf(const Type& t) {
    switch (t.base) {
        case Type::Base::INT: return IrType::INT;
        case Type::Base::FLOAT: return IrType::FLOAT;
        case Type::Base::BOOL: return IrType::BOOL;
        case Type::Base::STRING: return IrType::STRING;
        default: return IrType::VOID;
    }
}

class IrGenerator {
public:
    explicit IrGenerator(const SemanticInfo& info) : info_(info) {}

    IrProgram generate(const Program& program);

private:
    const SemanticInfo& info_;
    IrProgram program_;
    IrFunction* fn_ = nullptr;           // function being generated
    const Symbol* fnSymbol_ = nullptr;   // its symbol (null for main)
    int tempCount_ = 0;                  // t1, t2, ... (numbered per function)
    int labelCount_ = 0;                 // L1, L2, ... (numbered per function)
    int line_ = 0;                       // source line of what is being generated

    // ---- ir_gen_stmts.cpp (Part 7)
    Operand newTemp(IrType type);
    std::string newLabel();
    void emit(Instr in);
    void emitLabel(const std::string& label);
    void emitJump(Op op, const Operand& cond, const std::string& label);
    Operand varOperand(const Symbol* s) const;
    Operand defaultValue(IrType type) const;
    void genFunction(const FuncDecl& f);
    void genStmt(const Stmt* s);
    void genVarDecl(const VarDecl& d);
    void genIf(const IfStmt& s);
    void genWhile(const WhileStmt& s);
    void genFor(const ForStmt& s);
    void genReturn(const ReturnStmt& s);
    void genPrint(const PrintStmt& s);
    void genRead(const ReadStmt& s);

    // ---- ir_gen_exprs.cpp (Part 8)
    Operand genExpr(const Expr* e);
    Operand genExprAs(const Expr* e, IrType wanted);
    Operand convert(const Operand& o, IrType to);
    Operand stable(const Operand& o, const Expr* later);
    Operand genIndex(const IndexExpr& e);
    Operand genUnary(const UnaryExpr& e);
    Operand genBinary(const BinaryExpr& e);
    Operand genLogical(const BinaryExpr& e);
    Operand genAssign(const AssignExpr& e);
    Operand genCall(const CallExpr& e);
    static bool contains(const Expr* e, ExprKind kind);
};

#endif
