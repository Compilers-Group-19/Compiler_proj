// Owner: PS/CSC/23/0209 (Phase 3, Part 9)

#include "ir_interpreter.h"
#include "../ast.h"

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace {

struct Value {
    IrType type = IrType::INT;
    long long i = 0;
    double f = 0;
    bool b = false;
    std::string s;
};

Value defaultValue(IrType t) {
    Value v;
    v.type = t;
    return v;
}

struct RuntimeError {
    std::string message;
};

long long wrapAdd(long long a, long long b) {
    return static_cast<long long>(static_cast<uint64_t>(a) + static_cast<uint64_t>(b));
}
long long wrapSub(long long a, long long b) {
    return static_cast<long long>(static_cast<uint64_t>(a) - static_cast<uint64_t>(b));
}
long long wrapMul(long long a, long long b) {
    return static_cast<long long>(static_cast<uint64_t>(a) * static_cast<uint64_t>(b));
}

std::string floatText(double v) {
    std::ostringstream ss;
    ss << v;
    std::string s = ss.str();
    if (s.find_first_of(".eEn") == std::string::npos) s += ".0";
    return s;
}

std::string text(const Value& v) {
    switch (v.type) {
        case IrType::INT: return std::to_string(v.i);
        case IrType::FLOAT: return floatText(v.f);
        case IrType::BOOL: return v.b ? "true" : "false";
        case IrType::STRING: return v.s;
        case IrType::VOID: return "";
    }
    return "";
}

struct Frame {
    const IrFunction* fn = nullptr;
    size_t pc = 0;
    std::unordered_map<std::string, Value> vars;
    std::unordered_map<std::string, std::vector<Value>> arrays;
    Operand returnTo;
};

class Machine {
public:
    Machine(const IrProgram& p, std::istream& in, std::ostream& out) : program_(p), in_(in), out_(out) {
        for (const IrFunction& f : p.functions) functions_[f.name] = &f;
        index(p.main);
        for (const IrFunction& f : p.functions) index(f);
        for (const IrVar& g : p.globals) {
            if (g.arraySize >= 0) globalArrays_[g.name].assign(static_cast<size_t>(g.arraySize), defaultValue(g.type));
            else globals_[g.name] = defaultValue(g.type);
        }
    }

    void run() {
        Frame mainFrame;
        mainFrame.fn = &program_.main;
        stack_.push_back(std::move(mainFrame));

        while (!stack_.empty()) {
            Frame& f = stack_.back();
            if (f.pc >= f.fn->code.size()) {
                if (stack_.size() == 1) return;
                fail(f.fn->code.empty() ? 0 : f.fn->code.back().line,
                     "function '" + f.fn->name + "' ended without returning a value");
            }
            const Instr& in = f.fn->code[f.pc++];
            line_ = in.line;
            step(in);
        }
    }

private:
    const IrProgram& program_;
    std::istream& in_;
    std::ostream& out_;
    std::unordered_map<std::string, const IrFunction*> functions_;
    std::unordered_map<const IrFunction*, std::unordered_map<std::string, size_t>> labels_;
    std::unordered_map<std::string, Value> globals_;
    std::unordered_map<std::string, std::vector<Value>> globalArrays_;
    std::vector<Frame> stack_;
    std::vector<Value> args_;
    int line_ = 0;

    [[noreturn]] void fail(int line, const std::string& message) {
        throw RuntimeError{"Runtime error at line " + std::to_string(line) + ": " + message};
    }
    [[noreturn]] void fail(const std::string& message) { fail(line_, message); }

    void index(const IrFunction& f) {
        auto& map = labels_[&f];
        for (size_t i = 0; i < f.code.size(); i++)
            if (f.code[i].op == Op::LABEL) map[f.code[i].label] = i;
    }

    Value get(const Operand& o) {
        switch (o.kind) {
            case Operand::Kind::INT: { Value v = defaultValue(IrType::INT); v.i = o.intValue; return v; }
            case Operand::Kind::FLOAT: { Value v = defaultValue(IrType::FLOAT); v.f = o.floatValue; return v; }
            case Operand::Kind::BOOL: { Value v = defaultValue(IrType::BOOL); v.b = o.boolValue; return v; }
            case Operand::Kind::STRING: { Value v = defaultValue(IrType::STRING); v.s = o.stringValue; return v; }
            case Operand::Kind::TEMP:
            case Operand::Kind::VAR: {
                auto& vars = o.global ? globals_ : stack_.back().vars;
                auto it = vars.find(o.name);
                return it == vars.end() ? defaultValue(o.type) : it->second;
            }
            case Operand::Kind::NONE: break;
        }
        return defaultValue(IrType::VOID);
    }

    void set(const Operand& o, const Value& v) {
        if (o.isNone()) return;
        (o.global ? globals_ : stack_.back().vars)[o.name] = v;
    }

    std::vector<Value>& array(const Operand& o) {
        auto& arrays = o.global ? globalArrays_ : stack_.back().arrays;
        return arrays[o.name];
    }

    size_t checkedIndex(const Operand& arr, const Value& idx) {
        std::vector<Value>& a = array(arr);
        if (idx.i < 0 || static_cast<size_t>(idx.i) >= a.size()) {
            fail("array index " + std::to_string(idx.i) + " is out of range for '" + arr.name + "' (size " +
                 std::to_string(a.size()) + ")");
        }
        return static_cast<size_t>(idx.i);
    }

    void jump(const std::string& label) {
        Frame& f = stack_.back();
        f.pc = labels_[f.fn].at(label);
    }

    Value binary(TokenType op, const Value& a, const Value& b) {
        Value r = defaultValue(a.type);
        auto compare = [&](bool result) {
            r = defaultValue(IrType::BOOL);
            r.b = result;
            return r;
        };

        if (a.type == IrType::INT) {
            switch (op) {
                case TokenType::PLUS: r.i = wrapAdd(a.i, b.i); return r;
                case TokenType::MINUS: r.i = wrapSub(a.i, b.i); return r;
                case TokenType::STAR: r.i = wrapMul(a.i, b.i); return r;
                case TokenType::SLASH:
                case TokenType::PERCENT:
                    if (b.i == 0) fail("division by zero");
                    if (b.i == -1) {
                        r.i = op == TokenType::SLASH ? wrapSub(0, a.i) : 0;
                        return r;
                    }
                    r.i = op == TokenType::SLASH ? a.i / b.i : a.i % b.i;
                    return r;
                case TokenType::EQ: return compare(a.i == b.i);
                case TokenType::NEQ: return compare(a.i != b.i);
                case TokenType::LT: return compare(a.i < b.i);
                case TokenType::LE: return compare(a.i <= b.i);
                case TokenType::GT: return compare(a.i > b.i);
                case TokenType::GE: return compare(a.i >= b.i);
                default: break;
            }
        } else if (a.type == IrType::FLOAT) {
            switch (op) {
                case TokenType::PLUS: r.f = a.f + b.f; return r;
                case TokenType::MINUS: r.f = a.f - b.f; return r;
                case TokenType::STAR: r.f = a.f * b.f; return r;
                case TokenType::SLASH:
                    if (b.f == 0.0) fail("division by zero");
                    r.f = a.f / b.f;
                    return r;
                case TokenType::EQ: return compare(a.f == b.f);
                case TokenType::NEQ: return compare(a.f != b.f);
                case TokenType::LT: return compare(a.f < b.f);
                case TokenType::LE: return compare(a.f <= b.f);
                case TokenType::GT: return compare(a.f > b.f);
                case TokenType::GE: return compare(a.f >= b.f);
                default: break;
            }
        } else if (a.type == IrType::BOOL) {
            if (op == TokenType::EQ) return compare(a.b == b.b);
            if (op == TokenType::NEQ) return compare(a.b != b.b);
        } else if (a.type == IrType::STRING) {
            if (op == TokenType::EQ) return compare(a.s == b.s);
            if (op == TokenType::NEQ) return compare(a.s != b.s);
        }
        fail(std::string("operator '") + opSymbol(op) + "' is not supported for " + irTypeName(a.type));
    }

    Value readValue(IrType type) {
        std::string word;
        if (!(in_ >> word)) fail("read: no more input");
        Value v = defaultValue(type);
        try {
            size_t used = 0;
            switch (type) {
                case IrType::INT: v.i = std::stoll(word, &used); break;
                case IrType::FLOAT: v.f = std::stod(word, &used); break;
                case IrType::BOOL:
                    if (word != "true" && word != "false") throw std::invalid_argument("bool");
                    v.b = word == "true";
                    used = word.size();
                    break;
                default: v.s = word; used = word.size(); break;
            }
            if (used != word.size()) throw std::invalid_argument("trailing");
        } catch (const std::exception&) {
            fail(std::string("read: expected ") + irTypeName(type) + ", got '" + word + "'");
        }
        return v;
    }

    void step(const Instr& in) {
        switch (in.op) {
            case Op::LABEL: return;
            case Op::COPY: set(in.dst, get(in.a)); return;
            case Op::BINARY: set(in.dst, binary(in.oper, get(in.a), get(in.b))); return;
            case Op::UNARY: {
                Value v = get(in.a);
                if (in.oper == TokenType::NOT) v.b = !v.b;
                else if (v.type == IrType::FLOAT) v.f = -v.f;
                else v.i = wrapSub(0, v.i);
                set(in.dst, v);
                return;
            }
            case Op::INT_TO_FLOAT: {
                Value v = defaultValue(IrType::FLOAT);
                v.f = static_cast<double>(get(in.a).i);
                set(in.dst, v);
                return;
            }
            case Op::JUMP: jump(in.label); return;
            case Op::IF_FALSE: if (!get(in.a).b) jump(in.label); return;
            case Op::IF_TRUE: if (get(in.a).b) jump(in.label); return;
            case Op::PARAM: args_.push_back(get(in.a)); return;
            case Op::CALL: call(in); return;
            case Op::RETURN: ret(in); return;
            case Op::ARRAY:
                array(in.dst).assign(static_cast<size_t>(in.count), defaultValue(in.dst.type));
                return;
            case Op::LOAD: {
                Value idx = get(in.b);
                set(in.dst, array(in.a)[checkedIndex(in.a, idx)]);
                return;
            }
            case Op::STORE: {
                Value idx = get(in.a);
                Value v = get(in.b);
                array(in.dst)[checkedIndex(in.dst, idx)] = v;
                return;
            }
            case Op::PRINT: out_ << text(get(in.a)); return;
            case Op::PRINTLN: out_ << "\n"; return;
            case Op::READ: set(in.dst, readValue(in.dst.type)); return;
        }
    }

    void call(const Instr& in) {
        auto it = functions_.find(in.label);
        if (it == functions_.end()) fail("call to unknown function '" + in.label + "'");
        if (stack_.size() >= IrInterpreter::kMaxCallDepth) {
            fail("stack overflow: more than " + std::to_string(IrInterpreter::kMaxCallDepth) +
                 " nested calls (endless recursion?)");
        }
        const IrFunction* fn = it->second;
        size_t n = static_cast<size_t>(in.count);

        Frame callee;
        callee.fn = fn;
        callee.returnTo = in.dst;
        for (size_t i = 0; i < n && i < fn->params.size(); i++)
            callee.vars[fn->params[i].name] = args_[args_.size() - n + i];
        args_.resize(args_.size() - n);
        stack_.push_back(std::move(callee));
    }

    void ret(const Instr& in) {
        Value result = in.a.isNone() ? defaultValue(IrType::VOID) : get(in.a);
        Operand dst = stack_.back().returnTo;
        stack_.pop_back();
        if (!stack_.empty()) set(dst, result);
    }
};

}

bool IrInterpreter::run() {
    error_.clear();
    try {
        Machine machine(program_, in_, out_);
        machine.run();
        out_.flush();
        return true;
    } catch (const RuntimeError& e) {
        out_.flush();
        error_ = e.message;
        return false;
    }
}
