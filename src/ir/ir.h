// Owner: PS/CSC/23/0211  (Phase 3, Part 6)

#ifndef IR_IR_H
#define IR_IR_H

#include <ostream>
#include <string>
#include <vector>
#include "../token.h"

enum class IrType
{
    INT,
    FLOAT,
    BOOL,
    STRING,
    VOID
};

const char *irTypeName(IrType t);

struct Operand
{
    enum class Kind
    {
        NONE,
        TEMP,
        VAR,
        INT,
        FLOAT,
        BOOL,
        STRING
    };

    Kind kind = Kind::NONE;
    IrType type = IrType::VOID;
    std::string name;
    bool global = false;
    long long intValue = 0;
    double floatValue = 0;
    bool boolValue = false;
    std::string stringValue;

    static Operand none() { return Operand{}; }
    static Operand temp(const std::string &name, IrType type);
    static Operand var(const std::string &name, IrType type, bool global);
    static Operand intConst(long long v);
    static Operand floatConst(double v);
    static Operand boolConst(bool v);
    static Operand stringConst(const std::string &v);

    bool isNone() const { return kind == Kind::NONE; }
    bool isConst() const
    {
        return kind == Kind::INT || kind == Kind::FLOAT || kind == Kind::BOOL || kind == Kind::STRING;
    }
};

enum class Op
{
    LABEL,
    COPY,
    BINARY,
    UNARY,
    INT_TO_FLOAT,
    JUMP,
    IF_FALSE,
    IF_TRUE,
    PARAM,
    CALL,
    RETURN,
    ARRAY,
    LOAD,
    STORE,
    PRINT,
    PRINTLN,
    READ
};

struct Instr
{
    Op op = Op::LABEL;
    Operand dst, a, b;
    TokenType oper = TokenType::PLUS;
    std::string label;
    int count = 0;
    int line = 0;
};

struct IrVar
{
    std::string name;
    IrType type = IrType::INT;
    int arraySize = -1;
};

struct IrFunction
{
    std::string name;
    IrType returnType = IrType::VOID;
    std::vector<IrVar> params;
    std::vector<IrVar> locals;
    std::vector<Instr> code;
};

struct IrProgram
{
    std::vector<IrVar> globals;
    std::vector<IrFunction> functions;
    IrFunction main;
};

std::string operandText(const Operand &o);
void printIr(const IrProgram &program, std::ostream &os);

#endif
