// Owner: PS/CSC/23/0211  (Phase 3, Part 6)

#include <sstream>
#include "../ast.h"
#include "ir.h"

const char *irTypeName(IrType t)
{
    switch (t)
    {
    case IrType::INT:
        return "int";
    case IrType::FLOAT:
        return "float";
    case IrType::BOOL:
        return "bool";
    case IrType::STRING:
        return "string";
    case IrType::VOID:
        return "void";
    }
    return "?";
}

Operand Operand::temp(const std::string &name, IrType type)
{
    Operand o;
    o.kind = Kind::TEMP;
    o.name = name;
    o.type = type;
    return o;
}

Operand Operand::var(const std::string &name, IrType type, bool global)
{
    Operand o;
    o.kind = Kind::VAR;
    o.name = name;
    o.type = type;
    o.global = global;
    return o;
}

Operand Operand::intConst(long long v)
{
    Operand o;
    o.kind = Kind::INT;
    o.type = IrType::INT;
    o.intValue = v;
    return o;
}

Operand Operand::floatConst(double v)
{
    Operand o;
    o.kind = Kind::FLOAT;
    o.type = IrType::FLOAT;
    o.floatValue = v;
    return o;
}

Operand Operand::boolConst(bool v)
{
    Operand o;
    o.kind = Kind::BOOL;
    o.type = IrType::BOOL;
    o.boolValue = v;
    return o;
}

Operand Operand::stringConst(const std::string &v)
{
    Operand o;
    o.kind = Kind::STRING;
    o.type = IrType::STRING;
    o.stringValue = v;
    return o;
}

namespace
{

    std::string escape(const std::string &s)
    {
        std::string out;
        for (char c : s)
        {
            switch (c)
            {
            case '\n':
                out += "\\n";
                break;
            case '\t':
                out += "\\t";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\0':
                out += "\\0";
                break;
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            default:
                out += c;
            }
        }
        return out;
    }

    std::string floatText(double v)
    {
        std::ostringstream ss;
        ss << v;
        std::string s = ss.str();
        if (s.find_first_of(".eEn") == std::string::npos)
            s += ".0";
        return s;
    }

    // The destination of an instruction: temporaries show their type where
    // they get their value, e.g. "t1:int".
    std::string dstText(const Operand &o)
    {
        if (o.kind == Operand::Kind::TEMP)
            return o.name + ":" + irTypeName(o.type);
        return operandText(o);
    }

    std::string varText(const IrVar &v)
    {
        std::string s = v.name + ": " + irTypeName(v.type);
        if (v.arraySize >= 0)
            s += "[" + std::to_string(v.arraySize) + "]";
        return s;
    }

    std::string instrText(const Instr &in)
    {
        switch (in.op)
        {
        case Op::LABEL:
            return in.label + ":";
        case Op::COPY:
            return "  " + dstText(in.dst) + " = " + operandText(in.a);
        case Op::BINARY:
            return "  " + dstText(in.dst) + " = " + operandText(in.a) + " " + opSymbol(in.oper) + " " +
                   operandText(in.b);
        case Op::UNARY:
            return "  " + dstText(in.dst) + " = " + opSymbol(in.oper) + operandText(in.a);
        case Op::INT_TO_FLOAT:
            return "  " + dstText(in.dst) + " = (float) " + operandText(in.a);
        case Op::JUMP:
            return "  goto " + in.label;
        case Op::IF_FALSE:
            return "  ifFalse " + operandText(in.a) + " goto " + in.label;
        case Op::IF_TRUE:
            return "  if " + operandText(in.a) + " goto " + in.label;
        case Op::PARAM:
            return "  param " + operandText(in.a);
        case Op::CALL:
        {
            std::string call = "call " + in.label + ", " + std::to_string(in.count);
            return in.dst.isNone() ? "  " + call : "  " + dstText(in.dst) + " = " + call;
        }
        case Op::RETURN:
            return in.a.isNone() ? "  return" : "  return " + operandText(in.a);
        case Op::ARRAY:
            return "  array " + operandText(in.dst) + "[" + std::to_string(in.count) + "]: " +
                   irTypeName(in.dst.type);
        case Op::LOAD:
            return "  " + dstText(in.dst) + " = " + operandText(in.a) + "[" + operandText(in.b) + "]";
        case Op::STORE:
            return "  " + operandText(in.dst) + "[" + operandText(in.a) + "] = " + operandText(in.b);
        case Op::PRINT:
            return "  print " + operandText(in.a);
        case Op::PRINTLN:
            return "  println";
        case Op::READ:
            return "  read " + dstText(in.dst);
        }
        return "  ?";
    }

    void printFunction(const IrFunction &f, bool isMain, std::ostream &os)
    {
        if (isMain)
        {
            os << "main\n";
        }
        else
        {
            os << "func " << f.name << "(";
            for (size_t i = 0; i < f.params.size(); i++)
                os << (i ? ", " : "") << varText(f.params[i]);
            os << ") -> " << irTypeName(f.returnType) << "\n";
        }
        for (const IrVar &v : f.locals)
            os << "  local " << varText(v) << "\n";
        for (const Instr &in : f.code)
            os << instrText(in) << "\n";
        os << "end " << (isMain ? "main" : f.name) << "\n";
    }

} // namespace

std::string operandText(const Operand &o)
{
    switch (o.kind)
    {
    case Operand::Kind::NONE:
        return "_";
    case Operand::Kind::TEMP:
        return o.name;
    case Operand::Kind::VAR:
        return (o.global ? "@" : "") + o.name;
    case Operand::Kind::INT:
        return std::to_string(o.intValue);
    case Operand::Kind::FLOAT:
        return floatText(o.floatValue);
    case Operand::Kind::BOOL:
        return o.boolValue ? "true" : "false";
    case Operand::Kind::STRING:
        return "\"" + escape(o.stringValue) + "\"";
    }
    return "?";
}

void printIr(const IrProgram &program, std::ostream &os)
{
    for (const IrVar &g : program.globals)
        os << "global @" << varText(g) << "\n";
    if (!program.globals.empty())
        os << "\n";
    for (const IrFunction &f : program.functions)
    {
        printFunction(f, false, os);
        os << "\n";
    }
    printFunction(program.main, true, os);
}
