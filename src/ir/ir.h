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

// One input or output of an instruction.
struct Operand
{
    enum class Kind
    {
        NONE, // not used
        TEMP, // compiler-made temporary: t1, t2, ...
        VAR,  // a variable or parameter of the program
        INT,
        FLOAT,
        BOOL,
        STRING // constants
    };

    Kind kind = Kind::NONE;
    IrType type = IrType::VOID;
    std::string name;    // TEMP and VAR
    bool global = false; // VAR: a global variable (printed as @name)
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
    LABEL,        // label:
    COPY,         // dst = a
    BINARY,       // dst = a <oper> b        (+ - * / % == != < <= > >=)
    UNARY,        // dst = <oper> a          (- or !)
    INT_TO_FLOAT, // dst = (float) a
    JUMP,         // goto label
    IF_FALSE,     // ifFalse a goto label
    IF_TRUE,      // if a goto label
    PARAM,        // param a                 (pushes one argument)
    CALL,         // dst = call label, count (dst is NONE for a void call)
    RETURN,       // return [a]
    ARRAY,        // array dst[count]        (creates the array, all elements 0)
    LOAD,         // dst = a[b]
    STORE,        // dst[a] = b
    PRINT,        // print a                 (no newline)
    PRINTLN,      // println                 (newline at the end of a print statement)
    READ          // read dst
};

struct Instr
{
    Op op = Op::LABEL;
    Operand dst, a, b;
    TokenType oper = TokenType::PLUS; // BINARY / UNARY
    std::string label;                // LABEL / JUMP / IF_*: the label; CALL: the function
    int count = 0;                    // CALL: number of arguments; ARRAY: number of elements
    int line = 0;                     // source line, for runtime error messages
};

// A variable that belongs to a function (or a global variable).
struct IrVar
{
    std::string name;
    IrType type = IrType::INT;
    int arraySize = -1; // number of elements for arrays, -1 otherwise
};

struct IrFunction
{
    std::string name; // "main" for the top-level code
    IrType returnType = IrType::VOID;
    std::vector<IrVar> params;
    std::vector<IrVar> locals;
    std::vector<Instr> code;
};

struct IrProgram
{
    std::vector<IrVar> globals;
    std::vector<IrFunction> functions;
    IrFunction main; // the top-level statements, run from top to bottom
};

std::string operandText(const Operand &o);
void printIr(const IrProgram &program, std::ostream &os);

#endif
