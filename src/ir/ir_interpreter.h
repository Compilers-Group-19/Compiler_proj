// Owner: PS/CSC/23/0209  (Phase 3, Part 9)

#ifndef IR_IR_INTERPRETER_H
#define IR_IR_INTERPRETER_H

#include <istream>
#include <ostream>
#include <string>
#include "ir.h"

class IrInterpreter {
public:
    IrInterpreter(const IrProgram& program, std::istream& in, std::ostream& out)
        : program_(program), in_(in), out_(out) {}

    bool run();

    const std::string& error() const { return error_; }

    static constexpr size_t kMaxCallDepth = 100000;

private:
    const IrProgram& program_;
    std::istream& in_;
    std::ostream& out_;
    std::string error_;
};

#endif
